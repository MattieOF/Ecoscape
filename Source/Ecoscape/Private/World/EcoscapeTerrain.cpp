﻿// copyright lololol

#include "World/EcoscapeTerrain.h"

#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "EcoscapeStats.h"
#include "ProceduralMeshComponent.h"
#include "Serialization/BufferArchive.h"
#include "World/FastNoise.h"

DECLARE_CYCLE_STAT(TEXT("Terrain: Generate"), STAT_GenTerrain, STATGROUP_EcoscapeTerrain);
DECLARE_CYCLE_STAT(TEXT("Terrain: Get Nearest Vertex"), STAT_GetNearestVert, STATGROUP_EcoscapeTerrain);
DECLARE_CYCLE_STAT(TEXT("Terrain: Add Vertex Color"), STAT_AddVertColor, STATGROUP_EcoscapeTerrain);
DECLARE_CYCLE_STAT(TEXT("Terrain: Get Verticies In Sphere"), STAT_VertsInSphere, STATGROUP_EcoscapeTerrain);

AEcoscapeTerrain::AEcoscapeTerrain()
{
	PrimaryActorTick.bCanEverTick = false;

	ProceduralMeshComponent = CreateDefaultSubobject<UProceduralMeshComponent>(TEXT("Procedural Mesh"));
	ProceduralMeshComponent->SetupAttachment(GetRootComponent());
	ProceduralMeshComponent->SetCollisionResponseToChannel(ECC_ITEM_PLACEABLE_ON, ECR_Block);
}

void AEcoscapeTerrain::BeginPlay()
{
	Super::BeginPlay();

	Regenerate();
}

void AEcoscapeTerrain::Tick(float DeltaSeconds)
{
}

void AEcoscapeTerrain::SerialiseTerrain(FArchive& Archive)
{
	Archive << Verticies;
	Archive << Triangles;
	Archive << UV0;
	Archive << VertexColors;
	Archive << ColorOffsets;
}

bool AEcoscapeTerrain::SerialiseTerrainToFile(FString Filename)
{
	// Serialise with buffer archive
	FBufferArchive BinarySaveArchive;
	SerialiseTerrain(BinarySaveArchive);

	// Save to disk
	const FString Path = *FString::Printf(TEXT("%lsSaves/%s"), *FPaths::ProjectSavedDir(), *Filename);
	const bool SaveResult = FFileHelper::SaveArrayToFile(BinarySaveArchive, *Path);
	if (!SaveResult)
	{
		UE_LOG(LogEcoscape, Error, TEXT("Failed to save terrain to %s!"), *Path);
		return false;
	}
	
	// Empty buffer
	BinarySaveArchive.FlushCache();
	BinarySaveArchive.Empty(); 
	return true;
}

bool AEcoscapeTerrain::DeserialiseTerrainFromFile(FString Filename)
{
	TArray<uint8> BinaryArray;

	// Load and verify file
	const FString Path = *FString::Printf(TEXT("%lsSaves/%ls"), *FPaths::ProjectSavedDir(), *Filename);
	const bool LoadFileResult = FFileHelper::LoadFileToArray(BinaryArray, *Path);
	if (!LoadFileResult)
	{
		UE_LOG(LogEcoscape, Error, TEXT("In Ecoscape Terrain loader, failed to open file %s!"), *Path);
		return false;
	}
	if (BinaryArray.Num() <= 0)
	{
		UE_LOG(LogEcoscape, Error, TEXT("In Ecoscape Terrain loader, file %s is empty!"), *Path);
		return false;
	}

	// Create binary loader
	FMemoryReader BinaryLoader = FMemoryReader(BinaryArray, true);
	BinaryLoader.Seek(0);
	
	// Do the loading
	ResetMeshData();
	SerialiseTerrain(BinaryLoader);

	// Recreate terrain from loaded data
	CreateMesh();

	// Empty buffer
	BinaryLoader.FlushCache();
	BinaryArray.Empty();
	BinaryLoader.Close();

	return true;
}

void AEcoscapeTerrain::ResetMeshData()
{	
	Verticies.Empty();
	Triangles.Empty();
	UV0.Empty();
	VertexColors.Empty();
	ColorOffsets.Empty();
}

void AEcoscapeTerrain::GenerateVerticies()
{
	for (int x = 0; x <= Width; x++)
	{
		for (int y = 0; y <= Height; y++)
		{
			// TODO: ensure that it's never an integer
			float Z = 0;
			for (const auto& [NoiseType, CoordinateScale, HeightScale, Seed, Offset] : NoiseLayers)
			{
				switch (NoiseType)
				{
				case ENTPerlin:
					Noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
					break;
				case ENTSimplex:
					Noise.SetNoiseType(FastNoiseLite::NoiseType_OpenSimplex2);
					break;
				default:
					UE_LOG(LogEcoscape, Error, TEXT("Invalid noise type: %i"), static_cast<int>(NoiseType));
					break;
				}

				Noise.SetSeed(Seed);

				Z += Noise.GetNoise(x * CoordinateScale, y * CoordinateScale) * HeightScale;
			}

			
			Verticies.Add(FVector(x * Scale, y * Scale, Z));
			UV0.Add(FVector2D(x * UVScale, y * UVScale));
			
			Noise.SetNoiseType(FastNoiseLite::NoiseType_Perlin);
			Noise.SetSeed(ColorOffsetSeed);
			
			float Value = Noise.GetNoise((x + 0.1f) * 5.0f, (y + 0.1f) * 5.0f);
			Value = (Value + 1) / 2;
			Value = FMath::Lerp(ColorOffsetRange.X, ColorOffsetRange.Y, Value);
			FVector Offset = FVector(Value);
			ColorOffsets.Add(Offset);
			
			FColor VertexColor = FColor(64,41,5, 255);
			VertexColor = UEcoscapeStatics::AddToColor(VertexColor, Offset);
			VertexColors.Add(VertexColor);
		}
	}
}

void AEcoscapeTerrain::GenerateIndicies()
{
	int Vertex = 0;

	for (int X = 0; X < Width; ++X)
	{
		for (int Y = 0; Y < Height; ++Y)
		{
			Triangles.Add(Vertex);//Bottom left corner
			Triangles.Add(Vertex + 1);//Bottom right corner
			Triangles.Add(Vertex + Height + 1);//Top left corner
			Triangles.Add(Vertex + 1);//Bottom right corner
			Triangles.Add(Vertex + Height + 2);//Top right corner
			Triangles.Add(Vertex + Height + 1);//Top left corner

			++Vertex;
		}
		++Vertex;
	}
}

void AEcoscapeTerrain::CreateMesh() const
{
	ProceduralMeshComponent->CreateMeshSection(0, Verticies, Triangles, TArray<FVector>(), UV0, VertexColors, TArray<FProcMeshTangent>(), true);
	ProceduralMeshComponent->SetMaterial(0, Material);
}

int AEcoscapeTerrain::GetClosestVertex(FVector Position)
{
	SCOPE_CYCLE_COUNTER(STAT_GetNearestVert);
	
	const FVector ActorLoc = GetActorLocation();
	const float OriginX = ActorLoc.X;
	const float EndX = OriginX + (Width * Scale);
	const float OriginY = ActorLoc.Y;
	const float EndY = OriginY + (Height * Scale);

	// Using these two lines would be shorter, but I think slower, and this is quite a hot path
	// const float XAlpha = FMath::GetMappedRangeValueUnclamped(FVector2D(ActorLoc.X, ActorLoc.X + (Width * Scale)), FVector2D(0, 1), Position.X);
	// const float YAlpha = FMath::GetMappedRangeValueUnclamped(FVector2D(ActorLoc.Y, ActorLoc.Y + (Height * Scale)), FVector2D(0, 1), Position.Y);
	const float XAlpha = FMath::Clamp((Position.X - OriginX) / (EndX - OriginX), 0, 1);
	const float YAlpha = FMath::Clamp((Position.Y - OriginY) / (EndY - OriginY), 0, 1);

	const int X = FMath::RoundToInt(Width  * XAlpha);
	const int Y = FMath::RoundToInt(Height * YAlpha);

	return (X * (Width + 1)) + Y;
}

TArray<int> AEcoscapeTerrain::GetVerticiesInSphere(FVector Position, float Radius, bool CheckZ)
{
	SCOPE_CYCLE_COUNTER(STAT_VertsInSphere);

	DrawDebugSphere(GetWorld(), Position, Radius, 30, FColor::Red);
	
	TArray<int> Indicies;

	const int RootVertex = GetClosestVertex(Position);
	const int IndexRadius = (Radius / Scale) + 1; // Add 1 so verts that should be inside but aren't due to offsets from the root vertex method are included.

	for (int CurrentRadius = 0; CurrentRadius < IndexRadius; CurrentRadius++)
	{
		// Special case for the root vertex. Just test distance.
		if (CurrentRadius == 0)
		{
			const float Dist = CheckZ ? FVector::Dist(Position, GetVertexPositionWorld(RootVertex)) : FVector::DistXY(Position, GetVertexPositionWorld(RootVertex));
			if (Dist <= Radius)
				Indicies.Add(RootVertex);
			if (!CheckZ && Dist > Radius)
				return Indicies;
			continue;
		}

		for (int X = -CurrentRadius; X <= CurrentRadius; X++)
		{
			for (int Y = -CurrentRadius; Y <= CurrentRadius; Y++)
			{
				int Index = RootVertex + X + (Y * (Width + 1));
				
				// Ensure no out of bounds indicies are given
				if (Index < 0 || Index >= Verticies.Num())
					continue;

				const float Dist = CheckZ ? FVector::Dist(Position, GetVertexPositionWorld(Index)) : FVector::DistXY(Position, GetVertexPositionWorld(Index));
				if (Dist <= Radius)
					Indicies.Add(Index);
			}
		}
	}
	
	return Indicies;
}

void AEcoscapeTerrain::AddVertexColour(int Index, FColor AddedColor)
{
	SCOPE_CYCLE_COUNTER(STAT_AddVertColor);
	
	if (Index < 0 || Index >= Verticies.Num())
	{
		UE_LOG(LogEcoscape, Error, TEXT("AEcoscapeTerrain::AddVertexColour called with out of range index %i (range is 0..%i)"), Index, Verticies.Num());
		return;	
	}

	VertexColors[Index] += AddedColor;
	ProceduralMeshComponent->UpdateMeshSection(0, Verticies, TArray<FVector>(), UV0, VertexColors, TArray<FProcMeshTangent>());
}

#if WITH_EDITOR
void AEcoscapeTerrain::DrawVerticies()
{
	for (int i = 0; i < Verticies.Num(); i++)
		DrawDebugSphere(GetWorld(), GetVertexPositionWorld(i), 20.0f, 8, FColor::Red, false, 5);
}

void AEcoscapeTerrain::DrawIndicies()
{
	for (int i = 0; i < Verticies.Num(); i++)
		DrawDebugString(GetWorld(), GetVertexPositionWorld(i) + FVector(0, 0, 100), FString::FromInt(i), nullptr, FColor::White, 3);
}
#endif

void AEcoscapeTerrain::Regenerate()
{
	SCOPE_CYCLE_COUNTER(STAT_GenTerrain);
	
	// Re-roll noise seeds	
	for (FTerrainNoiseLayer& Layer : NoiseLayers)
		Layer.Seed = FMath::RandRange(0.0f, 1000000.0f);
	ColorOffsetSeed = FMath::RandRange(0.0f, 1000000.0f);
	
	ResetMeshData();
	GenerateVerticies();
	GenerateIndicies();
	CreateMesh();
}

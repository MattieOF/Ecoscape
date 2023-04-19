// copyright lololol

#include "World/EcoscapeTerrain.h"

#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "EcoscapeStats.h"
#include "ProceduralMeshComponent.h"
#include "Serialization/BufferArchive.h"
#include "World/FastNoise.h"

DECLARE_CYCLE_STAT(TEXT("Terrain: Generate"), STAT_GenTerrain, STATGROUP_EcoscapeTerrain);
DECLARE_CYCLE_STAT(TEXT("Terrain: Get Nearest Vertex"), STAT_GetNearestVert, STATGROUP_Ecoscape);

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

void AEcoscapeTerrain::SerialiseTerrain(FString Filename)
{
	// Serialise with buffer archive
	FBufferArchive BinarySaveArchive;
	BinarySaveArchive << Verticies;
	BinarySaveArchive << Triangles;
	BinarySaveArchive << UV0;
	BinarySaveArchive << VertexColors;
	BinarySaveArchive << ColorOffsets;

	// Save to disk
	const FString Path = *FString::Printf(TEXT("%lsSaves/%s"), *FPaths::ProjectSavedDir(), *Filename);
	const bool SaveResult = FFileHelper::SaveArrayToFile(BinarySaveArchive, *Path);
	if (!SaveResult)
	{
		UE_LOG(LogEcoscape, Error, TEXT("Failed to save terrain to %s!"), *Path);
		return;
	}
	
	// Empty buffer
	BinarySaveArchive.FlushCache();
	BinarySaveArchive.Empty();
}

void AEcoscapeTerrain::DeserialiseTerrain(FString Filename)
{
	
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
			ColorOffsets.Add(FVector(Value));
			const bool bNegative = Value < 0;
			Value = FMath::Abs(Value);
			FColor ColorOffset = FColor(static_cast<int>(Value), static_cast<int>(Value), static_cast<int>(Value), 255);
			
			FColor VertexColor = FColor(64,41,5, 255);
			if (bNegative)
				VertexColor = VertexColor - ColorOffset;
			else
				VertexColor += ColorOffset;
			VertexColor.A = 255;
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

	const float XAlpha = FMath::Clamp((Position.X - OriginX) / (EndX - OriginX), 0, 1);
	const float YAlpha = FMath::Clamp((Position.Y - OriginY) / (EndY - OriginY), 0, 1);

	const int X = FMath::RoundToInt(Width  * XAlpha);
	const int Y = FMath::RoundToInt(Height * YAlpha);

	// DrawDebugSphere(GetWorld(), ActorLoc + FVector(Width * Scale * XAlpha, Height * Scale * YAlpha, 300), 20.0f, 6, FColor::Red);
	// DrawDebugSphere(GetWorld(), ActorLoc + FVector(Scale * X, Scale * Y, 500), 20.0f, 6, FColor::Red);
	
	return (X * (Width + 1)) + Y;
}

void AEcoscapeTerrain::AddVertexColour(int Index, FColor AddedColor)
{
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
	
	// Reroll noise seeds	
	for (FTerrainNoiseLayer& Layer : NoiseLayers)
		Layer.Seed = FMath::RandRange(0.0f, 1000000.0f);
	ColorOffsetSeed = FMath::RandRange(0.0f, 1000000.0f);
	
	ResetMeshData();
	GenerateVerticies();
	GenerateIndicies();
	CreateMesh();
}

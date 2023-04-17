// copyright lololol

#include "World/EcoscapeTerrain.h"

#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "ProceduralMeshComponent.h"
#include "Serialization/BufferArchive.h"

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

void AEcoscapeTerrain::SerialiseTerrain()
{
	// Serialise with buffer archive
	FBufferArchive BinarySaveArchive;
	BinarySaveArchive << Verticies;
	BinarySaveArchive << Triangles;
	BinarySaveArchive << UV0;

	// Save to disk
	const FString Path = *FString::Printf(TEXT("%lsSaves/Terrain.esl"), *FPaths::ProjectSavedDir());
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

void AEcoscapeTerrain::ResetMeshData()
{
	Verticies.Empty();
	Triangles.Empty();
	UV0.Empty();
}

void AEcoscapeTerrain::GenerateVerticies()
{
	for (int x = 0; x <= Width; x++)
	{
		for (int y = 0; y <= Height; y++)
		{
			// TODO: ensure that it's never an integer
			float Z = (FMath::PerlinNoise2D(FVector2D(PrimaryOctaveSeed + x * NoiseScale + 0.1, PrimaryOctaveSeed + y * NoiseScale + 0.1)) * HeightScale)
			          + (FMath::PerlinNoise2D(FVector2D(SecondaryOctaveSeed + x * (NoiseScale * 4) + 0.1, SecondaryOctaveSeed + y * (NoiseScale * 4) + 0.1)) * HeightScale / 3)
			          + (FMath::PerlinNoise2D(FVector2D(TertiaryOctaveSeed + x * (NoiseScale / 2) + 0.1, TertiaryOctaveSeed + y * (NoiseScale / 2) + 0.1)) * HeightScale * 2.5);
			
			Verticies.Add(FVector(x * Scale, y * Scale, Z));
			UV0.Add(FVector2D(x * UVScale, y * UVScale));
			VertexColors.Add(FColor::MakeRandomColor());
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

void AEcoscapeTerrain::Regenerate()
{
	PrimaryOctaveSeed   = FMath::RandRange(0.0f, 100000.0f);
	SecondaryOctaveSeed = FMath::RandRange(0.0f, 100000.0f);
	TertiaryOctaveSeed  = FMath::RandRange(0.0f, 100000.0f);
	
	ResetMeshData();
	GenerateVerticies();
	GenerateIndicies();
	CreateMesh();
}

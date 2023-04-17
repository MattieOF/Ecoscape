// copyright lololol

#include "World/EcoscapeTerrain.h"

#include "EcoscapeStatics.h"
#include "ProceduralMeshComponent.h"

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
			float Z = FMath::PerlinNoise2D(FVector2D(NoiseOffset + x * NoiseScale + 0.1, NoiseOffset + y * NoiseScale + 0.1));
			
			Verticies.Add(FVector(x * Scale, y * Scale, Z * HeightScale));
			UV0.Add(FVector2D(x * UVScale, y * UVScale));
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
	ProceduralMeshComponent->CreateMeshSection(0, Verticies, Triangles, TArray<FVector>(), UV0, TArray<FColor>(), TArray<FProcMeshTangent>(), true);
	ProceduralMeshComponent->SetMaterial(0, Material);
}

void AEcoscapeTerrain::Regenerate()
{
	NoiseOffset = FMath::RandRange(0.0f, 10000.0f);
	
	ResetMeshData();
	GenerateVerticies();
	GenerateIndicies();
	CreateMesh();
}

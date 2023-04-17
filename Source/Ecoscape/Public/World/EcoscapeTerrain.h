// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EcoscapeTerrain.generated.h"

class UProceduralMeshComponent;

USTRUCT(BlueprintType)
struct FTerrainNoiseLayer
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	float Scale = .1f;

	UPROPERTY(EditAnywhere)
	float Seed = 0;

	
};

UCLASS()
class ECOSCAPE_API AEcoscapeTerrain : public AActor
{
	GENERATED_BODY()

public:
	AEcoscapeTerrain();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable, CallInEditor)
	void SerialiseTerrain();

	void ResetMeshData();
	void GenerateVerticies();
	void GenerateIndicies();
	void CreateMesh() const;

	UFUNCTION(BlueprintCallable, CallInEditor)
	void Regenerate();
	
	UPROPERTY()
	UProceduralMeshComponent* ProceduralMeshComponent;

	UPROPERTY(EditAnywhere)
	int Width = 50;
	UPROPERTY(EditAnywhere)
	int Height = 50;
	UPROPERTY(EditAnywhere, meta=(ClampMin=1))
	float Scale = 100;
	UPROPERTY(EditAnywhere, meta=(ClampMin=0.0001))
	float UVScale = 100;
	UPROPERTY(EditAnywhere)
	float HeightScale = 100;
	UPROPERTY(EditAnywhere)
	float NoiseScale = 0.1f;

	float PrimaryOctaveSeed = 0;
	float SecondaryOctaveSeed = 0;
	float TertiaryOctaveSeed = 0;
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	// Mesh data
	UPROPERTY()
	TArray<FVector> Verticies;
	TArray<FColor> VertexColors;
	UPROPERTY()
	TArray<int> Triangles;
	UPROPERTY()
	TArray<FVector2D> UV0;
};

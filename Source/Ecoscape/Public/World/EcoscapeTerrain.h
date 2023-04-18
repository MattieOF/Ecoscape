// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "FastNoise.h"
#include "GameFramework/Actor.h"
#include "EcoscapeTerrain.generated.h"

class UProceduralMeshComponent;

UENUM(BlueprintType)
enum ENoiseType
{
	ENTPerlin    UMETA(DisplayName = "Perlin"),
	ENTSimplex   UMETA(DisplayName = "Simplex")
};

USTRUCT(BlueprintType)
struct FTerrainNoiseLayer
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ENoiseType> NoiseType = ENTPerlin;
	
	UPROPERTY(EditAnywhere)
	float CoordinateScale = .1f;

	UPROPERTY(EditAnywhere)
	float HeightScale = 100.0f;
	
	UPROPERTY(EditAnywhere)
	float Seed = 0;

	UPROPERTY(EditAnywhere)
	float Offset = .1f;
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
	
	/**
	 * @brief Distance in units between each vertex
	 */
	UPROPERTY(EditAnywhere)
	float Scale = 150;

	/**
	 * @brief UV scale
	 */
	UPROPERTY(EditAnywhere, meta=(ClampMin=0.0001))
	float UVScale = 1;
	
	UPROPERTY(EditAnywhere)
	TArray<FTerrainNoiseLayer> NoiseLayers;
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	// Mesh data
	UPROPERTY()
	TArray<FVector> Verticies;
	UPROPERTY()
	TArray<FColor> VertexColors;
	UPROPERTY()
	TArray<int> Triangles;
	UPROPERTY()
	TArray<FVector2D> UV0;

	FastNoiseLite Noise;
};

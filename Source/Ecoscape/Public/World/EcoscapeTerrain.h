﻿// copyright lololol

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
struct FVertexOverlapInfo
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	int Index;
	
	UPROPERTY(BlueprintReadWrite, EditAnywhere)
	float Distance;
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

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE FVector GetVertexPositionLocal(int Index) { return Verticies[Index]; }
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE FVector GetVertexPositionWorld(int Index) { return GetActorLocation() + GetVertexPositionLocal(Index); }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE float GetScale() { return Scale; }

	UFUNCTION(BlueprintCallable, BlueprintPure)
	int GetClosestVertex(FVector Position);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<FVertexOverlapInfo> GetVerticiesInSphere(FVector Position, float Radius, bool CheckZ = false);

	UFUNCTION(BlueprintCallable)
	void AddVertexColour(int Index, FColor AddedColor, bool Flush = true);

	UFUNCTION(BlueprintCallable)
	FORCEINLINE void FlushMesh();
	
protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

	// -----------------------
	// Serialisation functions
	// -----------------------
	void SerialiseTerrain(FArchive& Archive);
	UFUNCTION(BlueprintCallable, CallInEditor)
	bool SerialiseTerrainToFile(FString Filename = "Terrain.esl");
	UFUNCTION(BlueprintCallable, CallInEditor)
	bool DeserialiseTerrainFromFile(FString Filename = "Terrain.esl");
#if WITH_EDITOR
	// These two functions are for buttons in the inspector to test serialisation
	UFUNCTION(BlueprintCallable, CallInEditor)
	void SerialiseTerrainToTestFile() { SerialiseTerrainToFile(); }
	UFUNCTION(BlueprintCallable, CallInEditor)
	void DeserialiseTerrainFromTestFile() { DeserialiseTerrainFromFile(); }
#endif

	void ResetMeshData();
	void GenerateVerticies();
	void GenerateIndicies();
	void CreateMesh() const;

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor)
	void DrawVerticies();
	
	UFUNCTION(BlueprintCallable, CallInEditor)
	void DrawIndicies();
#endif

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
	FVector2D ColorOffsetRange = FVector2D(-30, 15);
	
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
	UPROPERTY()
	TArray<FVector> ColorOffsets;

	float ColorOffsetSeed = 0;
	
	FastNoiseLite Noise;
};

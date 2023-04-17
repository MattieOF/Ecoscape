// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "EcoscapeTerrain.generated.h"

class UProceduralMeshComponent;

UCLASS()
class ECOSCAPE_API AEcoscapeTerrain : public AActor
{
	GENERATED_BODY()

public:
	AEcoscapeTerrain();

protected:
	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;

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

	float NoiseOffset = 0;
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;

	// Mesh data
	UPROPERTY()
	TArray<FVector> Verticies;
	UPROPERTY()
	TArray<int> Triangles;
	UPROPERTY()
	TArray<FVector2D> UV0;
};

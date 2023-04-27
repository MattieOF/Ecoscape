// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "EcoscapeObject.h"
#include "ProceduralMeshComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "ProceduralFenceMesh.generated.h"

class AFenceGate;
class AEcoscapeTerrain;

UCLASS()
class ECOSCAPE_API AProceduralFenceMesh : public AEcoscapeObject
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralFenceMesh();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void Regenerate();
	
	UPROPERTY(EditAnywhere)
	USplineComponent* SplineComponent;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldGenerateGate = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDestroyable = true;

	UPROPERTY()
	AEcoscapeTerrain* AssociatedTerrain;
	
	UPROPERTY(EditAnywhere)
	UProceduralMeshComponent* ProceduralMeshComponent;

	UPROPERTY(EditAnywhere)
	TSubclassOf<AFenceGate> FenceGateClass;

	virtual void Destroyed() override;
	
#if WITH_EDITOR
	void DrawVerticies();
#endif
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	void CalculateNormals();

	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;
	
	TArray<FVector> Verticies, Normals;
	TArray<FVector2D> UV0;
	TArray<int> Indicies;
	TArray<FProcMeshTangent> Tangents;

	UPROPERTY()
	TArray<AFenceGate*> Gates;
};

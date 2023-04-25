// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "EcoscapeTerrain.h"
#include "GameFramework/Actor.h"
#include "FencePlacementPreview.generated.h"

UCLASS()
class ECOSCAPE_API AFencePlacementPreview : public AActor
{
	GENERATED_BODY()

public:
	AFencePlacementPreview();

	void DisablePreview();
	void StartPreview(FVector2D StartPos);
	void UpdatePreview(FVector2D EndPos);
	void RegeneratePreview();
	void CheckValid();

	void CreateFence() const;
	
	void CreateQuad(FVector Start, FVector End, float Thickness = 30, float ZOffset = 100);
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* ValidMaterial;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* InvalidMaterial;
	
	UPROPERTY(EditAnywhere)
	UProceduralMeshComponent* ProceduralMesh;

	TArray<FVector> Verticies;
	TArray<int>     Indicies;

	UPROPERTY()
	AEcoscapeTerrain* Terrain;
	
	FVector2D StartLocation, EndLocation;

	bool bEnabled = false;
	bool bValid   = false;
};

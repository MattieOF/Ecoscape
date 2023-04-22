// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "ProceduralMeshComponent.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "ProceduralFenceMesh.generated.h"

UCLASS()
class ECOSCAPE_API AProceduralFenceMesh : public AActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralFenceMesh();

	UFUNCTION(BlueprintCallable, CallInEditor)
	void Regenerate();
	
	UPROPERTY(EditAnywhere)
	USplineComponent* SplineComponent;
	
#if WITH_EDITOR
	void DrawVerticies();
#endif
	
protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	FORCEINLINE void AddTriangle(int V1, int V2, int V3)
	{
		Indicies.Add(V1);
		Indicies.Add(V2);
		Indicies.Add(V3);
	}
	void AddCuboid(FVector Center, FVector Extents, bool OverrideUV = false, FVector2D UV = FVector2D());
	void AddCuboid(FVector Start, FVector End, FVector2D Extents, bool OverrideUV = false, FVector2D UV = FVector2D());

	void CalculateNormals();

	UPROPERTY(EditAnywhere)
	UMaterialInterface* Material;
	
	UPROPERTY(EditAnywhere)
	UProceduralMeshComponent* ProceduralMeshComponent;

	TArray<FVector> Verticies, Normals;
	TArray<FVector2D> UV0;
	TArray<int> Indicies;
	TArray<FProcMeshTangent> Tangents;
};

// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "Fence.generated.h"

class USplineMeshComponent;

UCLASS()
class ECOSCAPE_API AFence : public AActor
{
	GENERATED_BODY()

public:
	AFence();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Length = 400;

	UFUNCTION(BlueprintCallable, CallInEditor)
	void GenerateMesh();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USplineComponent* Spline;

protected:
	virtual void OnConstruction(const FTransform& Transform) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* Mesh;
};

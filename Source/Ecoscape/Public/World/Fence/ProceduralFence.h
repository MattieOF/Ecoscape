// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "DynamicMeshActor.h"
#include "Components/SplineComponent.h"
#include "GameFramework/Actor.h"
#include "ProceduralFence.generated.h"

UCLASS()
class ECOSCAPE_API AProceduralFence : public ADynamicMeshActor
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	AProceduralFence();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	USplineComponent* SplineComponent;

	UFUNCTION(BlueprintCallable, CallInEditor)
	void Generate();

private:
	virtual void OnConstruction(const FTransform& Transform) override;
};

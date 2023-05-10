// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "PlacedItem.h"
#include "Components/ActorComponent.h"
#include "StagedItemComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FOnItemGrow, APlacedItem*, Item, int, NewStage);

UCLASS(ClassGroup=(Ecoscape), meta=(BlueprintSpawnableComponent), Within=PlacedItem)
class ECOSCAPE_API UStagedItemComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UStagedItemComponent();

	virtual void BeginPlay() override;
	
	virtual void TickComponent(float DeltaTime, ELevelTick TickType,
	                           FActorComponentTickFunction* ThisTickFunction) override;

	UFUNCTION(BlueprintCallable)
	void SetStage(int Stage, float NewGrowthTime = -1);
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	int CurrentStage = 0;

	UPROPERTY(BlueprintReadWrite)
	float GrowthTimer = 0;

	UPROPERTY(BlueprintAssignable)
	FOnItemGrow OnGrow;

protected:
	UPROPERTY(BlueprintReadWrite)
	APlacedItem* Item;
};

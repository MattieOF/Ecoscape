// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "EcoscapeObject.h"
#include "PlaceableItemData.h"
#include "GameFramework/Actor.h"
#include "PlacedItem.generated.h"

class UNavModifierComponent;
class AEcoscapeTerrain;
class UStagedItemComponent;

UCLASS()
class ECOSCAPE_API APlacedItem : public AEcoscapeObject
{
	GENERATED_BODY()

public:
	APlacedItem();

	UFUNCTION(BlueprintCallable)
	void SetItemData(UPlaceableItemData* NewItem);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UPlaceableItemData* GetItemData() const { return ItemData; }

	UFUNCTION(BlueprintCallable, meta=(WorldContext=World))
	static APlacedItem* SpawnItem(UWorld* World, UPlaceableItemData* ItemData, 
		FVector Position, FVector Scale = FVector(1, 1, 1), FRotator Rotation = FRotator::ZeroRotator);

	UPROPERTY(BlueprintReadOnly)
	AEcoscapeTerrain* AssociatedTerrain;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UStaticMeshComponent* GetMesh() { return MainMesh; }

	UFUNCTION(BlueprintCallable)
	bool TrySetStage(int Stage, bool bIsDelta);

	UStagedItemComponent* AddStagedGrowthComponent();

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UPlaceableItemData* ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* MainMesh;

	UPROPERTY()
	UNavModifierComponent* NavModifierComponent = nullptr;
};

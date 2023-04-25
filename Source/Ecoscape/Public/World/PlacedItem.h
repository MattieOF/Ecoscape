// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "EcoscapeObject.h"
#include "PlaceableItemData.h"
#include "GameFramework/Actor.h"
#include "PlacedItem.generated.h"

class AEcoscapeTerrain;

UCLASS()
class ECOSCAPE_API APlacedItem : public AEcoscapeObject
{
	GENERATED_BODY()

public:
	APlacedItem();

	UFUNCTION(BlueprintCallable)
	void SetItemData(UPlaceableItemData* NewItem);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UPlaceableItemData* GetItemData() { return ItemData; }

	UFUNCTION(BlueprintCallable, meta=(WorldContext=World))
	static APlacedItem* SpawnItem(UWorld* World, UPlaceableItemData* ItemData, 
		FVector Position, FVector Scale = FVector(1, 1, 1), FRotator Rotation = FRotator::ZeroRotator);

	UPROPERTY()
	AEcoscapeTerrain* AssociatedTerrain;

protected:
	virtual void BeginPlay() override;

#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif // WITH_EDITOR
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UPlaceableItemData* ItemData;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* MainMesh;
};

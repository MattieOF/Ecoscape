// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "PlaceableItemData.h"
#include "GameFramework/Actor.h"
#include "PlacedItem.generated.h"

UCLASS()
class ECOSCAPE_API APlacedItem : public AActor
{
	GENERATED_BODY()

public:
	APlacedItem();

	UFUNCTION(BlueprintCallable)
	void SetItemData(UPlaceableItemData* NewItem);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UPlaceableItemData* GetItemData() { return ItemData; }
	
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

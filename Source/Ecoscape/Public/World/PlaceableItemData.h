// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlaceableItemData.generated.h"

class APlacedItem;

/**
 * Data for a placeable item
 */
UCLASS()
class ECOSCAPE_API UPlaceableItemData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true))
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<APlacedItem> PlacedItemClass;
};

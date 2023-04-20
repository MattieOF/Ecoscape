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
	UPlaceableItemData();
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Details")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true), Category="Basic Details")
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Placement")
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Placement")
	TSubclassOf<APlacedItem> PlacedItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Placement")
	FVector2D ScaleBounds = FVector2D(0.5, 1.75);

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Placement")
	float MaxAngle = 40;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Placement")
	float ZOffset = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Colour")
	float ColourRange = 150 * 6;

	UPROPERTY(BlueprintReadOnly, Category="Colour")
	float ColourRangeSquared;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Colour")
	FLinearColor LandColour;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Colour")
	FLinearColor MaxLandColour;
};

/**
 * Data asset containing a list of placeable item data, allowing for one selection to result in a random selection of one of many item types.
 */
UCLASS()
class ECOSCAPE_API UItemDataList : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TArray<UPlaceableItemData*> Options;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	UPlaceableItemData* GetRandomItem();
};

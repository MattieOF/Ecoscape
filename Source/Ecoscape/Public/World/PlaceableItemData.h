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
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true))
	FText Description;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMesh* Mesh;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	TSubclassOf<APlacedItem> PlacedItemClass;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FVector2D ScaleBounds = FVector2D(0.5, 1.75);

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float MaxAngle = 40;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ZOffset = 0;

	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	float ColourRange = 150 * 6;

	UPROPERTY(BlueprintReadOnly)
	float ColourRangeSquared;
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	FLinearColor LandColour;
};

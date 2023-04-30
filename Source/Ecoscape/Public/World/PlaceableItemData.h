// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Engine/DataAsset.h"
#include "PlaceableItemData.generated.h"

class AEcoscapeTerrain;
class APlacedItem;

/**
 * Data for a placeable item
 */
UCLASS(BlueprintType)
class ECOSCAPE_API UPlaceableItemData : public UDataAsset
{
	GENERATED_BODY()

public:
	UPlaceableItemData();
	UPlaceableItemData(FObjectInitializer& ObjectInitializer);

#if WITH_EDITOR
	UFUNCTION(BlueprintCallable, CallInEditor, Category = "Icon")
	void CreateIcon();

	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	void CreateValidTerrainsArray();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsValidForTerrain(AEcoscapeTerrain* Terrain);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsValidForTerrainName(FString TerrainName);
	
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Details")
	FText Name;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, meta=(MultiLine=true), Category="Basic Details")
	FText Description;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Basic Details")
	FString Categorisation = "";

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Placement")
	// Either "All" or semi-colon seperated valid terrains
	FString ValidTerrains = "All";

	UPROPERTY(BlueprintReadOnly, Category="Placement")
	TArray<FString> ValidTerrainsArray;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Icon")
	UTexture2D* Icon;

#if WITH_EDITORONLY_DATA
	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Icon")
	float IconFOV = 70;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Icon")
	FVector IconCameraOffset;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Icon")
	FRotator IconObjectRotation;

	UPROPERTY(EditAnywhere, BlueprintReadOnly, Category="Icon")
	int32 IconSize = 512;
#endif
	
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

// UNUSED
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

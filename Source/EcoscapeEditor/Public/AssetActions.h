#pragma once
	 
#include "CoreMinimal.h"
#include "AssetTypeActions_Base.h"
#include "Character/Animals/AnimalData.h"
#include "World/PlaceableItemData.h"

#define LOCTEXT_NAMESPACE "EcoscapeEditor"

extern EAssetTypeCategories::Type EcoscapeAssetCategory;

class FPlaceableItemAssetActions : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return LOCTEXT("PlaceableItem", "Placeable Item"); }
	virtual uint32 GetCategories() override { return EcoscapeAssetCategory; }
	virtual FColor GetTypeColor() const override { return FColor::Cyan; }
	virtual FText GetAssetDescription(const FAssetData &AssetData) const override { return LOCTEXT("PlaceableItemDesc", "Asset containing data for a type of placeable item."); }
	virtual UClass* GetSupportedClass() const override { return UPlaceableItemData::StaticClass(); }
};

class FAnimalDataAssetActions : public FAssetTypeActions_Base
{
public:
	virtual FText GetName() const override { return LOCTEXT("AnimalData", "Animal"); }
	virtual uint32 GetCategories() override { return EcoscapeAssetCategory; }
	virtual FColor GetTypeColor() const override { return FColor::Red; }
	virtual FText GetAssetDescription(const FAssetData &AssetData) const override { return LOCTEXT("AnimalDesc", "Asset containg data for a type of animal."); }
	virtual UClass* GetSupportedClass() const override { return UAnimalData::StaticClass(); }
};

#undef LOCTEXT_NAMESPACE

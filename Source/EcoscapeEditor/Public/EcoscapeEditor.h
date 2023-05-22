#pragma once

#include "AssetActions.h"
#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEcoscapeEditor, All, All)

extern EAssetTypeCategories::Type EcoscapeAssetCategory;

class FEcoscapeEditorModule: public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void AddMenuEntries(FMenuBuilder& MenuBuilder);
	void GeneratePlaceableData();
	void RefreshItemDir();

	TSharedPtr<FPlaceableItemAssetActions> PlaceableItemAssetActions;
	TSharedPtr<FAnimalDataAssetActions> AnimalDataAssetActions;
	TSharedPtr<FCodexEntryAssetActions> CodexEntryAssetActions;
};

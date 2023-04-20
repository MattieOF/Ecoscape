// copyright lololol

#include "EcoscapeGameInstance.h"

#include "AssetRegistry/AssetRegistryModule.h"

void UEcoscapeGameInstance::Init()
{
	Super::Init();
	
	// Get the asset registry module
	const FAssetRegistryModule& AssetRegistryModule = FModuleManager::LoadModuleChecked<FAssetRegistryModule>(
		FName("AssetRegistry"));
	IAssetRegistry& AssetRegistry = AssetRegistryModule.Get();

	// Since asset registry scanning is asyncronous, we initiate a synchronous scan 
	// of the Placeables Data directory to ensure they've all been discovered.
	TArray<FString> ContentPaths;
	ContentPaths.Add(TEXT("/Game/Blueprints/Placeables/Data/"));
	AssetRegistry.ScanPathsSynchronous(ContentPaths);

	// Find all the resource data assets and add them to the resources map
	TArray<FAssetData> ItemAssetData;
	AssetRegistry.GetAssetsByClass(UPlaceableItemData::StaticClass()->GetFName(), ItemAssetData, true);
	for (auto& Asset : ItemAssetData)
	{
		UPlaceableItemData* Item = Cast<UPlaceableItemData>(Asset.GetAsset());
		ItemTypes.Add(Item->GetName(), Item);
	}
}

#include "EcoscapeEditor.h"

#include "EcoscapeGameInstance.h"
#include "LevelEditor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Modules/ModuleManager.h"
#include "Modules/ModuleInterface.h"
#include "Styling/SlateIconFinder.h"
#include "World/PlaceableItemData.h"

IMPLEMENT_GAME_MODULE(FEcoscapeEditorModule, MyGameEditor);

DEFINE_LOG_CATEGORY(LogEcoscapeEditor);

#define LOCTEXT_NAMESPACE "EcoscapeEditor"

void FEcoscapeEditorModule::StartupModule()
{
	UE_LOG(LogEcoscapeEditor, Warning, TEXT("EcoscapeEditor: Log Started"));

	FLevelEditorModule& LevelEditorModule = FModuleManager::LoadModuleChecked<FLevelEditorModule>("LevelEditor");
	TSharedPtr<FExtensibilityManager> ExtensibilityManager = LevelEditorModule.GetMenuExtensibilityManager();

	// Add a new menu item
	auto Extender = MakeShareable(new FExtender());
	Extender.Object->AddMenuExtension("Tools", EExtensionHook::After, nullptr, FMenuExtensionDelegate::CreateRaw(this, &FEcoscapeEditorModule::AddMenuEntries));
	ExtensibilityManager->AddExtender(Extender);
}

void FEcoscapeEditorModule::ShutdownModule()
{
	UE_LOG(LogEcoscapeEditor, Warning, TEXT("EcoscapeEditor: Log Ended"));
}

void FEcoscapeEditorModule::AddMenuEntries(FMenuBuilder& MenuBuilder)
{
	MenuBuilder.BeginSection("EcoscapeSection", FText::FromString("Ecoscape"));
	{
		MenuBuilder.AddMenuEntry(LOCTEXT("EcoscapeTool_GenIcons", "Generate Icons"),
			LOCTEXT("EcoscapeTool_GenIcons_Desc", "Generate icons for all placeable item types"),
			FSlateIconFinder::FindIcon("ClassIcon.Texture2D"),
			FUIAction(FExecuteAction::CreateRaw(this, &FEcoscapeEditorModule::GenIcons)));
		// MenuBuilder.AddMenuEntry(LOCTEXT("EcoscapeTool_RefreshItemDir", "Refresh Item Directory"),
		// 	LOCTEXT("EcoscapeTool_RefreshItemDir_Desc", "Refresh the GameInstance's item directory"),
		// 	FSlateIconFinder::FindIcon("SourceControl.Actions.Refresh"),
		// 	FUIAction(FExecuteAction::CreateRaw(this, &FEcoscapeEditorModule::RefreshItemDir)));
	}
	MenuBuilder.EndSection();
}

void FEcoscapeEditorModule::GenIcons()
{
	UE_LOG(LogEcoscapeEditor, Log, TEXT("Generating icons for all item types"));
	
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
    		Item->CreateIcon();
    	}
}

void FEcoscapeEditorModule::RefreshItemDir()
{
	UWorld* EditorWorld = GEditor->GetEditorWorldContext(false).World();
	UEcoscapeGameInstance* GI = Cast<UEcoscapeGameInstance>(EditorWorld->GetGameInstance());
	if (!GI)
	{
		UE_LOG(LogEcoscapeEditor, Error, TEXT("Game Instance is null in RefreshItemDir()"));
		return;
	}
	GI->GenerateItemDirectory();
}

#undef LOCTEXT_NAMESPACE

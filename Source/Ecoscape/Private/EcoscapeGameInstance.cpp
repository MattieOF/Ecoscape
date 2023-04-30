// copyright lololol

#include "EcoscapeGameInstance.h"

#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Character/EcoscapePlayerController.h"
#include "Kismet/KismetStringLibrary.h"
#include "World/EcoscapeTerrain.h"

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

	GenerateItemDirectory();
}

UTextPopup* UEcoscapeGameInstance::ShowPopup(FString Title, FString Message)
{
	UTextPopup* Popup = CreateWidget<UTextPopup>(this, TextPopupClass);
	Popup->Show(Title, Message);
	Popup->AddToViewport();
	return Popup;
}

AEcoscapeTerrain* UEcoscapeGameInstance::GetTerrain(FString TerrainName)
{
	TArray<AActor*> Terrains;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEcoscapeTerrain::StaticClass(), Terrains);

	for (AActor* TerrainActor : Terrains)
	{
		AEcoscapeTerrain* Terrain = Cast<AEcoscapeTerrain>(TerrainActor);
		if (Terrain->TerrainName == TerrainName)
			return Terrain;
	}
	
	UE_LOG(LogEcoscape, Error, TEXT("In UEcoscapeGameInstance::GetTerrain, failed to find terrain named %s"), *TerrainName);
	return nullptr;
}

void UEcoscapeGameInstance::SaveTerrain(const FString& TerrainName, const FString& Filename) const
{
	TArray<AActor*> Terrains;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEcoscapeTerrain::StaticClass(), Terrains);

	for (AActor* TerrainActor : Terrains)
	{
		AEcoscapeTerrain* Terrain = Cast<AEcoscapeTerrain>(TerrainActor);
		if (Terrain->TerrainName != TerrainName)
			continue;
		Terrain->SerialiseTerrainToFile(Filename);
		UE_LOG(LogEcoscape, Log, TEXT("Saved terrain %s to %s"), *TerrainName, *Filename);
	}
}

void UEcoscapeGameInstance::LoadTerrain(const FString& TerrainName, const FString& Filename) const
{
	TArray<AActor*> Terrains;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEcoscapeTerrain::StaticClass(), Terrains);

	for (AActor* TerrainActor : Terrains)
	{
		AEcoscapeTerrain* Terrain = Cast<AEcoscapeTerrain>(TerrainActor);
		if (Terrain->TerrainName != TerrainName)
			continue;
		Terrain->DeserialiseTerrainFromFile(Filename);
		AEcoscapePlayerController* Player = AEcoscapePlayerController::GetEcoscapePlayerController(GetWorld());
		if (Player->GetCurrentTerrain() == Terrain)
			Player->GoToTerrain(Terrain); // Reposition player on new terrain
		UE_LOG(LogEcoscape, Log, TEXT("Loaded terrain %s from %s"), *TerrainName, *Filename);
	}
}

void UEcoscapeGameInstance::RegenerateTerrain(const FString& TerrainName) const
{
	TArray<AActor*> Terrains;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEcoscapeTerrain::StaticClass(), Terrains);

	for (AActor* TerrainActor : Terrains)
	{
		AEcoscapeTerrain* Terrain = Cast<AEcoscapeTerrain>(TerrainActor);
		if (Terrain->TerrainName != TerrainName)
			continue;
		Terrain->Regenerate();
		AEcoscapePlayerController* Player = AEcoscapePlayerController::GetEcoscapePlayerController(GetWorld());
		if (Player->GetCurrentTerrain() == Terrain)
			Player->GoToTerrain(Terrain); // Reposition player on new terrain
		UE_LOG(LogEcoscape, Log, TEXT("Regenerated terrain %s"), *TerrainName);
	}
}

void UEcoscapeGameInstance::GenerateItemDirectory()
{
	const double Start = FPlatformTime::Seconds();
	
	// Initialise item directory
	if (ItemDirectory)
		ItemDirectory->Empty();
	else
		ItemDirectory = NewObject<UItemDirectory>(this);
	
	for (const auto& Item : ItemTypes)
		ItemDirectory->AddItem(Item.Value);

	ItemDirectory->RootFolder->GenValidTerrains();

	const double End = FPlatformTime::Seconds();
	UE_LOG(LogEcoscape, Log, TEXT("Took %fs to generate item directory"), End - Start);
}

void UEcoscapeGameInstance::PrintItemDirectory()
{
	FString Directory = "";
	SearchItemDirFolder(ItemDirectory->RootFolder, Directory, 0);
	ShowPopup("Item Directory", Directory);
}

void UEcoscapeGameInstance::AddWithIndent(FString& Output, FString Message, int Indent, bool NewLine)
{
	for (int i = 0; i < Indent; i++)
		Output += "    ";
	Output += Message;
	if (NewLine)
		Output += "\n";
}

void UEcoscapeGameInstance::SearchItemDirFolder(UItemFolder* Folder, FString& Output, int Level)
{
	AddWithIndent(Output, FString::Printf(TEXT("%s - Valid for %s"), *Folder->Name.ToString(),
	                                      Folder->bValidForAllTerrains
		                                      ? *FString("all")
		                                      : *UEcoscapeStatics::JoinStringArray(Folder->ValidTerrains, ", ")),
	              Level);

	if (Folder->Folders.Num() == 0 && Folder->Items.Num() == 0)
	{
		AddWithIndent(Output, "<EMPTY>", Level + 1);
		return;
	}
	
	for (const auto& NestedFolder : Folder->Folders)
		SearchItemDirFolder(NestedFolder.Value, Output, Level + 1);

	for (const UPlaceableItemData* Item : Folder->Items)
		AddWithIndent(Output, FString::Printf(
			              TEXT("%ls (%ls) - Valid for %s"), *Item->Name.ToString(), *Item->GetName(),
			              Item->ValidTerrains == "All"
				              ? *FString("all")
				              : *UEcoscapeStatics::JoinStringArray(Item->ValidTerrainsArray, ", ")), Level + 1);
}

void UItemFolder::GenValidTerrains()
{
	// Recursive method. Sucks, but only on init, so probably ok.

	for (const auto& Folder : Folders)
	{
		Folder.Value->GenValidTerrains();
		
		if (Folder.Value->bValidForAllTerrains)
		{
			bValidForAllTerrains = true;
			return;
		}

		for (const auto& TerrainName : Folder.Value->ValidTerrains)
			if (!ValidTerrains.Contains(TerrainName))
				ValidTerrains.Add(TerrainName);
	}

	for (const auto Item : Items)
	{
		if (Item->ValidTerrains == "All")
		{
			bValidForAllTerrains = true;
			return;	
		}

		for (const auto& TerrainName : Item->ValidTerrainsArray)
			if (!ValidTerrains.Contains(TerrainName))
				ValidTerrains.Add(TerrainName);
	}
}

UItemDirectory::UItemDirectory()
{
	RootFolder = CreateDefaultSubobject<UItemFolder>("RootFolder");
	RootFolder->Name = FText::FromString("Item Directory");
}

void UItemDirectory::AddItem(UPlaceableItemData* Item)
{
	TArray<FString> Folders = UKismetStringLibrary::ParseIntoArray(Item->Categorisation, "/", true);

	if (Folders.IsEmpty())
	{
		RootFolder->Items.Add(Item);
	}
	else
	{
		UItemFolder* CurrentFolder = RootFolder;
		for (const auto& FolderName : Folders)
		{
			if (!CurrentFolder->Folders.Contains(FolderName))
			{
				UItemFolder* Folder = NewObject<UItemFolder>(CurrentFolder);
				Folder->Name = FText::FromString(FolderName);
				CurrentFolder->Folders.Add(FolderName, Folder);
			}
			CurrentFolder = CurrentFolder->Folders[FolderName];
		}
		CurrentFolder->Items.Add(Item);
	}
}

void UItemDirectory::Empty()
{
	RootFolder->ConditionalBeginDestroy();
	RootFolder = NewObject<UItemFolder>(this);
	RootFolder->Name = FText::FromString("Item Directory");
}

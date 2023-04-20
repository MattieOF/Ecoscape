﻿// copyright lololol

#include "EcoscapeGameInstance.h"

#include "EcoscapeLog.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Character/EcoscapePlayerController.h"
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
}

AEcoscapeTerrain* UEcoscapeGameInstance::GetTerrain(FString TerrainName)
{
	TArray<AActor*> Terrains;
	UGameplayStatics::GetAllActorsOfClass(GetWorld(), AEcoscapeTerrain::StaticClass(), Terrains);

	for (AActor* TerrainActor : Terrains)
	{
		AEcoscapeTerrain* Terrain = Cast<AEcoscapeTerrain>(TerrainActor);
		if (Terrain->DebugName == TerrainName)
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
		if (Terrain->DebugName != TerrainName)
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
		if (Terrain->DebugName != TerrainName)
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
		if (Terrain->DebugName != TerrainName)
			continue;
		Terrain->Regenerate();
		AEcoscapePlayerController* Player = AEcoscapePlayerController::GetEcoscapePlayerController(GetWorld());
		if (Player->GetCurrentTerrain() == Terrain)
			Player->GoToTerrain(Terrain); // Reposition player on new terrain
		UE_LOG(LogEcoscape, Log, TEXT("Regenerated terrain %s"), *TerrainName);
	}
}

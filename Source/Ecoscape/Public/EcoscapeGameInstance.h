// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UI/TextPopup.h"
#include "World/PlaceableItemData.h"
#include "EcoscapeGameInstance.generated.h"

class UCodexEntry;
class UAnimalData;

UCLASS(BlueprintType)
class UItemFolder : public UObject
{
	GENERATED_BODY()

public:
	UPROPERTY(BlueprintReadOnly)
	FText Name;

	UPROPERTY(BlueprintReadOnly)
	TMap<FString, UItemFolder*> Folders;
	UPROPERTY(BlueprintReadOnly)
	TArray<UPlaceableItemData*> Items;

	UPROPERTY(BlueprintReadOnly)
	TArray<FString> ValidTerrains;

	UPROPERTY(BlueprintReadOnly)
	bool bValidForAllTerrains = false;

	void GenValidTerrains();

	UFUNCTION(BlueprintCallable)
	UPlaceableItemData* GetRandomItem(const FString& TerrainName);
	
	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsValidForTerrain(const FString& TerrainName) const { return bValidForAllTerrains || ValidTerrains.Contains(TerrainName); }
};

UCLASS(BlueprintType)
class UItemDirectory : public UObject
{
	GENERATED_BODY()

public:
	UItemDirectory();
	
	UPROPERTY(BlueprintReadOnly)
	UItemFolder* RootFolder;

	UFUNCTION(BlueprintCallable)
	void AddItem(UPlaceableItemData* Item);
	UFUNCTION(BlueprintCallable)
	void Empty();
};

/**
 * Game instance class for Ecoscape. Contains things like a map of placeable items
 */
UCLASS()
class ECOSCAPE_API UEcoscapeGameInstance : public UGameInstance
{
	GENERATED_BODY()

public:
	virtual void Init() override;

	UPROPERTY(EditAnywhere)
	TSubclassOf<UTextPopup> TextPopupClass;

	UFUNCTION(BlueprintCallable)
	UTextPopup* ShowPopup(FString Title, FString Message);
	
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, UPlaceableItemData*> ItemTypes;
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, UAnimalData*> AnimalTypes;
	UPROPERTY(BlueprintReadOnly)
	TMap<FString, UCodexEntry*> CodexEntries;

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE UCodexEntry* GetCodexEntry(FString EntryName) { return CodexEntries.Contains(EntryName) ? CodexEntries[EntryName] : nullptr; }
	
	UPROPERTY(BlueprintReadOnly)
	UItemDirectory* ItemDirectory;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContext"))
	static FORCEINLINE UEcoscapeGameInstance* GetEcoscapeGameInstance(UObject* WorldContext)
	{
		return Cast<UEcoscapeGameInstance>(UGameplayStatics::GetGameInstance(WorldContext));
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AEcoscapeTerrain* GetTerrain(FString TerrainName);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	TArray<AEcoscapeTerrain*> GetAllTerrains();

	UFUNCTION(BlueprintCallable)
	int AddTerrain(AEcoscapeTerrain* Terrain);
	UFUNCTION(BlueprintCallable, BlueprintPure)
	AEcoscapeTerrain* GetTerrainFromIndex(int Index);

	UFUNCTION(Exec)
	void SaveTerrain(const FString& TerrainName, const FString& Filename) const;
	UFUNCTION(Exec)
	void LoadTerrain(const FString& TerrainName, const FString& Filename) const;
	UFUNCTION(Exec)
	void RegenerateTerrain(const FString& TerrainName) const;

	UFUNCTION(Exec, BlueprintCallable)
	void GenerateItemDirectory();

	UFUNCTION(Exec, BlueprintCallable)
	void PrintItemDirectory();

	UFUNCTION(Exec)
	void SetTimeDilation(float NewTimeDilation) const;

private:
	void AddWithIndent(FString& Output, FString Message, int Indent, bool NewLine = true);
	void SearchItemDirFolder(UItemFolder* Folder, FString& Output, int Level);

	int CurrentTerrainIndex = 0;
	UPROPERTY()
	TMap<int, AEcoscapeTerrain*> Terrains;
	UPROPERTY()
	TMap<FString, AEcoscapeTerrain*> TerrainNameMap;
};

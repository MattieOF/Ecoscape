// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Engine/GameInstance.h"
#include "Kismet/GameplayStatics.h"
#include "UI/TextPopup.h"
#include "World/PlaceableItemData.h"
#include "EcoscapeGameInstance.generated.h"

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

	void AddItem(UPlaceableItemData* Item);
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
	UItemDirectory* ItemDirectory;

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContext"))
	static FORCEINLINE UEcoscapeGameInstance* GetEcoscapeGameInstance(UObject* WorldContext)
	{
		return Cast<UEcoscapeGameInstance>(UGameplayStatics::GetGameInstance(WorldContext));
	}

	UFUNCTION(BlueprintCallable, BlueprintPure)
	AEcoscapeTerrain* GetTerrain(FString TerrainName);

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

private:
	void AddWithIndent(FString& Output, FString Message, int Indent, bool NewLine = true);
	void SearchItemDirFolder(UItemFolder* Folder, FString& Output, int Level);
};

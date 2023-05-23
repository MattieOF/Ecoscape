// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataAsset.h"
#include "Codex.generated.h"

/**
 * Data representing a codex entry
 */
UCLASS(BlueprintType)
class ECOSCAPE_API UCodexEntry : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText Title;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText ShortDescription;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, meta = (MultiLine = true))
	FText Content;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bDismissable = true;
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bShouldNotify = true;
};

UCLASS(Blueprintable)
class UCodexUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void Show();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void Hide();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void ShowEntry(UCodexEntry* Entry);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void UpdateEntries();
};

UCLASS(Blueprintable)
class UCodexFeedUI : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void AddUnlock(UCodexEntry* Entry);

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void OpenLatest();

	UFUNCTION(BlueprintCallable, BlueprintImplementableEvent)
	void TryDismissLatest();
};

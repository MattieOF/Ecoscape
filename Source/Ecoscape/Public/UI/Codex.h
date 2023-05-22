﻿// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "Engine/DataAsset.h"
#include "Codex.generated.h"

/**
 * Data representing a codex entry
 */
UCLASS()
class ECOSCAPE_API UCodexEntry : public UDataAsset
{
	GENERATED_BODY()

public:
	UPROPERTY(EditAnywhere)
	FText Title;
	UPROPERTY(EditAnywhere, meta = (MultiLine = true))
	FText Content;
	UPROPERTY(EditAnywhere)
	bool bDismissable = true;
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
};
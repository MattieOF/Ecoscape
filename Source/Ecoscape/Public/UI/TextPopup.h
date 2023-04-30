// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "TextPopup.generated.h"

/**
 * Base widget class for a text popup
 */
UCLASS()
class ECOSCAPE_API UTextPopup : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void Show(const FString& Title, const FString& Message);
};

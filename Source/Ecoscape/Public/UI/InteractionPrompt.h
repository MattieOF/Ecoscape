// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Blueprint/UserWidget.h"
#include "InteractionPrompt.generated.h"

class UInteractableComponent;

/**
 * UI widget used to prompt user for interactions
 */
UCLASS()
class ECOSCAPE_API UInteractionPrompt : public UUserWidget
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void ShowPrompt(UInteractableComponent* InteractableComponent);

	UFUNCTION(BlueprintImplementableEvent)
	void HidePrompt();

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsPromptVisible() { return bPromptVisible; }

protected:
	UPROPERTY(BlueprintReadWrite)
	bool bPromptVisible = false;
};

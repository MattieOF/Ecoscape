// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Character/Animals/BaseAnimal.h"
#include "AnimalUI.generated.h"

UINTERFACE(Blueprintable)
class UAnimalUI : public UInterface
{
	GENERATED_BODY()
};

class IAnimalUI
{
	GENERATED_BODY()

public:
	UFUNCTION(BlueprintImplementableEvent)
	void Show();
	UFUNCTION(BlueprintImplementableEvent)
	void Hide();
	UFUNCTION(BlueprintImplementableEvent)
	void SetAnimal(ABaseAnimal* Animal);
};

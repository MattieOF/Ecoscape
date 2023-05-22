// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "World/InteractableComponent.h"
#include "SickAnimalInteractionComponent.generated.h"

class ABaseAnimal;

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ECOSCAPE_API USickAnimalInteractionComponent : public UInteractableComponent
{
	GENERATED_BODY()

public:
	USickAnimalInteractionComponent();

	virtual void BeginPlay() override;

	UFUNCTION()
	void OnInteraction();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	ABaseAnimal* Animal;
};

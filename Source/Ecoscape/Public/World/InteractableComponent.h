// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "InteractableComponent.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE(FOnInteractedWith);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class ECOSCAPE_API UInteractableComponent : public UActorComponent
{
	GENERATED_BODY()

public:
	UInteractableComponent();

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FText InteractionName = FText::FromString("Interact");
	
	void OnInteract();

	UPROPERTY(EditAnywhere, BlueprintAssignable)
	FOnInteractedWith OnInteractedWith;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bCanInteract = true;
};

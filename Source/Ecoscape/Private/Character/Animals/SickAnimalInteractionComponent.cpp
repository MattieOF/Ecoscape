// copyright lololol

#include "Character/Animals/SickAnimalInteractionComponent.h"

#include "Ecoscape.h"
#include "Character/Animals/BaseAnimal.h"

#if WITH_EDITOR
#include "MessageLogModule.h"
#endif

USickAnimalInteractionComponent::USickAnimalInteractionComponent()
{
	PrimaryComponentTick.bCanEverTick = true;

	InteractionName = FText::FromString("Give Medicine");
	bCanInteract = false;
}

void USickAnimalInteractionComponent::BeginPlay()
{
	Super::BeginPlay();

	Animal = Cast<ABaseAnimal>(GetOwner());
	if (!Animal)
	{
		ECO_LOG_ERROR("USickAnimalInteractionComponent is on a non animal actor!");
		DestroyComponent();
		return;
	}
	
	FScriptDelegate Delegate;
	Delegate.BindUFunction(this, "OnInteraction");
	OnInteractedWith.Add(Delegate);
}

void USickAnimalInteractionComponent::OnInteraction()
{
	Animal->GiveMedicine();
	bCanInteract = false;
}

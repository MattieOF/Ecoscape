// copyright lololol

#include "World/InteractableComponent.h"

UInteractableComponent::UInteractableComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UInteractableComponent::OnInteract()
{
	OnInteractedWith.Broadcast();
}

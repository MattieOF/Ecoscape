// copyright lolol

#include "World//OutlineComponent.h"

// Sets default values for this component's properties
UOutlineComponent::UOutlineComponent()
{
	PrimaryComponentTick.bCanEverTick = false;
}

void UOutlineComponent::ShowOutline()
{
	for (const auto Component : OutlinedComponents)
	{
		if (Component) // Component seems to be occasionally null?
			Component->SetRenderCustomDepth(true);
	}
}

void UOutlineComponent::HideOutline()
{
	for (const auto Component : OutlinedComponents)
	{
		if (Component) // Component seems to be occasionally null?
			Component->SetRenderCustomDepth(false);
	}
}

void UOutlineComponent::RefreshOutlinedComponents()
{
	OutlinedComponents.Empty();
	for (const auto Component : GetOwner()->GetComponentsByTag(UPrimitiveComponent::StaticClass(), OutlineTag))
		OutlinedComponents.Add(Cast<UPrimitiveComponent>(Component));
}

// Called when the game starts
void UOutlineComponent::BeginPlay()
{
	Super::BeginPlay();
	RefreshOutlinedComponents();
}

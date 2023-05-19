// copyright lololol

#include "World/StagedItemComponent.h"

#include "Ecoscape.h"
#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "MessageLogModule.h"

UStagedItemComponent::UStagedItemComponent()
{
	PrimaryComponentTick.bCanEverTick = true;
}

void UStagedItemComponent::BeginPlay()
{
	Item = Cast<APlacedItem>(GetOwner());
	if (Item->GetItemData()->StageMeshes.Num() == 0)
	{
		ECO_LOG_ERROR(
			FString::Printf(TEXT("Staged item component attached to item (%s, item %s) with no stages!"), *GetOwner()->
				GetName(), *Item->GetItemData()->Name.ToString()));
		return;
	}

	CurrentStage = 0;
	Item->GetMesh()->SetStaticMesh(Item->GetItemData()->StageMeshes[CurrentStage]);
	if (Item->GetItemData()->StageMeshes.Num() == 1)
		GrowthTimer = -1;
	else
		GrowthTimer = FMath::FRandRange(Item->GetItemData()->GrowthTimeRangeSecs.X, Item->GetItemData()->GrowthTimeRangeSecs.Y);
}

// Called every frame
void UStagedItemComponent::TickComponent(float DeltaTime, ELevelTick TickType,
                                         FActorComponentTickFunction* ThisTickFunction)
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GrowthTimer > 0)
	{
		GrowthTimer -= DeltaTime;
		if (GrowthTimer <= 0)
			SetStage(CurrentStage + 1);
	}
}

void UStagedItemComponent::SetStage(int Stage, float NewGrowthTime)
{
	UPlaceableItemData* ItemData = Item->GetItemData();
	
	if (Stage >= ItemData->StageMeshes.Num())
	{
		UE_LOG(LogEcoscape, Error, TEXT("StagedItem (%s) tried to grow above the amount of stages it has"), *GetName());
		return;
	}

	CurrentStage = Stage;
	Item->GetMesh()->SetStaticMesh(ItemData->StageMeshes[CurrentStage]);
	
	// Reposition if it can
	FHitResult Hit;
	if (GetWorld()->LineTraceSingleByChannel(Hit, Item->GetActorLocation() + FVector(0, 0, 1000), Item->GetActorLocation() + FVector(0, 0, -1000), ECC_ITEM_PLACEABLE_ON))
		Item->SetActorLocation(Hit.ImpactPoint + FVector(0, 0, UEcoscapeStatics::GetZUnderOrigin(Item)) + ItemData->ZOffset);

	// Reroll growth time if suitable
	if (Stage == ItemData->StageMeshes.Num() - 1)
		GrowthTimer = -1;
	else if (NewGrowthTime >= 0)
		GrowthTimer = NewGrowthTime;
	else
		GrowthTimer = FMath::FRandRange(ItemData->GrowthTimeRangeSecs.X, ItemData->GrowthTimeRangeSecs.Y);
	
	OnGrow.Broadcast(Item, Stage);
}

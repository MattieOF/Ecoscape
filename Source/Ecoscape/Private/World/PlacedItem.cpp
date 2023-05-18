// copyright lololol

#include "World/PlacedItem.h"

#include "Ecoscape.h"
#include "EcoscapeStatics.h"
#include "MessageLogModule.h"
#include "World/StagedItemComponent.h"

APlacedItem::APlacedItem()
{
	PrimaryActorTick.bCanEverTick = false;
	
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Main Mesh"));
	MainMesh->SetCollisionResponseToChannel(ECC_BLOCKS_ITEM_PLACEMENT, ECR_Block);
	MainMesh->SetCollisionResponseToChannel(ECC_BLOCKS_HABITAT, ECR_Block);
	MainMesh->SetCollisionResponseToChannel(ECC_HIGHLIGHTABLE, ECR_Block);
	MainMesh->ComponentTags.Add("Outline");

	RootComponent = MainMesh;
}

void APlacedItem::SetItemData(UPlaceableItemData* NewItem)
{
	ItemData = NewItem;
	MainMesh->SetStaticMesh(NewItem->Mesh);
}

APlacedItem* APlacedItem::SpawnItem(UWorld* World, UPlaceableItemData* ItemData, const FVector Position, const FVector Scale, const FRotator Rotation)
{
	if (!ItemData || !ItemData->PlacedItemClass)
	{
		ECO_LOG_ERROR("Attempted to spawn item with invalid item data");
		return nullptr;
	}
	
	APlacedItem* PlacedItem = World->SpawnActor<APlacedItem>(ItemData->PlacedItemClass, Position, Rotation);
	PlacedItem->SetActorScale3D(Scale);
	PlacedItem->SetItemData(ItemData);

	if (ItemData->StagedGrowth)
		PlacedItem->AddStagedGrowthComponent();
	
	return PlacedItem;
}

UStagedItemComponent* APlacedItem::AddStagedGrowthComponent()
{
	return Cast<UStagedItemComponent>(AddComponentByClass(UStagedItemComponent::StaticClass(), false, FTransform::Identity, false));
}

void APlacedItem::BeginPlay()
{
	Super::BeginPlay();

	if (ItemData != nullptr)
		SetItemData(ItemData);
}

#if WITH_EDITOR
void APlacedItem::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property->GetName() == "ItemData")
		SetItemData(ItemData);
}
#endif // WITH_EDITOR

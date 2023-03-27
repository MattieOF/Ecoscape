// copyright lololol

#include "World/PlacedItem.h"

#include "EcoscapeStatics.h"

APlacedItem::APlacedItem()
{
	PrimaryActorTick.bCanEverTick = false;
	
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Main Mesh"));
	MainMesh->SetCollisionResponseToChannel(ECC_BLOCKS_ITEM_PLACEMENT, ECR_Block);
	RootComponent = MainMesh;
}

void APlacedItem::SetItemData(UPlaceableItemData* NewItem)
{
	ItemData = NewItem;
	MainMesh->SetStaticMesh(NewItem->Mesh);
}

APlacedItem* APlacedItem::SpawnItem(UWorld* World, UPlaceableItemData* ItemData, FVector Position, FVector Scale, FRotator Rotation)
{
	APlacedItem* PlacedItem = World->SpawnActor<APlacedItem>(ItemData->PlacedItemClass, Position, Rotation);
	PlacedItem->SetActorScale3D(Scale);
	PlacedItem->SetItemData(ItemData);
	return PlacedItem;
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

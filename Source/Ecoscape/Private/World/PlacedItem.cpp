// copyright lololol

#include "World/PlacedItem.h"

APlacedItem::APlacedItem()
{
	PrimaryActorTick.bCanEverTick = false;
	
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Main Mesh"));
	RootComponent = MainMesh;
}

void APlacedItem::SetItemData(UPlaceableItemData* NewItem)
{
	ItemData = NewItem;
	MainMesh->SetStaticMesh(NewItem->Mesh);
}

// Called when the game starts or when spawned
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

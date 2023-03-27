// copyright lololol

#include "World/PlacedItem.h"

#include "EcoscapeStatics.h"

APlacedItem::APlacedItem()
{
	PrimaryActorTick.bCanEverTick = false;
	
	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Main Mesh"));
	MainMesh->SetCollisionResponseToChannel(ECC_BLOCKS_ITEM_PLACEMENT, ECR_Block);
	MainMesh->ComponentTags.Add("Outline");

	Outline = CreateDefaultSubobject<UOutlineComponent>(TEXT("Outline"));
	
	RootComponent = MainMesh;
}

void APlacedItem::SetItemData(UPlaceableItemData* NewItem)
{
	ItemData = NewItem;
	MainMesh->SetStaticMesh(NewItem->Mesh);
}

APlacedItem* APlacedItem::SpawnItem(UWorld* World, UPlaceableItemData* ItemData, const FVector Position, const FVector Scale, const FRotator Rotation)
{
	APlacedItem* PlacedItem = World->SpawnActor<APlacedItem>(ItemData->PlacedItemClass, Position, Rotation);
	PlacedItem->SetActorScale3D(Scale);
	PlacedItem->SetItemData(ItemData);
	return PlacedItem;
}

void APlacedItem::OnMouseOver() const
{
	Outline->ShowOutline();
}

void APlacedItem::OnMouseLeave() const
{
	Outline->HideOutline();
}

void APlacedItem::BeginPlay()
{
	Super::BeginPlay();

	FScriptDelegate MouseOver;
	MouseOver.BindUFunction(this, "OnMouseOver");
	MainMesh->OnBeginCursorOver.Add(MouseOver);
	FScriptDelegate MouseLeave;
	MouseLeave.BindUFunction(this, "OnMouseLeave");
	MainMesh->OnEndCursorOver.Add(MouseLeave);
	
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

// copyright lololol

#include "World/PlaceableItemPreview.h"

#include "EcoscapeStatics.h"
#include "EcoscapeStats.h"

DECLARE_CYCLE_STAT(TEXT("Move Item Preview"), STAT_MovePreview, STATGROUP_Ecoscape);

APlaceableItemPreview::APlaceableItemPreview()
{
	PrimaryActorTick.bCanEverTick = false;

	MainMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("Mesh"));
	MainMesh->SetMobility(EComponentMobility::Movable);
	MainMesh->SetCollisionResponseToAllChannels(ECR_Ignore);
	RootComponent = MainMesh;
}

void APlaceableItemPreview::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);
	
	// Update rotation
	if (CurrentRotationAlpha < 1)
	{
		CurrentRotationAlpha += (1 - CurrentRotationAlpha) * 10 * DeltaSeconds;
		if (1 - CurrentRotationAlpha < 0.02)
			CurrentRotationAlpha = 1;

		SetActorRotation(FMath::Lerp(GetActorRotation(), FRotator(0, TargetItemRotation, 0), CurrentRotationAlpha));
	}
}

void APlaceableItemPreview::SetItem(UPlaceableItemData* Item)
{
	MainMesh->SetStaticMesh(Item->Mesh);
	UEcoscapeStatics::SetAllMaterials(MainMesh, InvalidMaterial);
}

void APlaceableItemPreview::SetPosition(FVector NewPosition)
{
	SCOPE_CYCLE_COUNTER(STAT_MovePreview);
	
	SetActorLocation(NewPosition);

	// Check to see if position is valid or not
	FComponentQueryParams ComponentQueryParams;
	ComponentQueryParams.AddIgnoredActor(MainMesh->GetOwner());
	TArray<FOverlapResult> OverlapResults;
	GetWorld()->ComponentOverlapMulti(OverlapResults, MainMesh,
		MainMesh->GetComponentLocation(), FRotator(0, TargetItemRotation, 0),
		ComponentQueryParams, FCollisionObjectQueryParams::AllObjects);
	
	bIsValidPlacement = true;
	for (const auto Item : OverlapResults)
	{
		if (Item.Component->GetCollisionResponseToChannel(BlockingChannel) == ECR_Block)
		{
			bIsValidPlacement = false;
			break;
		}
	}
	
	// Update preview material
	if (bIsValidPlacement)
		UEcoscapeStatics::SetAllMaterials(MainMesh, ValidMaterial);
	else
		UEcoscapeStatics::SetAllMaterials(MainMesh, InvalidMaterial);
}

void APlaceableItemPreview::SetTargetRotation(float NewValue)
{
	TargetItemRotation = NewValue;
	CurrentRotationAlpha = 0;
}

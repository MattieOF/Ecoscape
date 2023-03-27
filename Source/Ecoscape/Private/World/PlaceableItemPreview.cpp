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

void APlaceableItemPreview::Tick(const float DeltaSeconds)
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

	// Update scale
	if (CurrentScaleAlpha < 1)
	{
		CurrentScaleAlpha += (1 - CurrentScaleAlpha) * 10 * DeltaSeconds;
		if (1 - CurrentScaleAlpha < 0.02)
			CurrentScaleAlpha = 1;

		const float Scale = FMath::InterpEaseOut(static_cast<float>(GetActorScale().X), TargetItemScale, CurrentScaleAlpha, 2);
		SetActorScale3D(FVector(Scale));
	}
}

void APlaceableItemPreview::SetItem(UPlaceableItemData* Item)
{
	MainMesh->SetStaticMesh(Item->Mesh);
	UEcoscapeStatics::SetAllMaterials(MainMesh, InvalidMaterial);
}

void APlaceableItemPreview::SetPosition(const FVector NewPosition)
{
	SCOPE_CYCLE_COUNTER(STAT_MovePreview);
	
	SetActorLocation(NewPosition);

	// Check to see if position is valid or not
	// First, set the scale to target scale, so we check against that instead of the lerped one.
	const float PreviousScale = GetActorScale().X;
	SetActorScale3D(FVector(TargetItemScale));
	
	FComponentQueryParams ComponentQueryParams;
	ComponentQueryParams.AddIgnoredActor(MainMesh->GetOwner());
	TArray<FOverlapResult> OverlapResults;
	GetWorld()->ComponentOverlapMulti(OverlapResults, MainMesh,
		MainMesh->GetComponentLocation(), FRotator(0, TargetItemRotation, 0),
		ComponentQueryParams, FCollisionObjectQueryParams::AllObjects);

	// Reset scale
	SetActorScale3D(FVector(PreviousScale));

	// Check that the placement is valid; i.e no overlaps with objects that block placement
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

void APlaceableItemPreview::SetTargetRotation(const float NewValue, const bool bInstant)
{
	TargetItemRotation = NewValue;
	CurrentRotationAlpha = bInstant ? .99f : 0;
}

void APlaceableItemPreview::SetTargetScale(const float NewValue, const bool bInstant)
{
	TargetItemScale = NewValue;
	CurrentScaleAlpha = bInstant ? .99f : 0;
}

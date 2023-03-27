// copyright lololol

#include "Character/EcoscapeTDCharacter.h"

#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "Character/EcoscapePlayerController.h"
#include "World/PlacedItem.h"

AEcoscapeTDCharacter::AEcoscapeTDCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetWorldRotation(FRotator(-80, 0, 0));
	RootComponent = Camera;
}

void AEcoscapeTDCharacter::SetCurrentTool(EEcoscapeTool NewTool)
{
	CurrentTool = NewTool;

	// Create or destroy item preview
	if (CurrentTool != ETPlaceObjects && ItemPreview)
	{
		ItemPreview->Destroy();
		ItemPreview = nullptr;
	} else if (CurrentTool == ETPlaceObjects)
	{
		SetItemPreview(TestItem);
	}
}

void AEcoscapeTDCharacter::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce)
{
	AddActorWorldOffset(WorldDirection * ScaleValue * Speed);
}

void AEcoscapeTDCharacter::OnToolUsed()
{
	switch (CurrentTool)
	{
	case ETPlaceObjects:
		{
			// Check position is valid
			if (!ItemPreview || !ItemPreview->IsValidPlacement())
			{
				OnFailedPlacementAttempt();
				break;
			}

			// Actually place the item
			FHitResult Hit;
			const TArray<AActor*> IgnoredActors;
			if (UEcoscapeStatics::GetHitResultAtCursorByChannel(Cast<const APlayerController>(GetController()), FloorChannel, true, Hit, IgnoredActors))
			{
				APlacedItem* Item = APlacedItem::SpawnItem(GetWorld(), TestItem, Hit.Location, FVector(PlacedItemScale), FRotator(0, PlacedItemRotation, 0));
				Item->AddActorWorldOffset(FVector(0, 0, UEcoscapeStatics::GetZUnderOrigin(Item))); // Move it so the bottom of the mesh is on the ground
				OnItemPlaced(Item->GetActorLocation(), Item);
			}
		}
		break;
	default: UE_LOG(LogEcoscape, Error, TEXT("Attempted to use unimplemented tool: %i"), static_cast<int>(CurrentTool)); break;
	}
}

void AEcoscapeTDCharacter::OnToolAltUsed()
{
	switch (CurrentTool)
	{
	case ETPlaceObjects:
		{
			PlacedItemRotation += 90;
			PlacedItemRotation = FMath::Fmod(PlacedItemRotation, 360);
			if (ItemPreview)
				ItemPreview->SetTargetRotation(PlacedItemRotation);	
			break;
		}
	default: UE_LOG(LogEcoscape, Error, TEXT("Attempted to use unimplemented tool: %i"), static_cast<int>(CurrentTool)); break;
	}
}

void AEcoscapeTDCharacter::ResetTool(bool bInstant)
{
	switch (CurrentTool)
	{
	case ETPlaceObjects:
		{
			PlacedItemRotation = 0;
			ItemPreview->SetTargetRotation(PlacedItemRotation, bInstant);
			PlacedItemScale = 1;
			ItemPreview->SetTargetScale(PlacedItemScale, bInstant);
			break;
		}
	default: UE_LOG(LogEcoscape, Error, TEXT("Attempted to use unimplemented tool: %i"), static_cast<int>(CurrentTool)); break;
	}
}

void AEcoscapeTDCharacter::AddScrollInput(float Value)
{
	if (CurrentTool == ETPlaceObjects && EcoscapePlayerController->IsModifierHeld())
	{
		PlacedItemScale = FMath::Clamp(PlacedItemScale += Value * 0.2f, ItemScaleBounds.X, ItemScaleBounds.Y);
		ItemPreview->SetTargetScale(PlacedItemScale);
	} else
		TargetHeight = FMath::Clamp(TargetHeight + -Value * ZoomSensitivity, HeightBounds.X, HeightBounds.Y);
}

void AEcoscapeTDCharacter::BeginPlay()
{
	Super::BeginPlay();
	TargetHeight = GetActorLocation().Z;
}

void AEcoscapeTDCharacter::Tick(float DeltaSeconds)
{
	Super::Tick(DeltaSeconds);

	if (!bIsPossessed)
		return;
	
	// Animate zoom height
	FVector Location = GetActorLocation();
	if (Location.Z != TargetHeight)
	{
		const float Diff = Location.Z - TargetHeight;
		if (FMath::Abs(Diff) < 1)
			Location.Z = TargetHeight;
		else
			Location.Z += -Diff * 10.f * DeltaSeconds;
		SetActorLocation(Location);	
	}

	// Do tool logic
	switch (CurrentTool)
	{
	case ETPlaceObjects:
		FHitResult Hit;
		const TArray<AActor*> IgnoredActors;
		if (UEcoscapeStatics::GetHitResultAtCursorByChannel(Cast<const APlayerController>(GetController()), FloorChannel, true, Hit, IgnoredActors))
			ItemPreview->SetPosition(Hit.Location + FVector(0, 0, UEcoscapeStatics::GetZUnderOrigin(ItemPreview)));
		break;
	}
}

void AEcoscapeTDCharacter::PossessedBy(AController* NewController)
{
	Super::PossessedBy(NewController);

	// Set possessed flag
	bIsPossessed = true;
	
	// Enable mouse
	if (AEcoscapePlayerController* PlayerController = Cast<AEcoscapePlayerController>(NewController))
	{
		PlayerController->SetMouseEnabled(true);
		EcoscapePlayerController = PlayerController;
	}
	else
		UE_LOG(LogEcoscape, Error, TEXT("Ecoscape pawn possessed by non-ecoscape controller!"));
	
	// Call set tool with our current tool so that we recreate item preview or anything else we need to do
	SetCurrentTool(CurrentTool);
}

void AEcoscapeTDCharacter::UnPossessed()
{
	bIsPossessed = false;

	if (ItemPreview)
	{
		ItemPreview->Destroy();
		ItemPreview = nullptr;
	}
}

void AEcoscapeTDCharacter::CreateItemPreview()
{
	ItemPreview = GetWorld()->SpawnActor<APlaceableItemPreview>(ItemPreviewClass, FVector(0, 0, 0), FRotator::ZeroRotator);
	ItemPreview->SetTargetRotation(PlacedItemRotation, true);
	ItemPreview->SetTargetScale(PlacedItemScale, true);
}

void AEcoscapeTDCharacter::SetItemPreview(UPlaceableItemData* ItemData)
{
	if (!ItemPreview)
		CreateItemPreview();
	ItemPreview->SetItem(ItemData);
}

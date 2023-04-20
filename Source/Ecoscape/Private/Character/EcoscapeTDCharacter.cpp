// copyright lololol

#include "Character/EcoscapeTDCharacter.h"

#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "Character/EcoscapePlayerController.h"
#include "World/EcoscapeTerrain.h"
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
	case ETSculpt:
		{
			FHitResult Hit;
			const TArray<AActor*> IgnoredActors;
			if (UEcoscapeStatics::GetHitResultAtCursorByChannel(Cast<const APlayerController>(GetController()), FloorChannel, true, Hit, IgnoredActors))
			{
				AEcoscapeTerrain* Terrain = Cast<AEcoscapeTerrain>(Hit.GetActor());
				if (!Terrain)
					return;
				auto Indicies = Terrain->GetVerticiesInSphere(Hit.ImpactPoint, 6 * Terrain->GetScale());
				for (auto& [Vertex, Dist] : Indicies)
					Terrain->AddVertexColour(Vertex, FColor(0, UEcoscapeStatics::MapFloat(Dist, 0, 6 * Terrain->GetScale(), 30, 0), 0), false);
				Terrain->FlushMesh();
			}
		}
		break;
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
				FRotator Rot = ItemPreview->GetActorRotation();
				APlacedItem* Item = APlacedItem::SpawnItem(GetWorld(), TestItem, Hit.Location, FVector(PlacedItemScale), FRotator(Rot.Pitch, PlacedItemRotation, Rot.Roll));
				Item->AddActorWorldOffset(FVector(0, 0, UEcoscapeStatics::GetZUnderOrigin(Item) + TestItem->ZOffset)); // Move it so the bottom of the mesh is on the ground
				OnItemPlaced(Item->GetActorLocation(), Item);

				if (AEcoscapeTerrain* Terrain = Cast<AEcoscapeTerrain>(Hit.GetActor()))
				{
					Terrain->PlacedItems.Add(Item);
					auto Verts = Terrain->GetVerticiesInSphere(Hit.ImpactPoint, TestItem->ColourRange * PlacedItemScale, true);
					for (auto& [Vertex, _] : Verts)
					{
						if (bDrawDebug)
							DrawDebugSphere(GetWorld(), Terrain->GetVertexPositionWorld(Vertex), 20, 6, FColor::Red, false, 2.5f);
						Terrain->CalculateVertColour(Vertex);
					}
					Terrain->FlushMesh();
				}
			}
		}
		break;
	case ETDestroyObjects:
		{
			
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
			}
			break;
		case ETSculpt:
			{
				
			}
			break;
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
		PlacedItemScale = FMath::Clamp(PlacedItemScale += Value * 0.2f, TestItem->ScaleBounds.X, TestItem->ScaleBounds.Y);
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

#if 0
	{
		FHitResult Hit;
		const TArray<AActor*> IgnoredActors;
		if (UEcoscapeStatics::GetHitResultAtCursorByChannel(Cast<const APlayerController>(GetController()), FloorChannel, true, Hit, IgnoredActors))
		{
			if (AEcoscapeTerrain* Terrain = Cast<AEcoscapeTerrain>(Hit.GetActor()))
			{
				TArray<int> Indicies = Terrain->GetVerticiesInSphere(Hit.ImpactPoint, 800);
				for (int i : Indicies)
				{
					FVector VertexPos = Terrain->GetVertexPositionWorld(i);
					DrawDebugSphere(GetWorld(), VertexPos, 20, 6, FColor::Red);
				}
			}
		}
	}
#endif

	// Do tool logic
	switch (CurrentTool)
	{
	case ETPlaceObjects:
		{
			FHitResult Hit;
			const TArray<AActor*> IgnoredActors;
			if (UEcoscapeStatics::GetHitResultAtCursorByChannel(Cast<const APlayerController>(GetController()), FloorChannel, true, Hit, IgnoredActors))
				ItemPreview->UpdateWithHitInfo(Hit);
		}
		break;
	case ETDestroyObjects:
		{
		}
		break;
	case ETSculpt:
		{
		}
		break;
	default: UE_LOG(LogEcoscape, Error, TEXT("Ticking unimplemented tool: %i"), static_cast<int>(CurrentTool)); break;
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
	Super::UnPossessed();
	
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

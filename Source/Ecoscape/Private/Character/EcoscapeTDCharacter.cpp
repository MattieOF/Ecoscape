// copyright lololol

#include "Character/EcoscapeTDCharacter.h"

#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "Character/EcoscapePlayerController.h"
#include "Net/NetworkProfiler.h"
#include "World/EcoscapeTerrain.h"
#include "World/Fence/FencePlacementPreview.h"
#include "World/PlacedItem.h"
#include "World/Fence/ProceduralFenceMesh.h"

AEcoscapeTDCharacter::AEcoscapeTDCharacter()
{
	PrimaryActorTick.bCanEverTick = true;
	
	Camera = CreateDefaultSubobject<UCameraComponent>(TEXT("Camera"));
	Camera->SetWorldRotation(FRotator(-80, 0, 0));
	RootComponent = Camera;
}

void AEcoscapeTDCharacter::SetCurrentTool(EEcoscapeTool NewTool)
{
	// Create or destroy item preview
	if (NewTool != ETPlaceObjects && ItemPreview)
	{
		ItemPreview->Destroy();
		ItemPreview = nullptr;
	} else if (NewTool == ETPlaceObjects)
		SetItemPreview(TestItem);
	else if (NewTool != ETPlaceFence)
		FencePlacementPreview->DisablePreview();

	if (CurrentTool != ETPlaceFence)
		FencePlacementStage = EFPNone;
	
	CurrentTool = NewTool;

	HighlightObject(nullptr);
}

void AEcoscapeTDCharacter::AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce)
{
	AddActorWorldOffset(WorldDirection * ScaleValue * Speed);
}

void AEcoscapeTDCharacter::OnToolUsed()
{
	switch (CurrentTool)
	{
	// case ETSculpt:
	// 	{
	// 		FHitResult Hit;
	// 		const TArray<AActor*> IgnoredActors;
	// 		if (UEcoscapeStatics::GetHitResultAtCursorByChannel(Cast<const APlayerController>(GetController()), FloorChannel, true, Hit, IgnoredActors))
	// 		{
	// 			AEcoscapeTerrain* Terrain = Cast<AEcoscapeTerrain>(Hit.GetActor());
	// 			if (!Terrain)
	// 				return;
	// 			auto Indicies = Terrain->GetVerticiesInSphere(Hit.ImpactPoint, 6 * Terrain->GetScale());
	// 			for (auto& [Vertex, Dist] : Indicies)
	// 				Terrain->AddVertexColour(Vertex, FColor(0, UEcoscapeStatics::MapFloat(Dist, 0, 6 * Terrain->GetScale(), 30, 0), 0), false);
	// 			Terrain->FlushMesh();
	// 		}
	// 	}
	// 	break;
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
					Item->AssociatedTerrain = Terrain; 
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
			if (APlacedItem* Item = Cast<APlacedItem>(HighlightedObject))
			{
				AEcoscapeTerrain* Terrain = Item->AssociatedTerrain;

				if (Terrain)
				{
					Terrain->PlacedItems.Remove(Item);
					auto Verts = Terrain->GetVerticiesInSphere(Item->GetActorLocation(), TestItem->ColourRange * PlacedItemScale, true);
					for (auto& [Vertex, _] : Verts)
					{
						if (bDrawDebug)
							DrawDebugSphere(GetWorld(), Terrain->GetVertexPositionWorld(Vertex), 20, 6, FColor::Red, false, 2.5f);
						Terrain->CalculateVertColour(Vertex);
					}
					Terrain->FlushMesh();
				}
				else
					UE_LOG(LogEcoscape, Error, TEXT("Placed item being destroyed by TDCharacter has no associated terrain!"));
				
				Item->Destroy();
				HighlightObject(nullptr);
			} else if (AProceduralFenceMesh* Fence = Cast<AProceduralFenceMesh>(HighlightedObject))
			{
				if (!Fence->bDestroyable)
					return;

				if (Fence->AssociatedTerrain)
					Fence->AssociatedTerrain->PlacedFences.Remove(Fence);
				
				Fence->Destroy();
				HighlightObject(nullptr);
			}
		}
		break;
	case ETPlaceFence:
		{
			if (FencePlacementStage == EFPNone)
			{
				// We haven't started placing yet.
				// Take the current position and turn that into our start
				FHitResult Hit;
				if (UEcoscapeStatics::GetHitResultAtCursorByChannel(Cast<const APlayerController>(GetController()), FloorChannel, true, Hit, TArray<AActor*>()))
				{
					if (AEcoscapeTerrain* Terrain = Cast<AEcoscapeTerrain>(Hit.GetActor()))
					{
						FencePlacementPreview->Terrain = Terrain;
						FencePlacementPreview->StartPreview(Terrain->GetVertexXY(Terrain->GetClosestVertex(Hit.ImpactPoint)));
						FencePlacementStage = EFPPlacing;
					}
				}
			}
			else
			{
				// This is our second location.
				// If this is valid, we can create our fence now

				if (FencePlacementPreview->bValid)
				{
					FencePlacementPreview->CreateFence();
					FencePlacementPreview->DisablePreview();
					FencePlacementStage = EFPNone;
				} else
				{
					OnFailedPlacementAttempt();
				}
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
			}
			break;
		case ETPlaceFence:
			{
				if (FencePlacementStage == EFPPlacing)
				{
					// We're cancelling the place
					FencePlacementStage = EFPNone;
					FencePlacementPreview->DisablePreview();
					FenceStart = FVector2D::ZeroVector;
				}
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

	// Make fence placement preview now
	FencePlacementPreview = GetWorld()->SpawnActor<AFencePlacementPreview>();
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
	case ETNone:
		break;
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
			CheckHoveredObject();
		}
		break;
	case ETPlaceFence:
		{
			if (FencePlacementStage == EFPPlacing)
			{
				// We're placing, so update the positions of the preview
				FHitResult Hit;
				if (UEcoscapeStatics::GetHitResultAtCursorByChannel(Cast<const APlayerController>(GetController()), FloorChannel, true, Hit, TArray<AActor*>()))
				{
					if (AEcoscapeTerrain* Terrain = Cast<AEcoscapeTerrain>(Hit.GetActor()))
					{
						auto XY = Terrain->GetVertexXY(Terrain->GetClosestVertex(Hit.ImpactPoint));
						auto Pos = Terrain->GetVertexPositionWorld(Terrain->GetVertexIndex(XY.X, XY.Y));
						if (bDrawDebug)
							DrawDebugSphere(GetWorld(), Pos, 20, 6, FColor::Red);
						
						FencePlacementPreview->UpdatePreview(Terrain->GetVertexXY(Terrain->GetClosestVertex(Hit.ImpactPoint)));
					}
				}
			}
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
	if (FencePlacementPreview)
		FencePlacementPreview->DisablePreview();
	FencePlacementStage = EFPNone;
	HighlightObject(nullptr);
}

void AEcoscapeTDCharacter::CheckHoveredObject()
{
	FHitResult Hit;
	UEcoscapeStatics::GetHitResultAtCursorByChannel(Cast<const APlayerController>(GetController()), ECC_HIGHLIGHTABLE, true, Hit, TArray<AActor*>());
	// No need to check for null here, as passing null to HighlightObject() simply unhighlights anything and sets HighlightedObject to null
	// Therefore, we have to be careful when using HighlightedObject
	AEcoscapeObject* EcoscapeObject = Cast<AEcoscapeObject>(Hit.GetActor());
	HighlightObject(EcoscapeObject);
}

void AEcoscapeTDCharacter::HighlightObject(AEcoscapeObject* Object)
{
	if (HighlightedObject)
		HighlightedObject->Outline->HideOutline();
	HighlightedObject = Object;
	if (HighlightedObject && HighlightedObject->Outline)
		HighlightedObject->Outline->ShowOutline();
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

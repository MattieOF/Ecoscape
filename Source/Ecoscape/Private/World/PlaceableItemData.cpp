// copyright lololol

// Disable this as MarkPackageDirty triggers it for some reason
// ReSharper disable CppExpressionWithoutSideEffects

#include "World/PlaceableItemData.h"

#include "EcoscapeLog.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/TextureRenderTarget2D.h"
#include "Kismet/KismetStringLibrary.h"
#include "UObject/SavePackage.h"
#include "World/EcoscapeTerrain.h"
#include "World/PlacedItem.h"

#if WITH_EDITOR
#include "ObjectTools.h"
#include "Editor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#endif

UPlaceableItemData::UPlaceableItemData()
{
	ColourRangeSquared = ColourRange * ColourRange;
	CreateValidTerrainsArray();
}

UPlaceableItemData::UPlaceableItemData(FObjectInitializer& ObjectInitializer)
	: Super(ObjectInitializer)
{
	ColourRangeSquared = ColourRange * ColourRange;
	CreateValidTerrainsArray();
}

#if WITH_EDITOR
// This function automatically creates an icon for a given placeable item type.
// It does this by creating a placed version of the object in the world, as well as creating a scene capture,
// setting both of these objects up, and then using the scene capture to capture a basic icon for the
// item and save it to an appropriately named Texture2D asset.
void UPlaceableItemData::CreateIcon()
{
	UE_LOG(LogEcoscape, Log, TEXT("Creating icon for %s"), *GetName());

	// Check mesh is valid
	// Commented out as it's probably not needed, since it was written when we were capturing
	// a StaticMeshActor with the mesh set instead of creating a placed item
	// 
	// if (!Mesh)
	// {
	// 	UE_LOG(LogEcoscape, Error, TEXT("Tried to create icon for %s, but it's mesh is null!"), *GetName());
	// 	return;
	// }

	// Get the editor world.
	// Normal GetWorld() doesn't work here!
	UWorld* EditorWorld = GEditor->GetEditorWorldContext(false).World();
	if (!EditorWorld)
	{
		UE_LOG(LogEcoscape, Error, TEXT("While generating icon for %s, failed to get editor world! Make sure a level is loaded."), *GetName());
		return;
	}

	// Create the scene capture and set up basic settings
	ASceneCapture2D* SceneCapture = EditorWorld->SpawnActor<ASceneCapture2D>();
	if (!SceneCapture)
	{
		UE_LOG(LogEcoscape, Error, TEXT("While generating icon for %s, failed create a SceneCapture2D!"), *GetName());
		return;
	}
	SceneCapture->GetCaptureComponent2D()->bCaptureEveryFrame = false;
	SceneCapture->GetCaptureComponent2D()->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	SceneCapture->GetCaptureComponent2D()->FOVAngle = IconFOV;
	SceneCapture->GetCaptureComponent2D()->ShowFlags.SetAtmosphere(false);

	// Create the placed item and add it to the capture's show list
	APlacedItem* Item = APlacedItem::SpawnItem(EditorWorld, this, FVector(0, 0, 0), FVector::OneVector, IconObjectRotation);
	if (!Item)
	{
		UE_LOG(LogEcoscape, Error, TEXT("While generating icon for %s, failed to spawn it's respective APlacedItem!"), *GetName());
		return;
	}
	SceneCapture->GetCaptureComponent2D()->ShowOnlyActors.Add(Item);

	// Calculate position of scene capture
	const UStaticMeshComponent* MeshComponent = Item->GetMesh();
	const FBoxSphereBounds Bounds = MeshComponent->Bounds;
	const FVector Extent = Bounds.BoxExtent;
	const float Distance = (Extent.GetMax() / FMath::Tan(FMath::DegreesToRadians(IconFOV) * 0.5f)) + 150; // Add 150 as this math isn't perfect usually
	const FVector NewLocation = Bounds.Origin - SceneCapture->GetActorForwardVector() * Distance;
	SceneCapture->SetActorLocation(NewLocation);
	SceneCapture->AddActorWorldOffset(IconCameraOffset);

	// Setup render target
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	RenderTarget->AddToRoot(); // Prevent GC while working
	RenderTarget->ClearColor = FLinearColor::Black;
	RenderTarget->InitAutoFormat(IconSize, IconSize);
	SceneCapture->GetCaptureComponent2D()->TextureTarget = RenderTarget;

	// Actually capture the scene
	SceneCapture->GetCaptureComponent2D()->CaptureScene();

	// Create the package to save the icon to
	FString TextureName = "ICON_";
	TextureName += GetName();
	FString PackageName = TEXT("/Game/Blueprints/Placeables/Data/Icons/");
	PackageName += TextureName;
	UPackage* Package = CreatePackage(*PackageName);
	Package->FullyLoad();

	// Create the Texture2D from the render target
	UTexture2D* Texture = RenderTarget->ConstructTexture2D(Package, TextureName, RF_Public | RF_Standalone | RF_MarkAsRootSet);

	// Save the Texture2D
	Texture->UpdateResource();
	Package->MarkPackageDirty();
	FAssetRegistryModule::AssetCreated(Texture);
	const FString PackageFileName = FPackageName::LongPackageNameToFilename(PackageName, FPackageName::GetAssetPackageExtension());
	FSavePackageArgs Args;
	Args.TopLevelFlags = RF_Public | RF_Standalone;
	Args.bForceByteSwapping = true;
	Args.bWarnOfLongFilename = true;
	Args.SaveFlags = SAVE_NoError;
	Args.Error = GError;
	const bool Saved = UPackage::SavePackage(Package, Texture, *PackageFileName, Args);
	if (!Saved)
	{	
		UE_LOG(LogEcoscape, Error, TEXT("While generating icon for %s, failed to save the texture (%s) to package %s!"),
		       *GetName(), *TextureName, *PackageName);
		return;
	}

	// Update this object's icon property and mark as dirty so it's saved
	Icon = Texture;
	MarkPackageDirty();
	
	// Clean up
	RenderTarget->RemoveFromRoot();
	RenderTarget->ConditionalBeginDestroy();
	Item->Destroy();
	SceneCapture->Destroy();
}

void UPlaceableItemData::DeleteItem()
{
	TArray<FAssetData> Assets;
	Assets.Add(FAssetData(this));
	if (Icon)
		Assets.Add(FAssetData(Icon));
	ObjectTools::DeleteAssets(Assets, true);
}

void UPlaceableItemData::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);

	CreateValidTerrainsArray();	
}
#endif

void UPlaceableItemData::CreateValidTerrainsArray()
{
	if (ValidTerrains == "All")
	{
		ValidTerrainsArray.Empty();
		return;
	}
	
	ValidTerrainsArray = UKismetStringLibrary::ParseIntoArray(ValidTerrains,  ";");
}

bool UPlaceableItemData::IsValidForTerrain(AEcoscapeTerrain* Terrain)
{
	return IsValidForTerrainName(Terrain->TerrainName);
}

UPlaceableItemData* UItemDataList::GetRandomItem()
{
	if (Options.Num() <= 0)
	{
		UE_LOG(LogEcoscape, Error, TEXT("In an item data list, we tried to get a random item, but there are 0 options!"));
		return nullptr;
	}

	const int Index = FMath::RandRange(0, Options.Num() - 1);
	return Options[Index];
}

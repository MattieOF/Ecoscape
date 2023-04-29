// copyright lololol

// Disable this as MarkPackageDirty triggers it for some reason
// ReSharper disable CppExpressionWithoutSideEffects

#include "World/PlaceableItemData.h"

#include "EcoscapeLog.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UObject/SavePackage.h"
#include "World/PlacedItem.h"

#if WITH_EDITOR
#include "Editor.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#endif

UPlaceableItemData::UPlaceableItemData()
{
	ColourRangeSquared = ColourRange * ColourRange;
}

#if WITH_EDITOR
void UPlaceableItemData::CreateIcon()
{
	UE_LOG(LogEcoscape, Log, TEXT("Creating icon for %s"), *GetName());

	if (!Mesh)
	{
		UE_LOG(LogEcoscape, Error, TEXT("Tried to create icon for %s, but it's mesh is null!"), *GetName());
		return;
	}

	UWorld* EditorWorld = GEditor->GetEditorWorldContext(false).World();
	ASceneCapture2D* SceneCapture = EditorWorld->SpawnActor<ASceneCapture2D>();

	SceneCapture->GetCaptureComponent2D()->bCaptureEveryFrame = false;
	SceneCapture->GetCaptureComponent2D()->PrimitiveRenderMode = ESceneCapturePrimitiveRenderMode::PRM_UseShowOnlyList;
	SceneCapture->GetCaptureComponent2D()->FOVAngle = IconFOV;

	APlacedItem* Item = APlacedItem::SpawnItem(EditorWorld, this, FVector(0, 0, 0));
	SceneCapture->GetCaptureComponent2D()->ShowOnlyActors.Add(Item);

	// Calculate position of scene capture
	const UStaticMeshComponent* MeshComponent = Item->GetMesh();
	const FBoxSphereBounds Bounds = MeshComponent->Bounds;
	const FVector Extent = Bounds.BoxExtent;
	const float Distance = (Extent.GetMax() / FMath::Tan(FMath::DegreesToRadians(IconFOV) * 0.5f)) + 200;
	const FVector NewLocation = Bounds.Origin - SceneCapture->GetActorForwardVector() * Distance;
	SceneCapture->SetActorLocation(NewLocation);

	// Setup render target
	UTextureRenderTarget2D* RenderTarget = NewObject<UTextureRenderTarget2D>();
	RenderTarget->AddToRoot(); // Prevent GC while working
	RenderTarget->ClearColor = FLinearColor::Black;
	constexpr int32 Size = 1024;
	RenderTarget->InitAutoFormat(Size, Size);
	SceneCapture->GetCaptureComponent2D()->TextureTarget = RenderTarget;
	
	SceneCapture->GetCaptureComponent2D()->CaptureScene();

	// Create the package
	FString TextureName = "ICON_";
	TextureName += GetName();
	FString PackageName = TEXT("/Game/Blueprints/Placeables/Data/Icons/");
	PackageName += TextureName;
	UPackage* Package = CreatePackage(*PackageName);
	Package->FullyLoad();

	// Create the texture
	UTexture2D* Texture = RenderTarget->ConstructTexture2D(Package, TextureName, RF_Public | RF_Standalone | RF_MarkAsRootSet);

	// Save
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
	UPackage::SavePackage(Package, Texture, *PackageFileName, Args);

	Icon = Texture;
	MarkPackageDirty();
	
	// Clean up
	RenderTarget->RemoveFromRoot();
	RenderTarget->ConditionalBeginDestroy();
	Item->Destroy();
	SceneCapture->Destroy();
}
#endif

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

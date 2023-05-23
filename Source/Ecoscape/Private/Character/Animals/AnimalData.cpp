// copyright lololol

#include "Character/Animals/AnimalData.h"

#include "EcoscapeLog.h"
#include "Animation/AnimInstance.h"
#include "Animation/SkeletalMeshActor.h"
#include "AssetRegistry/AssetRegistryModule.h"
#include "Character/Animals/BaseAnimal.h"
#include "Components/SceneCaptureComponent2D.h"
#include "Engine/SceneCapture2D.h"
#include "Engine/TextureRenderTarget2D.h"
#include "UObject/ConstructorHelpers.h"
#include "UObject/SavePackage.h"

UAnimalData::UAnimalData()
{
	static ConstructorHelpers::FClassFinder<APawn> DefaultAnimalClass(TEXT("Blueprint'/Game/Blueprints/BP_BaseAnimal.BP_BaseAnimal_C'"));
	if (DefaultAnimalClass.Succeeded())
		AnimalClass = DefaultAnimalClass.Class;
	else
		AnimalClass = ABaseAnimal::StaticClass();
}

#if WITH_EDITOR
void UAnimalData::CreateIcon()
{
	UE_LOG(LogEcoscape, Log, TEXT("Creating icon for %s"), *GetName());
    
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
    SceneCapture->GetCaptureComponent2D()->ShowFlags.SetLighting(false);
    
    // Create the placed item and add it to the capture's show list
    ASkeletalMeshActor* TempMeshActor = EditorWorld->SpawnActor<ASkeletalMeshActor>(FVector::ZeroVector, FRotator::ZeroRotator);
	TempMeshActor->GetSkeletalMeshComponent()->SetSkeletalMesh(Mesh);
	TempMeshActor->GetSkeletalMeshComponent()->SetAnimInstanceClass(AnimationClass);
	TempMeshActor->SetActorRotation(IconObjectRotation);
    if (!TempMeshActor)
    {
    	UE_LOG(LogEcoscape, Error, TEXT("While generating icon for %s, failed to spawn it's respective mesh!"), *GetName());
    	return;
    }
    SceneCapture->GetCaptureComponent2D()->ShowOnlyActors.Add(TempMeshActor);
    
    // Calculate position of scene capture
    const FBoxSphereBounds Bounds = TempMeshActor->GetSkeletalMeshComponent()->Bounds;
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
    FString PackageName = TEXT("/Game/Blueprints/Character/Animals/Icons/");
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
    TempMeshActor->Destroy();
    SceneCapture->Destroy();
}
#endif

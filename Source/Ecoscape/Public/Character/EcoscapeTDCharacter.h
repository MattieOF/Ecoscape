// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "EcoscapeGameInstance.h"
#include "EcoscapeObject.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "World/PlaceableItemPreview.h"
#include "EcoscapeTDCharacter.generated.h"

class UItemFolder;
class AFencePlacementPreview;
class AEcoscapePlayerController;

UENUM(BlueprintType)
enum EEcoscapeTool
{
	ETNone            UMETA(DisplayName = "None"),
	ETPlaceObjects    UMETA(DisplayName = "Place Objects"),
	ETDestroyObjects  UMETA(DisplayName = "Destroy Objects"),
	ETPlaceFence      UMETA(DisplayName = "Place Fence"),
};

enum EFencePlacementStage
{
	EFPNone,
	EFPPlacing,
};

// ------------------------------------------
// Some data types relating to item selection
// TODO: Move to separate files?
enum EItemDataType
{
	Item,
	Folder
};

struct FItemDataInterface
{
	AEcoscapeTerrain* CurrentTerrain;
	EItemDataType Type;
	UPlaceableItemData* Item;
	UItemFolder* Folder;

	void RerollItem();
	FORCEINLINE UPlaceableItemData* GetItem() { return Type == EItemDataType::Folder ? NextItem : Item; }

private:
	UPlaceableItemData* NextItem; // For use with folders
};
// ------------------------------------------

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnToolChanged, TEnumAsByte<EEcoscapeTool>, NewTool);

UCLASS()
class ECOSCAPE_API AEcoscapeTDCharacter : public APawn
{
	GENERATED_BODY()

public:
	AEcoscapeTDCharacter();

	UFUNCTION(BlueprintCallable, BlueprintPure, meta=(WorldContext="WorldContext"))
	static FORCEINLINE AEcoscapeTDCharacter* GetEcoscapeTDCharacter(UObject* WorldContext, int32 Index = 0)
	{
		return Cast<AEcoscapeTDCharacter>(UGameplayStatics::GetPlayerPawn(WorldContext, Index));
	}

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Tools")
	FORCEINLINE EEcoscapeTool GetCurrentTool() const { return CurrentTool; }

	/**
	 * @brief Used to switch the currently used tool. Will do any extra logic, such as creating an item preview actor
	 * @param NewTool Tool to switch to
	 */
	UFUNCTION(BlueprintCallable, Category = "Tools")
	void SetCurrentTool(EEcoscapeTool NewTool);
	
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce) override;

	/**
	 * @brief Called by the player controller when a tool is used
	 */
	UFUNCTION(BlueprintCallable, Category = "Tools")
	void OnToolUsed();

	UFUNCTION(BlueprintCallable, Category = "Tools")
	void OnToolAltUsed();

	UFUNCTION(BlueprintCallable, Category = "Tools")
	void ResetTool(bool bInstant = false);

	/**
	 * @brief Called when a placeable item is placed by the player. Used to implement cosmetic events like sound and particles
	 * @param Location Location of the placed item
	 * @param Item The placed item object
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tools")
	void OnItemPlaced(FVector Location, APlacedItem* Item);

	/**
	 * @brief Called when the player attempts to place an item, but it fails
	 */
	UFUNCTION(BlueprintImplementableEvent, Category = "Tools")
	void OnFailedPlacementAttempt();

	UPROPERTY(BlueprintAssignable, BlueprintReadWrite, Category = "Tools")
	FOnToolChanged OnToolChanged;

	UFUNCTION(BlueprintCallable, Category = "Movement")
	void AddScrollInput(float Value);

	UFUNCTION(BlueprintCallable)
	void SetCurrentItem(UPlaceableItemData* Item);
	UFUNCTION(BlueprintCallable)
	void SetCurrentFolder(UItemFolder* Folder);

	void GoToTerrain(AEcoscapeTerrain* Terrain);
	
	/**
	 * @brief Speed the player moves
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float Speed = 10;

	/**
	 * @brief Sensitivity for scroll-wheel zoom
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	float ZoomSensitivity = 75;

	/**
	 * @brief Bounds of the height the player camera can be at
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Movement")
	FVector2D HeightBounds = FVector2D(800, 2500);

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Tools")
	FVector2D ItemScaleBounds = FVector2D(0.5, 2.5);

	/**
	 * @brief Class of APlaceableItemPreview to use when previewing placeable items
	 */
	UPROPERTY(EditAnywhere, Category = "Tools")
	TSubclassOf<APlaceableItemPreview> ItemPreviewClass = APlaceableItemPreview::StaticClass();

	/**
	 * @brief Collision channel blocked by objects that can have items placed on
	 */
	UPROPERTY(EditAnywhere, Category = "Tools")
	TEnumAsByte<ECollisionChannel> FloorChannel;

	UPROPERTY(EditAnywhere, Category = "Debugd")
	bool bDrawDebug = false;

	FItemDataInterface CurrentItemData;
	
protected:
	UPROPERTY(BlueprintReadOnly)
	UCameraComponent* Camera;

	UPROPERTY(BlueprintReadOnly, Category = "Tools")
	APlaceableItemPreview* ItemPreview;

	UPROPERTY(BlueprintReadOnly, Category = "Tools")
	float PlacedItemRotation = 0;

	UPROPERTY(BlueprintReadOnly, Category = "Tools")
	float PlacedItemScale = 1;

	UPROPERTY(BlueprintReadOnly, Category = "Movement")
	float TargetHeight = 0;
	
	UPROPERTY(BlueprintReadOnly, Category = "Tools")
	TEnumAsByte<EEcoscapeTool> CurrentTool = ETNone;

	UPROPERTY(BlueprintReadWrite, Category = "Tools")
	AEcoscapeObject* HighlightedObject = nullptr;
	
	UPROPERTY()
	AFencePlacementPreview* FencePlacementPreview;

	EFencePlacementStage FencePlacementStage = EFPNone;

	FVector2D FenceStart = FVector2D::ZeroVector;
	
	UPROPERTY(BlueprintReadWrite)
	bool bIsPossessed = false;

	UPROPERTY()
	AEcoscapePlayerController* EcoscapePlayerController;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	virtual void PossessedBy(AController* NewController) override;

	virtual void UnPossessed() override;

	UFUNCTION(BlueprintCallable)
	void CheckHoveredObject();
	
	UFUNCTION(BlueprintCallable)
	void HighlightObject(AEcoscapeObject* Object);

	UFUNCTION(BlueprintCallable)
	void CreateItemPreview();

	UFUNCTION(BlueprintCallable)
	void SetItemPreview(UPlaceableItemData* ItemData);
};

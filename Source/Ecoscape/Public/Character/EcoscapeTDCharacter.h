// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "World/PlaceableItemPreview.h"
#include "EcoscapeTDCharacter.generated.h"

UENUM(BlueprintType)
enum EEcoscapeTool
{
	ETNone            UMETA(DisplayName = "None"),
	ETPlaceObjects    UMETA(DisplayName = "Place Objects"),
	ETDestroyObjects  UMETA(DisplayName = "Destroy Objects")
};

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

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE EEcoscapeTool GetCurrentTool() const { return CurrentTool; }

	/**
	 * @brief Used to switch the currently used tool. Will do any extra logic, such as creating an item preview actor
	 * @param NewTool Tool to switch to
	 */
	UFUNCTION(BlueprintCallable)
	void SetCurrentTool(EEcoscapeTool NewTool);
	
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce) override;

	/**
	 * @brief Called by the player controller when a tool is used
	 */
	UFUNCTION(BlueprintCallable)
	void OnToolUsed();

	/**
	 * @brief Called when a placeable item is placed by the player. Used to implement cosmetic events like sound and particles
	 * @param Location Location of the placed item
	 * @param Item The placed item object
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnItemPlaced(FVector Location, APlacedItem* Item);

	/**
	 * @brief Called when the player attempts to place an item, but it fails
	 */
	UFUNCTION(BlueprintImplementableEvent)
	void OnFailedPlacementAttempt();

	UFUNCTION(BlueprintCallable)
	void AddScrollInput(float Value);

	/**
	 * @brief Speed the player moves
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 10;

	/**
	 * @brief Sensitivity for scroll-wheel zoom
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZoomSensitivity = 75;

	/**
	 * @brief Bounds of the height the player camera can be at
	 */
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D HeightBounds = FVector2D(800, 2500);

	/**
	 * @brief Class of APlaceableItemPreview to use when previewing placeable items
	 */
	UPROPERTY(EditAnywhere)
	TSubclassOf<APlaceableItemPreview> ItemPreviewClass = APlaceableItemPreview::StaticClass();

	/**
	 * @brief Collision channel blocked by objects that can have items placed on
	 */
	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> FloorChannel;

	UPROPERTY(EditAnywhere)
	UPlaceableItemData* TestItem;

protected:
	UPROPERTY(BlueprintReadOnly)
	UCameraComponent* Camera;

	UPROPERTY()
	APlaceableItemPreview* ItemPreview;

	UPROPERTY(BlueprintReadOnly)
	float TargetHeight = 0;
	
	UPROPERTY(BlueprintReadOnly)
	TEnumAsByte<EEcoscapeTool> CurrentTool = ETNone;

	UPROPERTY(BlueprintReadWrite)
	bool bIsPossessed = false;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	virtual void PossessedBy(AController* NewController) override;

	virtual void UnPossessed() override;

	void CreateItemPreview();
	void SetItemPreview(UPlaceableItemData* ItemData);
};

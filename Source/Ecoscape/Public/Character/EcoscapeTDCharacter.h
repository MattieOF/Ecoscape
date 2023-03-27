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

	UFUNCTION(BlueprintCallable)
	void SetCurrentTool(EEcoscapeTool NewTool);
	
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce) override;

	UFUNCTION(BlueprintCallable)
	void AddScrollInput(float Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZoomSensitivity = 75;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D HeightBounds = FVector2D(800, 2500);

	UPROPERTY(EditAnywhere)
	TSubclassOf<APlaceableItemPreview> ItemPreviewClass = APlaceableItemPreview::StaticClass();

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

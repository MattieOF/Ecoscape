// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "PlaceableItemData.h"
#include "GameFramework/Actor.h"
#include "PlaceableItemPreview.generated.h"

UCLASS()
class ECOSCAPE_API APlaceableItemPreview : public AActor
{
	GENERATED_BODY()

public:
	APlaceableItemPreview();

	virtual void Tick(float DeltaSeconds) override;

	UFUNCTION(BlueprintCallable)
	void SetItem(UPlaceableItemData* Item);

	UFUNCTION(BlueprintCallable)
	void SetPosition(FVector NewPosition);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsValidPlacement() const { return bIsValidPlacement; }

	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> BlockingChannel;

	UFUNCTION(BlueprintCallable)
	void SetTargetRotation(float NewValue, bool bInstant = false);

	UFUNCTION(BlueprintCallable)
	void SetTargetScale(float NewValue, bool bInstant = false);
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* MainMesh;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* ValidMaterial;
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* InvalidMaterial;

	UPROPERTY(BlueprintReadOnly)
	bool bIsValidPlacement = false;

	float CurrentRotationAlpha = 1;
	float CurrentScaleAlpha = 1;

	UPROPERTY(BlueprintReadWrite)
	float TargetItemRotation = 0;

	UPROPERTY(BlueprintReadWrite)
	float TargetItemScale = 1;
};

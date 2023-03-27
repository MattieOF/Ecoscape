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

	UFUNCTION(BlueprintCallable)
	void SetItem(UPlaceableItemData* Item);

	UFUNCTION(BlueprintCallable)
	void SetPosition(FVector NewPosition);

	UFUNCTION(BlueprintCallable, BlueprintPure)
	FORCEINLINE bool IsValidPlacement() const { return bIsValidPlacement; }

	UPROPERTY(EditAnywhere)
	TEnumAsByte<ECollisionChannel> BlockingChannel;
	
protected:
	UPROPERTY(EditAnywhere, BlueprintReadOnly)
	UStaticMeshComponent* MainMesh;

	UPROPERTY(EditAnywhere)
	UMaterialInterface* ValidMaterial;
	
	UPROPERTY(EditAnywhere)
	UMaterialInterface* InvalidMaterial;

	UPROPERTY(BlueprintReadOnly)
	bool bIsValidPlacement = false;
};

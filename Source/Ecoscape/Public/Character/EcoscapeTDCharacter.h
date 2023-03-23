// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Camera/CameraComponent.h"
#include "Kismet/GameplayStatics.h"
#include "EcoscapeTDCharacter.generated.h"

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
	
	virtual void AddMovementInput(FVector WorldDirection, float ScaleValue, bool bForce) override;

	UFUNCTION(BlueprintCallable)
	void AddScrollInput(float Value);

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 10;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float ZoomSensitivity = 75;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FVector2D HeightBounds = FVector2D(800, 2000);

protected:
	UPROPERTY(BlueprintReadOnly)
	UCameraComponent* Camera;

	UPROPERTY(BlueprintReadOnly)
	float TargetHeight = 0;

	virtual void BeginPlay() override;

	virtual void Tick(float DeltaSeconds) override;
	
	virtual void PossessedBy(AController* NewController) override;
};

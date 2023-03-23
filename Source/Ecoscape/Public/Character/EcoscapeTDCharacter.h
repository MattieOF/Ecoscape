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

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float Speed = 10;

protected:
	UPROPERTY(BlueprintReadOnly)
	UCameraComponent* Camera;
	
	virtual void PossessedBy(AController* NewController) override;
};

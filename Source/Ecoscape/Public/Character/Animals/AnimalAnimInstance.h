// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "Animation/AnimInstance.h"
#include "AnimalAnimInstance.generated.h"

/**
 * Base animation blueprint for animals
 */
UCLASS()
class ECOSCAPE_API UAnimalAnimInstance : public UAnimInstance
{
	GENERATED_BODY()

public:
	virtual void NativeUpdateAnimation(float DeltaSeconds) override;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	float MoveSpeed = 0;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsEating = false;
	
	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDrinking = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsSleeping = false;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	bool bIsDead = false;
};

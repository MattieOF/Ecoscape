// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "AnimalData.h"
#include "GameFramework/Actor.h"
#include "GameFramework/Character.h"
#include "BaseAnimal.generated.h"

UCLASS(BlueprintType, Blueprintable)
class ECOSCAPE_API ABaseAnimal : public ACharacter
{
	GENERATED_BODY()

public:
	// Sets default values for this actor's properties
	ABaseAnimal();

	virtual void BeginPlay() override;
	
	virtual void Tick(float DeltaSeconds) override;
	
#if WITH_EDITOR
	virtual void PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent) override;
#endif

	UFUNCTION(BlueprintCallable)
	void SetAnimalData(UAnimalData* Data, bool bRecreateAI = true);
	
	UPROPERTY(EditAnywhere)
	UAnimalData* AnimalData;

	UPROPERTY(EditAnywhere, BlueprintReadWrite)
	FString GivenName;

private:
	FRotator TargetRotation;
};

// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_AnimalType.generated.h"

/**
 * Environment query test to test animal types
 */
UCLASS()
class ECOSCAPE_API UEnvQueryTest_AnimalType : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_AnimalType();

	UPROPERTY(EditDefaultsOnly, Category = Test)
	FString ValidAnimalTypes;

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};

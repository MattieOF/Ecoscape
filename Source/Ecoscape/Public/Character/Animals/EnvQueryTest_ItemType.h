// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "EnvironmentQuery/EnvQueryTest.h"
#include "EnvQueryTest_ItemType.generated.h"

/**
 * Environment query test to test placeable item types
 */
UCLASS()
class ECOSCAPE_API UEnvQueryTest_ItemType : public UEnvQueryTest
{
	GENERATED_BODY()

public:
	UEnvQueryTest_ItemType();

	UPROPERTY(EditDefaultsOnly, Category = Test)
	FString ValidItemTypes;

	virtual void RunTest(FEnvQueryInstance& QueryInstance) const override;
	
	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};

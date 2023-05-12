// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "DataProviders/AIDataProvider.h"
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "EnvQueryGenerator_TerrainPlayableSpace.generated.h"

/**
 * EQS Generator for the playable space in an AEcoscapeTerrain
 */
UCLASS()
class ECOSCAPE_API UEnvQueryGenerator_TerrainPlayableSpace : public UEnvQueryGenerator
{
	GENERATED_BODY()

public:
	UEnvQueryGenerator_TerrainPlayableSpace();

	UPROPERTY(EditDefaultsOnly, Category=Generator)
	FAIDataProviderIntValue TerrainIndex;

	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;
	virtual void ProcessItems(FEnvQueryInstance& QueryInstance, TArray<FVector>& MatchingActors) const {}

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};

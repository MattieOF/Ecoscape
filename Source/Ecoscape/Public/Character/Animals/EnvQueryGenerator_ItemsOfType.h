// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "DataProviders/AIDataProvider.h"
#include "EnvironmentQuery/EnvQueryGenerator.h"
#include "EnvQueryGenerator_ItemsOfType.generated.h"

class UPlaceableItemData;
/**
 * Generator to find placed items of type within a range
 */
UCLASS()
class ECOSCAPE_API UEnvQueryGenerator_ItemsOfType : public UEnvQueryGenerator
{
	GENERATED_BODY()

public:
	UEnvQueryGenerator_ItemsOfType();

	UPROPERTY(EditDefaultsOnly, Category=Generator)
	UPlaceableItemData* SearchedItemData;

	/** If true, this will only return items of the specified type within the SearchRadius of the SearchCenter context.  If false, it will return ALL items of the specified type in the world. */
	UPROPERTY(EditDefaultsOnly, Category=Generator)
	FAIDataProviderBoolValue GenerateOnlyItemsInRadius;

	/** Max distance of path between point and context.  NOTE: Zero and negative values will never return any results if
	  * UseRadius is true.  "Within" requires Distance < Radius.  Actors ON the circle (Distance == Radius) are excluded.
	  */
	UPROPERTY(EditDefaultsOnly, Category=Generator)
	FAIDataProviderFloatValue SearchRadius;

	UPROPERTY(EditDefaultsOnly, Category=Generator)
	FAIDataProviderBoolValue CheckPathExists;

	/** context */
	UPROPERTY(EditAnywhere, Category=Generator)
	TSubclassOf<UEnvQueryContext> SearchCenter;

	virtual void GenerateItems(FEnvQueryInstance& QueryInstance) const override;
	virtual void ProcessItems(FEnvQueryInstance& QueryInstance, TArray<AActor*>& MatchingActors) const {}

	virtual FText GetDescriptionTitle() const override;
	virtual FText GetDescriptionDetails() const override;
};

// copyright lololol


#include "Character/Animals/EnvQueryGenerator_ItemsOfType.h"

#include "EngineUtils.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Actor.h"
#include "World/PlacedItem.h"

UEnvQueryGenerator_ItemsOfType::UEnvQueryGenerator_ItemsOfType()
{
	ItemType = UEnvQueryItemType_Actor::StaticClass();

	GenerateOnlyItemsInRadius.DefaultValue = true;
	SearchRadius.DefaultValue = 7500.f;
	SearchCenter = UEnvQueryContext_Querier::StaticClass();
}

void UEnvQueryGenerator_ItemsOfType::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	if (SearchedItemData == nullptr)
		return;

	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
		return;

	const UWorld* World = GEngine->GetWorldFromContextObject(QueryOwner, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
		return;

	GenerateOnlyItemsInRadius.BindData(QueryOwner, QueryInstance.QueryID);
	const bool bUseRadius = GenerateOnlyItemsInRadius.GetValue();

	TArray<AActor*> MatchingActors;
	if (bUseRadius)
	{
		TArray<FVector> ContextLocations;
		QueryInstance.PrepareContext(SearchCenter, ContextLocations);

		SearchRadius.BindData(QueryOwner, QueryInstance.QueryID);
		const float RadiusValue = SearchRadius.GetValue();
		const float RadiusSq = FMath::Square(RadiusValue);

		for (TActorIterator<APlacedItem> ItActor = TActorIterator<APlacedItem>(World, APlacedItem::StaticClass()); ItActor; ++ItActor)
		{
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ++ContextIndex)
			{
				if (FVector::DistSquared(ContextLocations[ContextIndex], ItActor->GetActorLocation()) < RadiusSq
					&& ItActor->GetItemData() == SearchedItemData)
				{
					MatchingActors.Add(*ItActor);
					break;
				}
			}
		}
	}

	ProcessItems(QueryInstance, MatchingActors);
	QueryInstance.AddItemData<UEnvQueryItemType_Actor>(MatchingActors);
}

FText UEnvQueryGenerator_ItemsOfType::GetDescriptionTitle() const
{
	return Super::GetDescriptionTitle();
}

FText UEnvQueryGenerator_ItemsOfType::GetDescriptionDetails() const
{
	return Super::GetDescriptionDetails();
}

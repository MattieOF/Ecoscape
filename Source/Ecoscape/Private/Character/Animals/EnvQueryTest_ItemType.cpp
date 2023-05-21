// copyright lololol

#include "Character\Animals\EnvQueryTest_ItemType.h"

#include "EcoscapeLog.h"
#include "EcoscapeStatics.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "Kismet/KismetStringLibrary.h"
#include "World/PlacedItem.h"

UEnvQueryTest_ItemType::UEnvQueryTest_ItemType()
{
	Cost = EEnvTestCost::Medium;
	TestPurpose = EEnvTestPurpose::Filter;
	ValidItemType = UEnvQueryItemType_ActorBase::StaticClass();
}

void UEnvQueryTest_ItemType::RunTest(FEnvQueryInstance& QueryInstance) const
{
	const TArray<FString> ValidTypes = UKismetStringLibrary::ParseIntoArray(ValidItemTypes, ";");

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		AActor* Item = GetItemActor(QueryInstance, It.GetIndex());
		const APlacedItem* PlacedItem = Cast<APlacedItem>(Item);

		// Check it isin fact a placed item
		if (!PlacedItem)
		{
			It.ForceItemState(EEnvItemStatus::Failed, 0);
			continue;
		}
		
		// Now check the type
		if (ValidTypes.Contains(PlacedItem->GetItemData()->GetName()))
			It.ForceItemState(EEnvItemStatus::Passed, 1);
		else
			It.ForceItemState(EEnvItemStatus::Failed, 0);
	}	
}

FText UEnvQueryTest_ItemType::GetDescriptionTitle() const
{
	return FText::FromString("Check objects are placed items of type");
}

FText UEnvQueryTest_ItemType::GetDescriptionDetails() const
{
	const TArray<FString> ValidTypes = UKismetStringLibrary::ParseIntoArray(ValidItemTypes, ";");
	return FText::FromString(FString::Printf(TEXT("Valid types: %s"), *UEcoscapeStatics::JoinStringArray(ValidTypes, ",")));
}

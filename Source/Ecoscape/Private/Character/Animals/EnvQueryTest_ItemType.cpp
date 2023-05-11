// copyright lololol

#include "Character\Animals\EnvQueryTest_ItemType.h"

#include "EcoscapeStatics.h"
#include "Kismet/KismetStringLibrary.h"
#include "World/PlacedItem.h"

UEnvQueryTest_ItemType::UEnvQueryTest_ItemType()
{
	Cost = EEnvTestCost::Low;
	TestPurpose = EEnvTestPurpose::Filter;
}

void UEnvQueryTest_ItemType::RunTest(FEnvQueryInstance& QueryInstance) const
{
	const TArray<FString> ValidTypes = UKismetStringLibrary::ParseIntoArray(ValidItemTypes, ";");

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		AActor* Item = GetItemActor(QueryInstance, It.GetIndex());
		const APlacedItem* PlacedItem = Cast<APlacedItem>(Item);

		// Check it is in fact a placed item
		if (!PlacedItem)
		{
			It.SetScore(TestPurpose, FilterType, false, true);
			continue;
		}

		// Now check the type
		if (ValidTypes.Contains(PlacedItem->GetItemData()->GetName()))
			It.SetScore(TestPurpose, FilterType, true, true);
		else
			It.SetScore(TestPurpose, FilterType, false, true);
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

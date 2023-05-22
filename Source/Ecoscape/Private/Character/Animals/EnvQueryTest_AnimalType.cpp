// copyright lololol

#include "Character\Animals\EnvQueryTest_AnimalType.h"

#include "EcoscapeStatics.h"
#include "Character/Animals/BaseAnimal.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_ActorBase.h"
#include "Kismet/KismetStringLibrary.h"

UEnvQueryTest_AnimalType::UEnvQueryTest_AnimalType()
{
	Cost = EEnvTestCost::Medium;
	TestPurpose = EEnvTestPurpose::Filter;
	ValidItemType = UEnvQueryItemType_ActorBase::StaticClass();
}

void UEnvQueryTest_AnimalType::RunTest(FEnvQueryInstance& QueryInstance) const
{
	const TArray<FString> ValidTypes = UKismetStringLibrary::ParseIntoArray(ValidAnimalTypes, ";");

	for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
	{
		AActor* Item = GetItemActor(QueryInstance, It.GetIndex());
		const ABaseAnimal* Animal = Cast<ABaseAnimal>(Item);

		// Check it isin fact a placed item
		if (!Animal)
		{
			It.ForceItemState(EEnvItemStatus::Failed, 0);
			continue;
		}
		
		// Now check the type
		if (ValidTypes.Contains(Animal->AnimalData->GetName()))
			It.ForceItemState(EEnvItemStatus::Passed, 1);
		else
			It.ForceItemState(EEnvItemStatus::Failed, 0);
	}	
}

FText UEnvQueryTest_AnimalType::GetDescriptionTitle() const
{
	return FText::FromString("Check objects are animals of type");
}

FText UEnvQueryTest_AnimalType::GetDescriptionDetails() const
{
	const TArray<FString> ValidTypes = UKismetStringLibrary::ParseIntoArray(ValidAnimalTypes, ";");
	return FText::FromString(FString::Printf(TEXT("Valid types: %s"), *UEcoscapeStatics::JoinStringArray(ValidTypes, ",")));
}

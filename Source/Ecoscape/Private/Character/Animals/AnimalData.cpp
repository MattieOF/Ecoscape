// copyright lololol

#include "Character/Animals/AnimalData.h"

#include "Character/Animals/BaseAnimal.h"
#include "UObject/ConstructorHelpers.h"

UAnimalData::UAnimalData()
{
	static ConstructorHelpers::FClassFinder<APawn> DefaultAnimalClass(TEXT("Blueprint'/Game/Blueprints/BP_BaseAnimal.BP_BaseAnimal_C'"));
	if (DefaultAnimalClass.Succeeded())
		AnimalClass = DefaultAnimalClass.Class;
	else
		AnimalClass = ABaseAnimal::StaticClass();
}

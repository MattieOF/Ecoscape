// copyright lololol

#include "AssetFactories.h"

#include "Character/Animals/AnimalData.h"
#include "UI/Codex.h"
#include "World/PlaceableItemData.h"

UPlaceableItemFactory::UPlaceableItemFactory()
{
	SupportedClass = UPlaceableItemData::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UPlaceableItemFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UPlaceableItemData>(InParent, Class, Name, Flags, Context);
}

UAnimalDataFactory::UAnimalDataFactory()
{
	SupportedClass = UAnimalData::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UAnimalDataFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UAnimalData>(InParent, Class, Name, Flags, Context);
}

UCodexEntryFactory::UCodexEntryFactory()
{
	SupportedClass = UCodexEntry::StaticClass();
	bEditAfterNew = true;
	bCreateNew = true;
}

UObject* UCodexEntryFactory::FactoryCreateNew(UClass* Class, UObject* InParent, FName Name, EObjectFlags Flags,
	UObject* Context, FFeedbackContext* Warn)
{
	return NewObject<UCodexEntry>(InParent, Class, Name, Flags, Context);
}

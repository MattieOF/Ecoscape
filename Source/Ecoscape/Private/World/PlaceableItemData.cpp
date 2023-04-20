// copyright lololol

#include "World/PlaceableItemData.h"

#include "EcoscapeLog.h"

UPlaceableItemData::UPlaceableItemData()
{
	ColourRangeSquared = ColourRange * ColourRange;
}

UPlaceableItemData* UItemDataList::GetRandomItem()
{
	if (Options.Num() <= 0)
	{
		UE_LOG(LogEcoscape, Error, TEXT("In an item data list, we tried to get a random item, but there are 0 options!"));
		return nullptr;
	}

	const int Index = FMath::RandRange(0, Options.Num() - 1);
	return Options[Index];
}

// copyright lololol

#include "Character/Animals/EnvQueryGenerator_TerrainPlayableSpace.h"

#include "EcoscapeGameInstance.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_Point.h"
#include "World/EcoscapeTerrain.h"

UEnvQueryGenerator_TerrainPlayableSpace::UEnvQueryGenerator_TerrainPlayableSpace()
{
	TerrainIndex.DefaultValue = 0;
}

void UEnvQueryGenerator_TerrainPlayableSpace::GenerateItems(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	if (QueryOwner == nullptr)
	{
		return;
	}
	
	UWorld* World = GEngine->GetWorldFromContextObject(QueryOwner, EGetWorldErrorMode::LogAndReturnNull);
	if (World == nullptr)
	{
		return;
	}

	TArray<FVector> Positions;
	
	TerrainIndex.BindData(QueryOwner, QueryInstance.QueryID);
	const int TerrainIndexValue = TerrainIndex.GetValue();

	AEcoscapeTerrain* Terrain = UEcoscapeGameInstance::GetEcoscapeGameInstance(World)->GetTerrainFromIndex(TerrainIndexValue);
	Terrain->GetPlayablePoints(Positions);

	ProcessItems(QueryInstance, Positions);
	QueryInstance.AddItemData<UEnvQueryItemType_Point>(Positions);
}

FText UEnvQueryGenerator_TerrainPlayableSpace::GetDescriptionTitle() const
{
	return FText::FromString("Get playable points in terrain");
}

FText UEnvQueryGenerator_TerrainPlayableSpace::GetDescriptionDetails() const
{
	return FText::FromString("What the title says");
}

// Copyright Epic Games, Inc. All Rights Reserved.

#include "Character/Animals/EnvQueryTest_Pathfinding_Ecoscape.h"

#include "EcoscapeStatics.h"
#include "AI/Navigation/NavAgentInterface.h"
#include "Engine/World.h"
#include "NavigationData.h"
#include "NavigationSystem.h"
#include "EnvironmentQuery/Items/EnvQueryItemType_VectorBase.h"
#include "EnvironmentQuery/Contexts/EnvQueryContext_Querier.h"

#define LOCTEXT_NAMESPACE "Ecoscape"

UEnvQueryTest_Pathfinding_Ecoscape::UEnvQueryTest_Pathfinding_Ecoscape(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	Context = UEnvQueryContext_Querier::StaticClass();
	Cost = EEnvTestCost::High;
	ValidItemType = UEnvQueryItemType_VectorBase::StaticClass();
	TestMode = EEnvTestPathfinding::PathExist;
	PathFromContext.DefaultValue = true;
	SkipUnreachable.DefaultValue = true;
	FloatValueMin.DefaultValue = 1000.0f;
	FloatValueMax.DefaultValue = 1000.0f;

	SetWorkOnFloatValues(TestMode != EEnvTestPathfinding::PathExist);
}

void UEnvQueryTest_Pathfinding_Ecoscape::RunTest(FEnvQueryInstance& QueryInstance) const
{
	UObject* QueryOwner = QueryInstance.Owner.Get();
	BoolValue.BindData(QueryOwner, QueryInstance.QueryID);
	PathFromContext.BindData(QueryOwner, QueryInstance.QueryID);
	SkipUnreachable.BindData(QueryOwner, QueryInstance.QueryID);
	FloatValueMin.BindData(QueryOwner, QueryInstance.QueryID);
	FloatValueMax.BindData(QueryOwner, QueryInstance.QueryID);

	bool bWantsPath = BoolValue.GetValue();
	bool bPathToItem = PathFromContext.GetValue();
	bool bDiscardFailed = SkipUnreachable.GetValue();
	float MinThresholdValue = FloatValueMin.GetValue();
	float MaxThresholdValue = FloatValueMax.GetValue();

	UNavigationSystemV1* NavSys = FNavigationSystem::GetCurrent<UNavigationSystemV1>(QueryInstance.World);
	if (NavSys == nullptr || QueryOwner == nullptr)
	{
		return;
	}

	ANavigationData* NavData = FindNavigationData(*NavSys, QueryOwner);
	if (NavData == nullptr)
	{
		return;
	}

	TArray<FVector> ContextLocations;
	if (!QueryInstance.PrepareContext(Context, ContextLocations))
	{
		return;
	}

	EPathFindingMode::Type PFMode(EPathFindingMode::Regular);
	FSharedConstNavQueryFilter NavFilter = UNavigationQueryFilter::GetQueryFilter(*NavData, QueryOwner, FilterClass);

	if (GetWorkOnFloatValues())
	{
		FFindPathSignature FindPathFunc;
		FindPathFunc.BindUObject(this, TestMode == EEnvTestPathfinding::PathLength ?
			(bPathToItem ? &UEnvQueryTest_Pathfinding_Ecoscape::FindPathLengthTo : &UEnvQueryTest_Pathfinding_Ecoscape::FindPathLengthFrom) :
			(bPathToItem ? &UEnvQueryTest_Pathfinding_Ecoscape::FindPathCostTo : &UEnvQueryTest_Pathfinding_Ecoscape::FindPathCostFrom) );

		NavData->BeginBatchQuery();
		for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
		{
			FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
			if (bUseBase)
				ItemLocation -= FVector(0, 0, UEcoscapeStatics::GetZUnderOrigin(GetItemActor(QueryInstance, It.GetIndex())));
			for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
			{
				const float PathValue = FindPathFunc.Execute(ItemLocation, ContextLocations[ContextIndex], PFMode, *NavData, *NavSys, NavFilter, QueryOwner);
				It.SetScore(TestPurpose, FilterType, PathValue, MinThresholdValue, MaxThresholdValue);

				if (bDiscardFailed && PathValue >= BIG_NUMBER)
				{
					It.ForceItemState(EEnvItemStatus::Failed);
				}
			}
		}
		NavData->FinishBatchQuery();
	}
	else
	{
		NavData->BeginBatchQuery();
		if (bPathToItem)
		{
			for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
			{
				FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
				if (bUseBase)
					ItemLocation -= FVector(0, 0, UEcoscapeStatics::GetZUnderOrigin(GetItemActor(QueryInstance, It.GetIndex())));
				for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
				{
					const bool bFoundPath = TestPathTo(ItemLocation, ContextLocations[ContextIndex], PFMode, *NavData, *NavSys, NavFilter, QueryOwner);
					It.SetScore(TestPurpose, FilterType, bFoundPath, bWantsPath);
				}
			}
		}
		else
		{
			for (FEnvQueryInstance::ItemIterator It(this, QueryInstance); It; ++It)
			{
				FVector ItemLocation = GetItemLocation(QueryInstance, It.GetIndex());
				if (bUseBase)
					ItemLocation -= FVector(0, 0, UEcoscapeStatics::GetZUnderOrigin(GetItemActor(QueryInstance, It.GetIndex())));
				for (int32 ContextIndex = 0; ContextIndex < ContextLocations.Num(); ContextIndex++)
				{
					const bool bFoundPath = TestPathFrom(ItemLocation, ContextLocations[ContextIndex], PFMode, *NavData, *NavSys, NavFilter, QueryOwner);
					It.SetScore(TestPurpose, FilterType, bFoundPath, bWantsPath);
				}
			}
		}
		NavData->FinishBatchQuery();
	}
}

FText UEnvQueryTest_Pathfinding_Ecoscape::GetDescriptionTitle() const
{
	FString ModeDesc[] = { TEXT("PathExist"), TEXT("PathCost"), TEXT("PathLength") };

	FString DirectionDesc = PathFromContext.IsDynamic() ?
		FString::Printf(TEXT("%s, direction: %s"), *UEnvQueryTypes::DescribeContext(Context).ToString(), *PathFromContext.ToString()) :
		FString::Printf(TEXT("%s %s"), PathFromContext.DefaultValue ? TEXT("from") : TEXT("to"), *UEnvQueryTypes::DescribeContext(Context).ToString());

	return FText::FromString(FString::Printf(TEXT("%s: %s (Ecoscape)"), *ModeDesc[TestMode], *DirectionDesc));
}

FText UEnvQueryTest_Pathfinding_Ecoscape::GetDescriptionDetails() const
{
	FText DiscardDesc = LOCTEXT("DiscardUnreachable", "discard unreachable");
	FText Desc2;
	if (SkipUnreachable.IsDynamic())
	{
		Desc2 = FText::Format(FText::FromString("{0}: {1}"), DiscardDesc, FText::FromString(SkipUnreachable.ToString()));
	}
	else if (SkipUnreachable.DefaultValue)
	{
		Desc2 = DiscardDesc;
	}

	FText TestParamDesc = GetWorkOnFloatValues() ? DescribeFloatTestParams() : DescribeBoolTestParams("existing path");
	if (!Desc2.IsEmpty())
	{
		return FText::Format(FText::FromString("{0}\n{1}"), Desc2, TestParamDesc);
	}

	return TestParamDesc;
}

#if WITH_EDITOR
void UEnvQueryTest_Pathfinding_Ecoscape::PostEditChangeProperty(FPropertyChangedEvent& PropertyChangedEvent)
{
	Super::PostEditChangeProperty(PropertyChangedEvent);
	if (PropertyChangedEvent.Property && PropertyChangedEvent.Property->GetFName() == GET_MEMBER_NAME_CHECKED(UEnvQueryTest_Pathfinding_Ecoscape,TestMode))
	{
		SetWorkOnFloatValues(TestMode != EEnvTestPathfinding::PathExist);
	}
}
#endif

void UEnvQueryTest_Pathfinding_Ecoscape::PostLoad()
{
	Super::PostLoad();
	
	SetWorkOnFloatValues(TestMode != EEnvTestPathfinding::PathExist);
}

bool UEnvQueryTest_Pathfinding_Ecoscape::TestPathFrom(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys, FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const
{
	FPathFindingQuery Query(PathOwner, NavData, ItemPos, ContextPos, NavFilter);
	Query.SetAllowPartialPaths(false);

	const bool bPathExists = NavSys.TestPathSync(Query, Mode);
	return bPathExists;
}

bool UEnvQueryTest_Pathfinding_Ecoscape::TestPathTo(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys, FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const
{
	FPathFindingQuery Query(PathOwner, NavData, ContextPos, ItemPos, NavFilter);
	Query.SetAllowPartialPaths(false);

	const bool bPathExists = NavSys.TestPathSync(Query, Mode);
	return bPathExists;
}

float UEnvQueryTest_Pathfinding_Ecoscape::FindPathCostFrom(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys, FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const
{
	FPathFindingQuery Query(PathOwner, NavData, ItemPos, ContextPos, NavFilter);
	Query.SetAllowPartialPaths(false);

	FPathFindingResult Result = NavSys.FindPathSync(Query, Mode);
	return (Result.IsSuccessful()) ? Result.Path->GetCost() : BIG_NUMBER;
}

float UEnvQueryTest_Pathfinding_Ecoscape::FindPathCostTo(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys, FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const
{
	FPathFindingQuery Query(PathOwner, NavData, ContextPos, ItemPos, NavFilter);
	Query.SetAllowPartialPaths(false);

	FPathFindingResult Result = NavSys.FindPathSync(Query, Mode);
	return (Result.IsSuccessful()) ? Result.Path->GetCost() : BIG_NUMBER;
}

float UEnvQueryTest_Pathfinding_Ecoscape::FindPathLengthFrom(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys, FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const
{
	FPathFindingQuery Query(PathOwner, NavData, ItemPos, ContextPos, NavFilter);
	Query.SetAllowPartialPaths(false);

	FPathFindingResult Result = NavSys.FindPathSync(Query, Mode);
	return (Result.IsSuccessful()) ? Result.Path->GetLength() : BIG_NUMBER;
}

float UEnvQueryTest_Pathfinding_Ecoscape::FindPathLengthTo(const FVector& ItemPos, const FVector& ContextPos, EPathFindingMode::Type Mode, const ANavigationData& NavData, UNavigationSystemV1& NavSys, FSharedConstNavQueryFilter NavFilter, const UObject* PathOwner) const
{
	FPathFindingQuery Query(PathOwner, NavData, ContextPos, ItemPos, NavFilter);
	Query.SetAllowPartialPaths(false);

	FPathFindingResult Result = NavSys.FindPathSync(Query, Mode);
	return (Result.IsSuccessful()) ? Result.Path->GetLength() : BIG_NUMBER;
}

ANavigationData* UEnvQueryTest_Pathfinding_Ecoscape::FindNavigationData(UNavigationSystemV1& NavSys, UObject* Owner) const
{
	INavAgentInterface* NavAgent = Cast<INavAgentInterface>(Owner);
	if (NavAgent)
	{
		return NavSys.GetNavDataForProps(NavAgent->GetNavAgentPropertiesRef(), NavAgent->GetNavAgentLocation());
	}

	return NavSys.GetDefaultNavDataInstance(FNavigationSystem::DontCreate);
}

#undef LOCTEXT_NAMESPACE

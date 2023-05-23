// copyright lololol

#include "BTComposite_Random.h"

UBTComposite_Random::UBTComposite_Random(const FObjectInitializer& ObjectInitializer) : Super(ObjectInitializer)
{
	NodeName = "Random";
}

int32 UBTComposite_Random::GetNextChildHandler(FBehaviorTreeSearchData& SearchData, int32 PrevChild,
	EBTNodeResult::Type LastResult) const
{
	int32 NextChildIdx = BTSpecialChild::ReturnToParent;

	if (GetChildrenNum() > 0)
		NextChildIdx = FMath::RandRange(0, GetChildrenNum() - 1);

	return NextChildIdx;
}

#if WITH_EDITOR
FName UBTComposite_Random::GetNodeIconName() const
{
	return FName("BTEditor.Graph.BTNode.Composite.Selector.Icon");
}
#endif

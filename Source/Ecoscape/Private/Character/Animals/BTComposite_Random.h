// copyright lololol

#pragma once

#include "CoreMinimal.h"
#include "BehaviorTree/BTCompositeNode.h"
#include "BTComposite_Random.generated.h"

/**
 * Random composite node.
 * Random Nodes execute a random one of their children.
 * If that child fails, it tries again if there is one.
 */
UCLASS()
class ECOSCAPE_API UBTComposite_Random : public UBTCompositeNode
{
	GENERATED_UCLASS_BODY()

	virtual int32 GetNextChildHandler(struct FBehaviorTreeSearchData& SearchData, int32 PrevChild, EBTNodeResult::Type LastResult) const override;

#if WITH_EDITOR
	virtual FName GetNodeIconName() const override;
#endif
};

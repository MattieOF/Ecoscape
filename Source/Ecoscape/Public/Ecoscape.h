// copyright lololol

#pragma once

#include "CoreMinimal.h"

class FEcoscapeModule: public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

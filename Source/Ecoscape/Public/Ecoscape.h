// copyright lololol

#pragma once

#include "CoreMinimal.h"

#define ECO_LOG(x) FMessageLog("Ecoscape").Info(FText::FromString(x));
#define ECO_LOG_ERROR(x) FMessageLog("Ecoscape").Error(FText::FromString(x));\
		{ FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");\
		MessageLogModule.OpenMessageLog("Ecoscape");}

class FEcoscapeModule: public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

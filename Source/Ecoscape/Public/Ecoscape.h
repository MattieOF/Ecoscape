// copyright lololol

#pragma once

#include "CoreMinimal.h"

#if WITH_EDITOR
#define ECO_LOG(x) FMessageLog("Ecoscape").Info(FText::FromString(x));
#define ECO_LOG_ERROR(x) do { FMessageLog("Ecoscape").Error(FText::FromString(x));\
		{ FMessageLogModule& MessageLogModule = FModuleManager::LoadModuleChecked<FMessageLogModule>("MessageLog");\
		MessageLogModule.OpenMessageLog("Ecoscape");} } while(false)
#else
// TODO: For now we strip out eco_logs, but it should maybe replace with just a UE_LOG
#define ECO_LOG(x)
#define ECO_LOG_ERROR(x) 
#endif

class FEcoscapeModule: public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;
};

// copyright lololol

#include "Ecoscape.h"

#include "EcoscapeLog.h"
#include "Modules/ModuleManager.h"

DEFINE_LOG_CATEGORY(LogEcoscape)

IMPLEMENT_PRIMARY_GAME_MODULE( FEcoscapeModule, Ecoscape, "Ecoscape" );

#define LOCTEXT_NAMESPACE "Ecoscape"

void FEcoscapeModule::StartupModule()
{
	UE_LOG(LogEcoscape, Log, TEXT("Started main Ecoscape module!"));
}

void FEcoscapeModule::ShutdownModule()
{
	UE_LOG(LogEcoscape, Log, TEXT("Shutdown main Ecoscape module!"));
}

#undef LOCTEXT_NAMESPACE

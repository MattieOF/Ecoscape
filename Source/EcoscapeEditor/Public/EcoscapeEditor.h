#pragma once

#include "Modules/ModuleInterface.h"
#include "Modules/ModuleManager.h"

DECLARE_LOG_CATEGORY_EXTERN(LogEcoscapeEditor, All, All)

class FEcoscapeEditorModule: public IModuleInterface
{
public:
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void AddMenuEntries(FMenuBuilder& MenuBuilder);
	void GeneratePlaceableData();
	void RefreshItemDir();

};

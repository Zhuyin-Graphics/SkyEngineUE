// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

struct FSkyEngineExportConfig;

class FSkyEngineUEModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void ExportWorld(const FSkyEngineExportConfig& config);
	static void ExportHLOD();

private:
	void PluginButtonClicked();
	void RegisterMenus();

	TSharedPtr<FUICommandList> PluginCommands;
};

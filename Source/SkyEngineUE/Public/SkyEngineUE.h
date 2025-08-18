// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

class FSkyEngineUEModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

private:
	void PluginButtonClicked();
	void RegisterMenus();

	TSharedPtr<FUICommandList> PluginCommands;
};

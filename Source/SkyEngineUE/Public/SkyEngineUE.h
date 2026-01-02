// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"
#include "SkyEngineContext.h"

struct FSkyEngineExportConfig;
struct FSkyEngineExportContext;

class FSkyEngineUEModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void UpdateSkyEnv(const FSkyEngineExportConfig& config);
	static void ExportWorld(const FSkyEngineExportConfig& config);
	static void ExportHLOD();

	static void Export(const FSkyEngineExportContext& context);

private:
	void PluginButtonClicked();
	void RegisterMenus();

	TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);
	void CreateAssetContextMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
	void AddPluginSubMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
	void OnProcessAssetsClicked(TArray<FAssetData> SelectedAssets);

	TSharedPtr<FUICommandList> PluginCommands;
};

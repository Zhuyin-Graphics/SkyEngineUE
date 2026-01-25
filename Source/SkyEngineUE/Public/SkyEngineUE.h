// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "Modules/ModuleManager.h"

struct FSkyEngineExportConfig;

namespace sky {
	struct SkyEngineExportContext;
} // namespace sky

class FSkyEngineUEModule : public IModuleInterface
{
public:

	/** IModuleInterface implementation */
	virtual void StartupModule() override;
	virtual void ShutdownModule() override;

	static void UpdateSkyEnv(const FSkyEngineExportConfig& config);
	static void ExportWorld(const FSkyEngineExportConfig& config);
	static void ExportHLOD();
	static void DeleteVolumeActors();

	static void Export(const sky::SkyEngineExportContext& context);

private:
	void PluginButtonClicked();
	void RegisterMenus();

	TSharedRef<FExtender> OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets);
	void CreateAssetContextMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
	void AddPluginSubMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets);
	void OnProcessAssetsClicked(TArray<FAssetData> SelectedAssets);

	TSharedPtr<FUICommandList> PluginCommands;
};

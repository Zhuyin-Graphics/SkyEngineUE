// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkyEngineUE.h"
#include "SkyEngineUEStyle.h"
#include "SkyEngineUECommands.h"
#include "Widgets/Docking/SDockTab.h"
#include "Framework/MultiBox/MultiBoxBuilder.h"

#include "Widget/ExportConfigWidget.h"

#include "Exporter/LevelExporter.h"

#define LOCTEXT_NAMESPACE "FSkyEngineUEModule"

void FSkyEngineUEModule::StartupModule()
{
	FSkyEngineUEStyle::Initialize();
	FSkyEngineUEStyle::ReloadTextures();
	FSkyEngineUECommands::Register();

	PluginCommands = MakeShareable(new FUICommandList);
	PluginCommands->MapAction(
		FSkyEngineUECommands::Get().PluginAction,
		FExecuteAction::CreateRaw(this, &FSkyEngineUEModule::PluginButtonClicked),
		FCanExecuteAction());

	UToolMenus::RegisterStartupCallback(FSimpleMulticastDelegate::FDelegate::CreateRaw(this, &FSkyEngineUEModule::RegisterMenus));
}

void FSkyEngineUEModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FSkyEngineUEStyle::Shutdown();
	FSkyEngineUECommands::Unregister();
}

void FSkyEngineUEModule::PluginButtonClicked() // NOLINT
{
	TSharedRef<SWindow> ConfigWindow = SNew(SWindow)
		.Title(LOCTEXT("ConfigWindowTitle", "Plugin Configuration"))
		.ClientSize(FVector2D(400, 300))
		.SupportsMaximize(false)
		.SupportsMinimize(false);
            
	ConfigWindow->SetContent(
		SNew(sky::FExportConfigWidget)
		.ParentWindow(ConfigWindow)
	);
            
	FSlateApplication::Get().AddWindow(ConfigWindow);
}

void FSkyEngineUEModule::RegisterMenus() // NOLINT
{
	FToolMenuOwnerScoped OwnerScoped(this);

	{
		UToolMenu* Menu = UToolMenus::Get()->ExtendMenu("LevelEditor.MainMenu.Window");
		FToolMenuSection& Section = Menu->FindOrAddSection("WindowLayout");
		Section.AddMenuEntryWithCommandList(FSkyEngineUECommands::Get().PluginAction, PluginCommands);
	}

	{
		UToolMenu* ToolbarMenu = UToolMenus::Get()->ExtendMenu("LevelEditor.LevelEditorToolBar.PlayToolBar");
		FToolMenuSection& Section = ToolbarMenu->FindOrAddSection("PluginTools");
		FToolMenuEntry& Entry = Section.AddEntry(FToolMenuEntry::InitToolBarButton(FSkyEngineUECommands::Get().PluginAction));
		Entry.SetCommandList(PluginCommands);
	}
}


void FSkyEngineUEModule::ExportWorld(const FSkyEngineExportConfig& config)
{
	sky::LevelExport ExportTask;
	ExportTask.Run(config);
}

void FSkyEngineUEModule::ExportHLOD()
{

}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSkyEngineUEModule, SkyEngineUE)
// Copyright Epic Games, Inc. All Rights Reserved.

#include "SkyEngineUE.h"
#include "SkyEngineUEStyle.h"
#include "SkyEngineUECommands.h"

#include "Framework/MultiBox/MultiBoxBuilder.h"
#include "Widgets/Docking/SDockTab.h"
#include "Widget/ExportConfigWidget.h"
#include "ContentBrowserModule.h"
#include "SkyEngineContext.h"

#include "Exporter/LevelExporter.h"
#include "Exporter/SkeletonExporter.h"
#include "Exporter/SkeletalMeshExporter.h"
#include "Exporter/StaticMeshExporter.h"
#include "Exporter/AnimationSequenceExporter.h"

#define LOCTEXT_NAMESPACE "FSkyEngineUEModule"

std::unique_ptr<sky::SkyEngineContext> g_SkyEngine;
FSkyEngineExportConfig g_ExportConfig;

void FSkyEngineUEModule::OnProcessAssetsClicked(TArray<FAssetData> SelectedAssets)
{
	sky::SkyEngineExportContext context;

	for (const FAssetData& AssetData : SelectedAssets)
	{
		if (UAnimSequence* anim = Cast<UAnimSequence>(AssetData.GetAsset()))
		{
			sky::AnimationSequenceExport::Payload Payload = {};
			Payload.Sequence = anim;
			if (sky::AnimationSequenceExport::Gather(anim, context, Payload.Skeleton)) {
				auto* Task = new sky::AnimationSequenceExport(Payload);
				Task->Init();

				context.Tasks.Emplace(FSoftObjectPath(anim).ToString(), Task);
			}
		}
		else if (USkeletalMesh* SkeletalMesh = Cast<USkeletalMesh>(AssetData.GetAsset()))
		{
			sky::SkeletalMeshExport::Payload Payload = {};
			Payload.Mesh = SkeletalMesh;
			if (sky::SkeletalMeshExport::Gather(SkeletalMesh, context, Payload)) {
				auto* Task = new sky::SkeletalMeshExport(Payload);
				Task->Init();

				context.Tasks.Emplace(FSoftObjectPath(SkeletalMesh).ToString(), Task);
			}

		}
		else if (UStaticMesh* StaticMesh = Cast<UStaticMesh>(AssetData.GetAsset()))
		{
			sky::StaticMeshExport::Payload Payload = {};
			Payload.StaticMesh = StaticMesh;
			if (sky::StaticMeshExport::Gather(StaticMesh, context, Payload)) {
				auto* Task = new sky::StaticMeshExport(Payload);
				Task->Init();

				context.Tasks.Emplace(FSoftObjectPath(StaticMesh).ToString(), Task);
			}
		}
	}

	Export(context);
}

void FSkyEngineUEModule::AddPluginSubMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	MenuBuilder.AddMenuEntry(
		LOCTEXT("ExportAsset", "ExportAsset"),
		LOCTEXT("ExportAsset", "ExportAsset"),
		FSlateIcon(FAppStyle::GetAppStyleSetName(), "Icons.BatchProcess"),
		FUIAction(
			FExecuteAction::CreateRaw(
				this, &FSkyEngineUEModule::OnProcessAssetsClicked,
				SelectedAssets
			)
		)
	);
}

void FSkyEngineUEModule::CreateAssetContextMenu(FMenuBuilder& MenuBuilder, TArray<FAssetData> SelectedAssets)
{
	MenuBuilder.AddSubMenu(
		LOCTEXT("SkyEngine", "SkyEngine"),
		LOCTEXT("SkyEngine", "SkyEngine"),
		FNewMenuDelegate::CreateRaw(
			this, &FSkyEngineUEModule::AddPluginSubMenu,
			SelectedAssets
		)
	);
}

TSharedRef<FExtender> FSkyEngineUEModule::OnExtendContentBrowserAssetSelectionMenu(const TArray<FAssetData>& SelectedAssets)
{
	TSharedRef<FExtender> Extender(new FExtender());

	bool bHasSupportedAssets = false;
	for (const FAssetData& AssetData : SelectedAssets)
	{
		if (AssetData.AssetClassPath == UAnimSequence::StaticClass()->GetClassPathName() ||
			AssetData.AssetClassPath == USkeletalMesh::StaticClass()->GetClassPathName() ||
			AssetData.AssetClassPath == UStaticMesh::StaticClass()->GetClassPathName() ||
			AssetData.AssetClassPath == UTexture::StaticClass()->GetClassPathName())
		{
			bHasSupportedAssets = true;
			break;
		}
	}

	if (bHasSupportedAssets)
	{
		Extender->AddMenuExtension(
			"GetAssetActions",
			EExtensionHook::After,
			nullptr,
			FMenuExtensionDelegate::CreateRaw(
				this, &FSkyEngineUEModule::CreateAssetContextMenu,
				SelectedAssets
			)
		);
	}

	return Extender;
}

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

	// register asset right click menu
	FContentBrowserModule& ContentBrowserModule = FModuleManager::LoadModuleChecked<FContentBrowserModule>("ContentBrowser");
	TArray<FContentBrowserMenuExtender_SelectedAssets>& CBMenuExtenderDelegates = ContentBrowserModule.GetAllAssetViewContextMenuExtenders();
	CBMenuExtenderDelegates.Add(
		FContentBrowserMenuExtender_SelectedAssets::CreateRaw(
			this, &FSkyEngineUEModule::OnExtendContentBrowserAssetSelectionMenu
		)
	);

}

void FSkyEngineUEModule::ShutdownModule()
{
	UToolMenus::UnRegisterStartupCallback(this);
	UToolMenus::UnregisterOwner(this);

	FSkyEngineUEStyle::Shutdown();
	FSkyEngineUECommands::Unregister();

	g_SkyEngine = nullptr;
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

void FSkyEngineUEModule::UpdateSkyEnv(const FSkyEngineExportConfig& config)
{
	if (config.SkyEnginePath.IsEmpty() || config.SkyProjectpath.IsEmpty())
	{
		g_SkyEngine = nullptr;
	}
	else
	{
		g_SkyEngine = std::make_unique<sky::SkyEngineContext>(TCHAR_TO_UTF8(*config.SkyEnginePath), TCHAR_TO_UTF8(*config.SkyProjectpath));
	}

	g_ExportConfig = config;
}

void FSkyEngineUEModule::ExportWorld(const FSkyEngineExportConfig& config)
{
	sky::LevelExport ExportTask;
	ExportTask.Run(config);
}

void FSkyEngineUEModule::ExportHLOD()
{

}

void FSkyEngineUEModule::Export(const sky::SkyEngineExportContext& context)
{
	for (const auto& [Guid, Task] : context.Tasks) {
		Task->Run();
	}

	if (g_SkyEngine != nullptr) {
		g_SkyEngine->Save();
	}
}

#undef LOCTEXT_NAMESPACE
	
IMPLEMENT_MODULE(FSkyEngineUEModule, SkyEngineUE)
#include "ExportConfigWidget.h"
#include "SkyEngineUEExport.h"
#include "SkyEngineUE.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SUniformGridPanel.h"

#include "Misc/Paths.h"
#include "Misc/FileHelper.h"
#include "Serialization/JsonSerializer.h"
#include "Serialization/JsonWriter.h"

#define LOCTEXT_NAMESPACE "ConfigWindow"

namespace sky
{
	FExportConfigWidget::FExportConfigWidget()
	{
		LoadConfig();
	}

	FExportConfigWidget::~FExportConfigWidget()
	{
		SaveConfig();
	}

	void FExportConfigWidget::Construct(const FArguments& InArgs)
	{
		ParentWindow = InArgs._ParentWindow;

		ChildSlot
	    [
	        SNew(SBox)
	        .WidthOverride(400.f)
	        .HeightOverride(300.f)
	        [
	        	SNew(SVerticalBox)
            
				+SVerticalBox::Slot()
				.Padding(10)
				.AutoHeight()
				[
					SNew(SVerticalBox)
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ExportPathLabel", "SkyEngine Path: "))
					]
					+ SVerticalBox::Slot()
					[
						SNew(SEditableTextBox)
						.Text(FText::FromString(ConfigValue.SkyEnginePath))
						.OnTextChanged_Lambda([this](const FText& NewText) {
							ConfigValue.SkyEnginePath = NewText.ToString();
						})
					]
					+ SVerticalBox::Slot()
					.AutoHeight()
					[
						SNew(STextBlock)
						.Text(LOCTEXT("ExportPathLabel", "Export Project Path: "))
					]
					+ SVerticalBox::Slot()
					[
						SNew(SEditableTextBox)
						.Text(FText::FromString(ConfigValue.SkyProjectpath))
						.OnTextChanged_Lambda([this](const FText& NewText) {
							ConfigValue.SkyProjectpath = NewText.ToString();
						})
					]
					+ SVerticalBox::Slot()
						.Padding(10)
						[
							SNew(SUniformGridPanel)
								+ SUniformGridPanel::Slot(0, 0)
								[
									SNew(SButton)
										.Text(LOCTEXT("Init Sky Project Environment", "Init"))
										.OnClicked(this, &FExportConfigWidget::OnEvnInitClicked)
								]
						]
					+ SVerticalBox::Slot()
					[
						SNew(SCheckBox)
						.IsChecked(ConfigValue.bExportGeometry ? ECheckBoxState::Checked : ECheckBoxState::Unchecked)
						.OnCheckStateChanged_Lambda([this](ECheckBoxState NewState) {
						ConfigValue.bExportGeometry = (NewState == ECheckBoxState::Checked);
						})
						[
							SNew(STextBlock).Text(LOCTEXT("ExportGeometry", "Export Geometry"))
						]
					]
					+ SVerticalBox::Slot()
					.Padding(10)
					[
						SNew(SUniformGridPanel)
						+ SUniformGridPanel::Slot(0, 0)
						[
							SNew(SButton)
								.Text(LOCTEXT("Export UE World", "Full Export"))
								.OnClicked(this, &FExportConfigWidget::OnExportFullClicked)
						]
					]
				]
	        ]
	    ];
		
	}

	FReply FExportConfigWidget::OnExportFullClicked() // NOLINT
	{
		FSkyEngineUEModule::ExportWorld(ConfigValue);
		return FReply::Handled();
	}

	FReply FExportConfigWidget::OnEvnInitClicked() // NOLINT
	{
		FSkyEngineUEModule::UpdateSkyEnv(ConfigValue);
		return FReply::Handled();
	}

	FReply FExportConfigWidget::OnCloseClicked() // NOLINT
	{
		if (ParentWindow.IsValid())
		{
			ParentWindow.Pin()->RequestDestroyWindow();
		}
		return FReply::Handled();
	}

	void FExportConfigWidget::LoadConfig()
	{
		FString IntermediateDir = FPaths::ProjectIntermediateDir();
		FString ConfigFilePath = FPaths::Combine(IntermediateDir, TEXT("SkyEngineExport.json"));

		FString FileContent;
		if (!FFileHelper::LoadFileToString(FileContent, *ConfigFilePath))
		{
			return;
		}

		TSharedPtr<FJsonObject> JsonObject;
		TSharedRef<TJsonReader<>> Reader = TJsonReaderFactory<>::Create(FileContent);

		if (!FJsonSerializer::Deserialize(Reader, JsonObject) || !JsonObject.IsValid())
		{
			return;
		}

		JsonObject->TryGetStringField(TEXT("EnginePath"), ConfigValue.SkyEnginePath);
		JsonObject->TryGetStringField(TEXT("ProjectPath"), ConfigValue.SkyProjectpath);

		FSkyEngineUEModule::UpdateSkyEnv(ConfigValue);
	}

	void FExportConfigWidget::SaveConfig() const
	{
		FString IntermediateDir = FPaths::ProjectIntermediateDir();
		FString ConfigFilePath = FPaths::Combine(IntermediateDir, TEXT("SkyEngineExport.json"));

		TSharedPtr<FJsonObject> ConfigObject = MakeShared<FJsonObject>();
		ConfigObject->SetStringField(TEXT("EnginePath"), ConfigValue.SkyEnginePath);
		ConfigObject->SetStringField(TEXT("ProjectPath"), ConfigValue.SkyProjectpath);

		FString OutputString;
		TSharedRef<TJsonWriter<>> Writer = TJsonWriterFactory<>::Create(&OutputString);
		FJsonSerializer::Serialize(ConfigObject.ToSharedRef(), Writer);

		FFileHelper::SaveStringToFile(OutputString, *ConfigFilePath);
	}
} // namespace sky

#undef LOCTEXT_NAMESPACE
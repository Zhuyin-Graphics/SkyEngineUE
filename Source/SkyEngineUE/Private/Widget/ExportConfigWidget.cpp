#include "ExportConfigWidget.h"
#include "SkyEngineUEExport.h"
#include "SkyEngineUE.h"
#include "Widgets/Text/STextBlock.h"
#include "Widgets/Layout/SUniformGridPanel.h"

#define LOCTEXT_NAMESPACE "ConfigWindow"

namespace sky
{
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
		FSkyEngineUEModule::UpdateSkyEvn(ConfigValue);
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
} // namespace sky
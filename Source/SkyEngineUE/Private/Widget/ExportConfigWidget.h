#pragma once

#include "CoreMinimal.h"
#include "Widgets/SCompoundWidget.h"
#include "SkyEngineUEExport.h"

namespace sky
{
	class FExportConfigWidget final : public SCompoundWidget
	{
	public:
		FExportConfigWidget();
		~FExportConfigWidget();

		SLATE_BEGIN_ARGS(FExportConfigWidget) {}
			SLATE_ARGUMENT(TSharedPtr<SWindow>, ParentWindow)
		SLATE_END_ARGS()

		void Construct(const FArguments& InArgs);
		FReply OnCloseClicked();
		FReply OnExportFullClicked();
		FReply OnEvnInitClicked();

		void LoadConfig();
		void SaveConfig() const;
	private:
		TWeakPtr<SWindow> ParentWindow;
		FSkyEngineExportConfig ConfigValue;
	};
	
} // namespace sky

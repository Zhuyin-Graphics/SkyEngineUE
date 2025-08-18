#include "SkyEngineUEStyle.h"
#include "Framework/Application/SlateApplication.h"
#include "Styling/SlateStyleRegistry.h"
#include "Interfaces/IPluginManager.h"
#include "Styling/SlateStyleMacros.h"

#define RootToContentDir Style->RootToContentDir

TSharedPtr<FSlateStyleSet> FSkyEngineUEStyle::StyleInstance = nullptr;

void FSkyEngineUEStyle::Initialize()
{
	if (!StyleInstance.IsValid())
	{
		StyleInstance = Create();
		FSlateStyleRegistry::RegisterSlateStyle(*StyleInstance);
	}
}

void FSkyEngineUEStyle::Shutdown()
{
	FSlateStyleRegistry::UnRegisterSlateStyle(*StyleInstance);
	ensure(StyleInstance.IsUnique());
	StyleInstance.Reset();
}

FName FSkyEngineUEStyle::GetStyleSetName()
{
	static FName StyleSetName(TEXT("SkyEngineUEStyle"));
	return StyleSetName;
}

TSharedRef<FSlateStyleSet> FSkyEngineUEStyle::Create()
{
	static const FVector2D Icon16X16(16.0f, 16.0f);
	static const FVector2D Icon20X20(20.0f, 20.0f);
	
	TSharedRef<FSlateStyleSet> Style = MakeShareable(new FSlateStyleSet("SkyEngineUEStyle"));
	Style->SetContentRoot(IPluginManager::Get().FindPlugin("SkyEngineUE")->GetBaseDir() / TEXT("Resources"));
	Style->Set("SkyEngineUE.PluginAction", new IMAGE_BRUSH_SVG(TEXT("ExportPanel"), Icon20X20));
	return Style;
}

void FSkyEngineUEStyle::ReloadTextures()
{
	if (FSlateApplication::IsInitialized())
	{
		FSlateApplication::Get().GetRenderer()->ReloadTextureResources();
	}
}

const ISlateStyle& FSkyEngineUEStyle::Get()
{
	return *StyleInstance;
}
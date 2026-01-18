
#include "Exporter/TextureExporter.h"
#include "SkyEngineUEExport.h"

#include <Engine/Texture.h>
#include <TextureCompiler.h>
#include <IImageWrapper.h>
#include <IImageWrapperModule.h>

#include <render/adaptor/assets/ImageAsset.h>
#include <framework/asset/AssetDataBase.h>

namespace sky {

	static void FullyLoad(const UTexture* InTexture)
	{
		UTexture* Texture = const_cast<UTexture*>(InTexture);
		FTextureCompilingManager::Get().FinishCompilation({ Texture });
		Texture->SetForceMipLevelsToBeResident(30.0f);
		Texture->WaitForStreaming();
	}

	void TextureExport::Init()
	{
		std::string TexName = TCHAR_TO_UTF8(*mPayload.Texture->GetName());

		mPath.bundle = SourceAssetBundle::WORKSPACE;
		mPath.path = FilePath("Textures") / FilePath(TexName);
		mPath.path.ReplaceExtension(".png");
	}

	void TextureExport::Run()
	{

		FTextureSource& Source = mPayload.Texture->Source;
		TArray64<uint8> RawData;

		IImageWrapperModule& ImageWrapperModule = FModuleManager::LoadModuleChecked<IImageWrapperModule>(FName("ImageWrapper"));

		Source.GetMipData(RawData, 0, &ImageWrapperModule);

		EImageFormat Format = EImageFormat::PNG;
		TSharedPtr<IImageWrapper> ImageWrapper = ImageWrapperModule.CreateImageWrapper(Format);

		const int32 Width = Source.GetSizeX();
		const int32 Height = Source.GetSizeY();
		const ERGBFormat RGBFormat = Source.GetFormat() == TSF_BGRA8 ?
			ERGBFormat::BGRA : ERGBFormat::RGBA;

		if (ImageWrapper->SetRaw(RawData.GetData(), RawData.Num(),
			Width, Height, RGBFormat, 8))
		{
			const TArray64<uint8>& CompressedData = ImageWrapper->GetCompressed();

			auto WorkFs = AssetDataBase::Get()->GetWorkSpaceFs();
			auto OutPath = WorkFs->GetPath() / mPath.path;

			FFileHelper::SaveArrayToFile(CompressedData, UTF8_TO_TCHAR(OutPath.GetStr().c_str()));
		}

		auto MatSource = AssetDataBase::Get()->RegisterAsset(mPath, false);
		MatSource->category = AssetTraits<Texture>::ASSET_TYPE;


		UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Texture %s"), *mPayload.Texture->GetFullName());
	}

} // namespace sky
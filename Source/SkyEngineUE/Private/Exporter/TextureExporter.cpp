
#include "Exporter/TextureExporter.h"

namespace sky {

	void TextureExport::Init()
	{
		std::string TexName = TCHAR_TO_UTF8(*mPayload.Texture->GetName());

		mPath.bundle = SourceAssetBundle::WORKSPACE;
		mPath.path = FilePath("Textures") / FilePath(TexName);
		mPath.path.ReplaceExtension(".image");
	}

	void TextureExport::Run()
	{
	}

} // namespace sky
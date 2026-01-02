
#include "SkyEngineContext.h"
#include "Framework/asset/AssetManager.h"
#include "framework/asset/AssetDataBase.h"

DEFINE_LOG_CATEGORY_STATIC(LogSkyEngineExporter, Log, All);

namespace sky {

	SkyEngineContext::SkyEngineContext(const std::string& engine, const std::string& proj)
	{
		workFs = new NativeFileSystem(proj);
		engineFs = new NativeFileSystem(engine);

		AssetManager::Get()->SetWorkFileSystem(workFs);
		AssetDataBase::Get()->SetEngineFs(engineFs);
		AssetDataBase::Get()->SetWorkSpaceFs(workFs);
		AssetDataBase::Get()->Load();
	}

	SkyEngineContext::~SkyEngineContext()
	{
		auto& sources = AssetDataBase::Get()->GetSources();
		for (auto& [uuid, source] : sources) {

			std::string path = source->path.path.GetStr();
			UE_LOG(LogSkyEngineExporter, Log, TEXT("export db path %hs"), path.c_str());
		}

		AssetDataBase::Get()->Save();
	}

} // namespace sky
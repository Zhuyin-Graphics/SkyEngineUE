
#include "SkyEngineContext.h"
#include "SkyEngineUEExport.h"
#include "Framework/asset/AssetManager.h"
#include "framework/asset/AssetDataBase.h"
#include "framework/serialization/SerializationContext.h"
#include "framework/asset/AssetBuilderManager.h"

namespace sky {

	SkyEngineContext::SkyEngineContext(const std::string& engine, const std::string& proj)
	{
		workFs = new NativeFileSystem(proj);
		engineFs = new NativeFileSystem(engine);

		auto* am = AssetBuilderManager::Get();

		AssetManager::Get()->SetWorkFileSystem(workFs);
		AssetDataBase::Get()->SetEngineFs(engineFs);
		AssetDataBase::Get()->SetWorkSpaceFs(workFs);
		AssetDataBase::Get()->Load();

		SerializationContext::Get();
	}

	SkyEngineContext::~SkyEngineContext()
	{
		auto& sources = AssetDataBase::Get()->GetSources();
		for (auto& [uuid, source] : sources) {

			std::string path = source->path.path.GetStr();
			UE_LOG(LogSkyEngineExporter, Log, TEXT("export db path %hs"), path.c_str());
		}

		Save();
	}

	void SkyEngineContext::Save()
	{
		AssetDataBase::Get()->Save();
	}

} // namespace sky
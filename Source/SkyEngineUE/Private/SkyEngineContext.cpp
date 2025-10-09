
#include "SkyEngineContext.h"
#include "Framework/asset/AssetManager.h"
#include "framework/asset/AssetDataBase.h"

namespace sky {

	SkyEngineContext::SkyEngineContext(const std::string& engine, const std::string& proj)
	{
		workFs = new NativeFileSystem(proj);
		engineFs = new NativeFileSystem(engine);

		AssetManager::Get()->SetWorkFileSystem(workFs);
		AssetDataBase::Get()->SetEngineFs(engineFs);
		AssetDataBase::Get()->SetWorkSpaceFs(workFs);
	}

} // namespace sky
#pragma once

#include "SkyEngineUEExport.h"
#include <core/util/Uuid.h>
#include <vector>

namespace sky {


	class LevelExport {
	public:
		LevelExport() = default;
		~LevelExport() = default;

		void Run(const FSkyEngineExportConfig& Config);

	private:
		void ExportWorldPartition(const FSkyEngineExportConfig& Config, UWorld* World);

		void Gather(UWorld* World, struct SkyEngineExportContext& Context, struct RenderPrefabAssetData& Data, std::vector<Uuid>& Deps);
	};

} // namespace sky
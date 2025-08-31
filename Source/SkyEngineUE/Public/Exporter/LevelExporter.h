#pragma once

#include "SkyEngineUEExport.h"

namespace sky {

	class LevelExport {
	public:
		LevelExport() = default;
		~LevelExport() = default;

		void Run(const FSkyEngineExportConfig& Config);

	private:
		void ExportWorldPartition(const FSkyEngineExportConfig& Config, UWorld* World);

		void GatherStaticMesh(UWorld* World);
	};

} // namespace sky
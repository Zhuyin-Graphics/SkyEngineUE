#pragma once

#include "SkyEngineUEExport.h"

namespace sky {


	class WorldPartitionExport {
	public:
		WorldPartitionExport() = default;
		~WorldPartitionExport() = default;

		void Run(const FSkyEngineExportConfig& config);
	};


} // namespace sky
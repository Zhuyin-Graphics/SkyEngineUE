#pragma once

#include <string>
#include "core/file/FileSystem.h"
#include "core/util/Uuid.h"
#include "Exporter/ExporterBase.h"

namespace sky {

	class SkyEngineContext {
	public:
		SkyEngineContext(const std::string& enginePath, const std::string& projPath);
		~SkyEngineContext();

		void Save();

	private:
		NativeFileSystemPtr workFs;
		NativeFileSystemPtr engineFs;
	};

	struct SkyEngineExportContext {
		TMap<FGuid, std::shared_ptr<ExporterBase>> Tasks;
	};

} // namespace sky
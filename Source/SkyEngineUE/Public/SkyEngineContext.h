#pragma once

#include <string>
#include "core/file/FileSystem.h"

namespace sky {

	class SkyEngineContext {
	public:
		SkyEngineContext(const std::string& enginePath, const std::string& projPath);
		~SkyEngineContext();

	private:
		NativeFileSystemPtr workFs;
		NativeFileSystemPtr engineFs;
	};

} // namespace sky
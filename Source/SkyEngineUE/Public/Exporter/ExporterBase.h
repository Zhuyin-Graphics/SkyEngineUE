#pragma once

#include "framework/asset/AssetCommon.h"

namespace sky {

	struct SkyEngineExportContext;

	class ExporterBase {
	public:
		ExporterBase() = default;
		virtual ~ExporterBase() {}

		virtual void Init() = 0;
		virtual void Run() = 0;
		Uuid GetUuid() const;

	protected:
		AssetSourcePath mPath;
	};

} // namespace sky
#include "Exporter/ExporterBase.h"
#include "Framework/asset/AssetDataBase.h"

namespace sky {

	Uuid ExporterBase::GetUuid() const
	{
		return AssetDataBase::Get()->CalculateUuidByPath(mPath);
	}

} // namespace sky
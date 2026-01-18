#pragma once

#include <CoreUObjectClasses.h>
#include "core/util/Uuid.h"
#include "Exporter/ExporterBase.h"
#include <render/adaptor/assets/MaterialAsset.h>

namespace sky {

	class MaterialExporter : public ExporterBase {
	public:
		struct Payload {
			TObjectPtr<class UMaterialInterface> Material;
			TMap<FName, Uuid> Textures;
		};

		explicit MaterialExporter(const Payload& Payload);
		~MaterialExporter() {}

		static bool Gather(UMaterialInterface* Material, struct SkyEngineExportContext& Context, Payload& Payload);

		void Init() override;
		void Run() override;

	private:
		bool ProcessParameters();
		bool ProcessBaseMaterialInfo();

		Payload mPayload;

		MaterialInstanceData mData;
	};

} // namespace sky
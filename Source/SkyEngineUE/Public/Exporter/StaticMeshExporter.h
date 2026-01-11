#pragma once
#include <CoreUObjectClasses.h>
#include "Exporter/ExporterBase.h"
#include "core/util/Uuid.h"
#include <vector>

namespace sky {

	class StaticMeshExport : public ExporterBase {
	public:
		struct Payload {
			TObjectPtr<UStaticMesh> StaticMesh;
			std::vector<Uuid> Materials;
		};

		explicit StaticMeshExport(const Payload& Payload) : mPayload(Payload) {}
		~StaticMeshExport() {}

		static bool Gather(UStaticMesh* mesh, struct SkyEngineExportContext& context, Payload& deps);

		void Init() override;
		void Run() override;

	private:
		Payload mPayload;
	};

} // namespace sky
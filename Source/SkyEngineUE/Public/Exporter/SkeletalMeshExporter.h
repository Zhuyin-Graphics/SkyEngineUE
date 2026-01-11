#pragma once
#include <CoreUObjectClasses.h>
#include "Exporter/ExporterBase.h"
#include "core/util/Uuid.h"
#include <vector>

namespace sky {

	class SkeletalMeshExport : public ExporterBase {
	public:
		struct Payload {
			TObjectPtr<USkeletalMesh> Mesh;
			Uuid Skeleton;
			std::vector<Uuid> Materials;
		};

		explicit SkeletalMeshExport(const Payload& Payload) : mPayload(Payload) {}
		~SkeletalMeshExport() {}

		static bool Gather(USkeletalMesh* mesh, struct SkyEngineExportContext& context, Payload& deps);

		void Init() override;
		void Run() override;

	private:
		Payload mPayload;
	};

} // namespace sky
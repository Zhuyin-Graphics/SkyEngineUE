#pragma once
#include <CoreUObjectClasses.h>
#include "core/util/Uuid.h"
#include <vector>

namespace sky {

	class SkeletonMeshExport {
	public:
		struct Payload {
			TObjectPtr<UStaticMesh> StaticMesh;
			std::vector<Uuid> Materials;
		};

		explicit SkeletonMeshExport(const Payload& Payload) : mPayload(Payload) {}
		~SkeletonMeshExport() {}

		void Run();

	private:
		Payload mPayload;
	};

} // namespace sky
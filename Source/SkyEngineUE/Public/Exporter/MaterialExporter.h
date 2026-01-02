#pragma once

#include <CoreUObjectClasses.h>
#include "core/util/Uuid.h"

namespace sky {

	class MaterialExporter {
	public:
		struct Payload {
			TObjectPtr<class UMaterialInterface> Material;
			std::vector<Uuid> Textures;
		};

		explicit MaterialExporter(const Payload& Payload);
		~MaterialExporter() {}

		const Uuid& GetGuid() const { return mGuid; }

		void Run();

	private:
		bool ProcessParameters();
		bool ProcessBaseMaterialInfo();

		Uuid mGuid;
		Payload mPayload;
	};

} // namespace sky
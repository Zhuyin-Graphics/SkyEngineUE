#pragma once

#include <CoreUObjectClasses.h>
#include "Exporter/ExporterBase.h"
#include "core/util/Uuid.h"
#include <vector>

namespace sky {

	class TextureExport : public ExporterBase {
	public:
		struct Payload {
			TObjectPtr<UTexture> Texture;
		};

		explicit TextureExport(const Payload& Payload) : mPayload(Payload) {}
		~TextureExport() {}

		void Init() override;
		void Run() override;

	private:
		Payload mPayload;
	};

} // namespace sky
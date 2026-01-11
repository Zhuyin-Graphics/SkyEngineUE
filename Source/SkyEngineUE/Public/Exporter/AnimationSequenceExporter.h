#pragma once
#include <CoreUObjectClasses.h>
#include "core/util/Uuid.h"
#include "Exporter/ExporterBase.h"
#include <vector>

namespace sky {

	class AnimationSequenceExport : public ExporterBase {
	public:
		struct Payload {
			TObjectPtr<UAnimSequence> Sequence;
			Uuid Skeleton;
		};

		explicit AnimationSequenceExport(const Payload& Payload) : mPayload(Payload) {}
		~AnimationSequenceExport() override {}

		static bool Gather(UAnimSequence* Sequence, SkyEngineExportContext& context, sky::Uuid& deps);

		void Init() override;
		void Run() override;

	private:
		bool UseDataModel();

		Payload mPayload;
	};

} // namespace sky
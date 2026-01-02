#pragma once
#include <CoreUObjectClasses.h>
#include "core/util/Uuid.h"
#include <vector>

struct FSkyEngineExportContext;

namespace sky {

	class AnimationSequenceExport {
	public:
		struct Payload {
			TObjectPtr<UAnimSequence> Sequence;
		};

		explicit AnimationSequenceExport(const Payload& Payload) : mPayload(Payload) {}
		~AnimationSequenceExport() {}

		static void Gather(UAnimSequence* sequence, FSkyEngineExportContext& context);

		void Run();

	private:
		Payload mPayload;
	};

} // namespace sky
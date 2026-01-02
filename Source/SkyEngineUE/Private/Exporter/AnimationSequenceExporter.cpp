
#include "Exporter/AnimationSequenceExporter.h"
#include "SkyEngineUEExport.h"

namespace sky {

	void AnimationSequenceExport::Gather(UAnimSequence* sequence, FSkyEngineExportContext & context)
	{
		auto *skeleton = sequence->GetSkeleton();

		context.Skeletons.Emplace(skeleton);
		context.Sequences.Emplace(sequence);
	}

	void AnimationSequenceExport::Run()
	{
		printf("test\n");
	}

} // namespace sky
#pragma once
#include <CoreUObjectClasses.h>
#include "Exporter/ExporterBase.h"
#include "core/util/Uuid.h"
#include <vector>

namespace sky {

	class SkeletonExport : public ExporterBase {
	public:
		struct Payload {
			TObjectPtr<USkeleton> Skeleton;
		};

		explicit SkeletonExport(const Payload& Payload) : mPayload(Payload) {}
		~SkeletonExport() {}

		void Init() override;
		void Run() override;

	private:
		Payload mPayload;
	};

} // namespace sky
#pragma once
#include <CoreUObjectClasses.h>
#include "core/util/Uuid.h"
#include <vector>

namespace sky {

	class SkeletonExport {
	public:
		struct Payload {
			TObjectPtr<USkeleton> Skeleton;
		};

		explicit SkeletonExport(const Payload& Payload) : mPayload(Payload) {}
		~SkeletonExport() {}

		void Run();

	private:
		Payload mPayload;
	};

} // namespace sky
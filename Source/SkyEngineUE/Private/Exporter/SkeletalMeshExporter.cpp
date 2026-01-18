#include "Exporter/SkeletalMeshExporter.h"
#include "Exporter/SkeletonExporter.h"
#include "Exporter/MaterialExporter.h"
#include "SkyEngineContext.h"

#include <Engine/SkinnedAssetCommon.h>
#include <Rendering/SkeletalMeshRenderData.h>

namespace sky {

	bool SkeletalMeshExport::Gather(USkeletalMesh* SkeletalMesh, SkyEngineExportContext& context, Payload& OutPayload)
	{
		if (context.Tasks.Find(SkeletalMesh->GetOutermost()->GetPersistentGuid()) != nullptr) {
			return false;
		}

		auto Skeleton = SkeletalMesh->GetSkeleton();
		auto SkeletonGuid = Skeleton->GetOutermost()->GetPersistentGuid();

		if (context.Tasks.Find(SkeletonGuid) == nullptr) {
			auto* Task = new SkeletonExport(SkeletonExport::Payload{ Skeleton });
			Task->Init();
			context.Tasks.Emplace(SkeletonGuid, Task);
		}
		{
			auto* Task = context.Tasks.Find(SkeletonGuid);
			OutPayload.Skeleton = (*Task)->GetUuid();
		}


		const TArray<FSkeletalMaterial>& MaterialSlots = SkeletalMesh->GetMaterials();
		for (auto& Material : MaterialSlots)
		{
			auto* UMat = Material.MaterialInterface->GetMaterial();
			auto MatId = UMat->GetOutermost()->GetPersistentGuid();

			MaterialExporter::Payload Payload = {};
			Payload.Material = UMat;

			if (MaterialExporter::Gather(UMat, context, Payload)) {

				auto* Task = new sky::MaterialExporter(Payload);
				Task->Init();

				context.Tasks.Emplace(MatId, Task);
			}

			{
				auto* Task = context.Tasks.Find(MatId);
				OutPayload.Materials.emplace_back((*Task)->GetUuid());
			}

		}

		return true;
	}

	void SkeletalMeshExport::Init()
	{
		std::string MeshName = TCHAR_TO_UTF8(*mPayload.Mesh->GetName());

		mPath.bundle = SourceAssetBundle::WORKSPACE;
		mPath.path = FilePath("SkeletalMesh") / FilePath(MeshName);
		mPath.path.ReplaceExtension(".mesh");
	}

	void SkeletalMeshExport::Run()
	{

	}

} // namespace sky
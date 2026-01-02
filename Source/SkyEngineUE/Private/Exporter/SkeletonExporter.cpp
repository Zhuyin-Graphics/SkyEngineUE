#include "Exporter/SkeletonExporter.h"
#include "SkyEngineConvert.h"
#include "render/adaptor/assets/SkeletonAsset.h"
#include <framework/asset/AssetDataBase.h>

namespace sky {

	void SkeletonExport::Run()
	{
		const FReferenceSkeleton& RefSkeleton = mPayload.Skeleton->GetReferenceSkeleton();
		int32 NumBones = RefSkeleton.GetNum();

		SkeletonAssetData skeletonData = {};
		skeletonData.boneData.resize(NumBones);
		skeletonData.refPos.resize(NumBones);

		auto skeletonName = mPayload.Skeleton->GetName();

		for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
		{
			FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
			int32 ParentIndex = RefSkeleton.GetParentIndex(BoneIndex);
			FTransform LocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];

			auto& SkyBone = skeletonData.boneData[BoneIndex];
			SkyBone.name = sky::Name(TCHAR_TO_UTF8(*BoneName.ToString()));
			SkyBone.parentIndex = static_cast<sky::BoneIndex>(ParentIndex);

			auto& SkyPos = skeletonData.refPos[BoneIndex];
			SkyPos.translation = FromUE(LocalTransform.GetLocation());
			SkyPos.rotation = FromUE(LocalTransform.GetRotation());
			SkyPos.scale = FromUE(LocalTransform.GetScale3D());
		}

		AssetSourcePath sourcePath = {};
		sourcePath.bundle = SourceAssetBundle::WORKSPACE;
		sourcePath.path = FilePath("Skeleton") / FilePath(*skeletonName);
		sourcePath.path.ReplaceExtension(".skeleton");

		{
			auto file = AssetDataBase::Get()->CreateOrOpenFile(sourcePath);
			auto archive = file->WriteAsArchive();
			JsonOutputArchive json(*archive);
			skeletonData.SaveJson(json);
		}

		AssetDataBase::Get()->RegisterAsset(sourcePath, false);
	}

} // namespace sky
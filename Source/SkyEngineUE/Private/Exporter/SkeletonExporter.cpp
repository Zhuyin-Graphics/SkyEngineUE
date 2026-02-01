#include "Exporter/SkeletonExporter.h"
#include "SkyEngineConvert.h"
#include "render/adaptor/assets/SkeletonAsset.h"
#include <framework/asset/AssetDataBase.h>

namespace sky {

	void SkeletonExport::Init()
	{
		auto skeletonName = mPayload.Skeleton->GetName();

		mPath.bundle = SourceAssetBundle::WORKSPACE;
		mPath.path = FilePath("Skeleton") / FilePath(*skeletonName);
		mPath.path.ReplaceExtension(".skeleton");
	}

	void SkeletonExport::Run()
	{
		const FReferenceSkeleton& RefSkeleton = mPayload.Skeleton->GetReferenceSkeleton();
		int32 NumBones = RefSkeleton.GetNum();

		SkeletonAssetData skeletonData = {};
		skeletonData.boneData.resize(NumBones);
		skeletonData.refPos.resize(NumBones);

		for (int32 BoneIndex = 0; BoneIndex < NumBones; BoneIndex++)
		{
			FName BoneName = RefSkeleton.GetBoneName(BoneIndex);
			int32 ParentIndex = RefSkeleton.GetParentIndex(BoneIndex);
			const FTransform &LocalTransform = RefSkeleton.GetRefBonePose()[BoneIndex];

			auto& SkyBone = skeletonData.boneData[BoneIndex];
			SkyBone.name = sky::Name(TCHAR_TO_UTF8(*BoneName.ToString()));
			SkyBone.parentIndex = static_cast<sky::BoneIndex>(ParentIndex);

			auto& SkyPos = skeletonData.refPos[BoneIndex];

			const auto& Translation = LocalTransform.GetLocation();
			const auto& Rotation = LocalTransform.GetRotation();
			const auto& Scale = LocalTransform.GetScale3D();

			SkyPos.translation = Vector3(Translation.X, Translation.Z, Translation.Y);
			SkyPos.rotation = Quaternion(Rotation.W, -Rotation.X, -Rotation.Z, -Rotation.Y);
			SkyPos.scale = Vector3(Scale.X, Scale.Z, Scale.Y);
		}

		{
			auto file = AssetDataBase::Get()->CreateOrOpenFile(mPath);
			auto archive = file->WriteAsArchive();
			JsonOutputArchive json(*archive);
			skeletonData.SaveJson(json);
		}

		auto skeletonSource = AssetDataBase::Get()->RegisterAsset(mPath, false);
		skeletonSource->category = AssetTraits<Skeleton>::ASSET_TYPE;
	}

} // namespace sky
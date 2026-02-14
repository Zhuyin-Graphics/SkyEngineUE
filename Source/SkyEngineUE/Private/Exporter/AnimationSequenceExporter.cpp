
#include "Exporter/AnimationSequenceExporter.h"
#include "Exporter/SkeletonExporter.h"

#include "SkyEngineUEExport.h"
#include "SkyEngineConvert.h"
#include "SkyEngineContext.h"
#include "render/adaptor/assets/AnimationAsset.h"
#include <framework/asset/AssetDataBase.h>

namespace sky {

	bool AnimationSequenceExport::UseDataModel()
	{
        if (mPayload.Sequence->HasAnyFlags(RF_NeedLoad | RF_NeedPostLoad))
        {
            mPayload.Sequence->ConditionalPostLoad();
        }

        const IAnimationDataModel* DataModel = mPayload.Sequence->GetDataModel();

		return DataModel != nullptr && DataModel->GetNumberOfFrames() > 0;
	}

	bool AnimationSequenceExport::Gather(UAnimSequence* Sequence, SkyEngineExportContext& context, sky::Uuid &outDep)
	{
		if (context.Tasks.Find(FSoftObjectPath(Sequence).ToString()) != nullptr) {
			return false;
		}

		auto* Skeleton = Sequence->GetSkeleton();
		auto SkeletonGuid = FSoftObjectPath(Skeleton).ToString();
		if (context.Tasks.Find(SkeletonGuid) == nullptr) {
			auto* Task = new SkeletonExport(SkeletonExport::Payload{ Skeleton });
			Task->Init();

			context.Tasks.Emplace(SkeletonGuid, Task);
		}

		auto* Task = context.Tasks.Find(SkeletonGuid);
		outDep = (*Task)->GetUuid();

		return true;
	}

	void AnimationSequenceExport::Init()
	{
		std::string SequenceName = TCHAR_TO_UTF8(*mPayload.Sequence->GetName());

		mPath.bundle = SourceAssetBundle::WORKSPACE;
		mPath.path = FilePath("Animation") / FilePath(SequenceName);
		mPath.path.ReplaceExtension(".clip");
	}

	void AnimationSequenceExport::Run()
	{
		if (UseDataModel()) {

			auto DataModel = mPayload.Sequence->GetDataModel();
			float SequenceLength = DataModel->GetPlayLength();
			int32 NumFrames = DataModel->GetNumberOfFrames();
			int32 NumTracks = DataModel->GetNumBoneTracks();
			auto FrameRate = DataModel->GetFrameRate();

			TArray<FName> BoneTrackNames;
			DataModel->GetBoneTrackNames(BoneTrackNames);

			AnimationClipAssetData ClipData = {};
			ClipData.name = TCHAR_TO_UTF8(*mPayload.Sequence->GetName());
			ClipData.frameRate = FrameRate.AsDecimal();
			ClipData.skeleton = mPayload.Skeleton;
			ClipData.nodeChannels.resize(NumTracks);

			for (int32 Index = 0; Index < NumTracks; ++Index) {
				auto& Channel = ClipData.nodeChannels[Index];

				const auto& BoneTrackName = BoneTrackNames[Index];
				Channel.name = TCHAR_TO_UTF8(*BoneTrackName.ToString());
				Channel.position.Resize(NumFrames);
				Channel.scale.Resize(NumFrames);
				Channel.rotation.Resize(NumFrames);

				for (int32 Frame = 0; Frame < NumFrames; ++Frame) {

					FTransform Trans = DataModel->GetBoneTrackTransform(BoneTrackName, Frame);

					Channel.position.times[Frame] = Frame;
					Channel.scale.times[Frame] = Frame;
					Channel.rotation.times[Frame] = Frame;

					const auto& Translation = Trans.GetLocation();
					const auto& Rotation = Trans.GetRotation();
					const auto& Scale = Trans.GetScale3D();

					Channel.position.keys[Frame] = Vector3(Translation.X, Translation.Z, Translation.Y);
					Channel.rotation.keys[Frame] = Quaternion(Rotation.W, -Rotation.X, -Rotation.Z, -Rotation.Y);
					Channel.scale.keys[Frame] = Vector3(Scale.X, Scale.Z, Scale.Y);
				}
			}

			{
				auto file = AssetDataBase::Get()->CreateOrOpenFile(mPath);
				auto archive = file->WriteAsArchive();
				BinaryOutputArchive bin(*archive);
				ClipData.Save(bin);
			}

			auto ClipSource = AssetDataBase::Get()->RegisterAsset(mPath, false);
			ClipSource->category = AssetTraits<AnimationClip>::ASSET_TYPE;

		}
	}

} // namespace sky
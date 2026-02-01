#include "Exporter/SkeletalMeshExporter.h"
#include "Exporter/SkeletonExporter.h"
#include "Exporter/MaterialExporter.h"
#include "SkyEngineContext.h"
#include "SkyEngineConvert.h"

#include <framework/asset/AssetDataBase.h>
#include <render/adaptor/assets/MeshAsset.h>
#include <render/resource/SkeletonMesh.h>
#include <Engine/SkinnedAssetCommon.h>
#include <Rendering/SkeletalMeshRenderData.h>

namespace sky {

	bool SkeletalMeshExport::Gather(USkeletalMesh* SkeletalMesh, SkyEngineExportContext& context, Payload& OutPayload)
	{
		if (context.Tasks.Find(FSoftObjectPath(SkeletalMesh).ToString()) != nullptr) {
			return false;
		}

		auto Skeleton = SkeletalMesh->GetSkeleton();
		auto SkeletonGuid = FSoftObjectPath(Skeleton).ToString();

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
			auto MatId = FSoftObjectPath(UMat).ToString();

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
		auto& SkeletalMesh = mPayload.Mesh;

		auto* RenderData = SkeletalMesh->GetResourceForRendering();
		auto NumLods = RenderData->LODRenderData.Num();

		if (NumLods == 0)
		{
			return;
		}

		FSkeletalMeshLODRenderData& Resource = RenderData->LODRenderData[0];

		uint32_t NumTexCoord = Resource.GetNumTexCoords();
		uint32_t NumVertices = Resource.GetNumVertices();
		uint32_t NumIndices = Resource.GetTotalFaces() * 3;

		FSkinWeightVertexBuffer& SkinWeightBuffer = Resource.SkinWeightVertexBuffer;
		FPositionVertexBuffer& PositionBuffer = Resource.StaticVertexBuffers.PositionVertexBuffer;
		FStaticMeshVertexBuffer& StaticVertexBuffer = Resource.StaticVertexBuffers.StaticMeshVertexBuffer;

		auto *IndexBuffer = Resource.MultiSizeIndexContainer.GetIndexBuffer();
		rhi::IndexType IndexType = Resource.MultiSizeIndexContainer.GetDataTypeSize() != sizeof(uint16) ? rhi::IndexType::U32 : rhi::IndexType::U16;

		CounterPtr<sky::SkeletalMeshGeometry> OutMesh = new sky::SkeletalMeshGeometry();
		sky::StaticMeshGeometry::Config config = {};
		config.UVNum = 1; // TODO

		int32 BoneInfluenceCount = SkinWeightBuffer.GetMaxBoneInfluences();
		OutMesh->Init(NumVertices, NumIndices, IndexType, config);
		for (uint32 j = 0; j < NumVertices; j++)
		{
			VertexBoneData BoneData = {};

			for (int32 InfluenceIndex = 0; InfluenceIndex < BoneInfluenceCount && InfluenceIndex < 4; InfluenceIndex++)
			{
				auto Index = SkinWeightBuffer.GetBoneIndex(j, InfluenceIndex);
				auto Weight = SkinWeightBuffer.GetBoneWeight(j, InfluenceIndex);

				BoneData.boneId[InfluenceIndex] = static_cast<uint8>(Index);
				BoneData.weight[InfluenceIndex] = static_cast<uint8>(Weight);
			}

			const FVector3f& Position = PositionBuffer.VertexPosition(j);
			FVector2f UV0 = StaticVertexBuffer.GetVertexUV(j, 0);
			FVector4f Normal = StaticVertexBuffer.VertexTangentZ(j);
			FVector4f Tangent = StaticVertexBuffer.VertexTangentX(j);

			OutMesh->SetPosition(j, FromUE(Position));
			OutMesh->SetTangent(j, Vector3(Normal.X, Normal.Y, Normal.Z), FromUE(Tangent));
			OutMesh->SetUv0(j, FromUE(UV0));
			OutMesh->SetBoneIndexAndWeight(j, BoneData);
		}

		uint8_t* RawIndexPtr = OutMesh->GetIndexBuffer()->GetDataPointer();
		if (IndexType == rhi::IndexType::U16)
		{
			Convert<uint16_t>(RawIndexPtr, NumIndices, (const uint16*)IndexBuffer->GetPointerTo(0));
		}
		else
		{
			Convert<uint32_t>(RawIndexPtr, NumIndices, (const uint32_t*)IndexBuffer->GetPointerTo(0));
		}

		auto Box = SkeletalMesh->GetImportedBounds().GetBox();
		auto Center = Box.GetCenter();
		auto Ext = Box.GetExtent();

		std::vector<Uuid> UsedGuid;

		sky::MeshSubSection subMesh = {};

		for (int32 SectionIndex = 0; SectionIndex < Resource.RenderSections.Num(); SectionIndex++)
		{
			FSkelMeshRenderSection& Section = Resource.RenderSections[SectionIndex];

			subMesh.firstVertex = 0;// Section.BaseVertexIndex;
			subMesh.vertexCount = Section.NumVertices;
			subMesh.firstIndex = Section.BaseIndex;
			subMesh.indexCount = Section.NumTriangles * 3;
			subMesh.materialIndex = static_cast<uint32_t>(UsedGuid.size());
			subMesh.aabb.min = FromUE(Center - Ext);
			subMesh.aabb.max = FromUE(Center + Ext);

			UsedGuid.emplace_back(mPayload.Materials[Section.MaterialIndex]);
			OutMesh->AddSubMesh(subMesh);
		}


		sky::StaticMeshAsset staticMeshAsset;
		staticMeshAsset.geometry = OutMesh;
		staticMeshAsset.skeletalGeometry = OutMesh;
		staticMeshAsset.materials.swap(UsedGuid);

		sky::MeshAssetData meshData = staticMeshAsset.MakeMeshAssetData();
		meshData.skeleton = mPayload.Skeleton;

		{
			auto file = AssetDataBase::Get()->CreateOrOpenFile(mPath);
			auto archive = file->WriteAsArchive();
			BinaryOutputArchive bin(*archive);
			meshData.Save(bin);
		}

		auto Source = AssetDataBase::Get()->RegisterAsset(mPath, false);
		Source->category = AssetTraits<Mesh>::ASSET_TYPE;
		Source->dependencies = staticMeshAsset.materials;

	}

} // namespace sky
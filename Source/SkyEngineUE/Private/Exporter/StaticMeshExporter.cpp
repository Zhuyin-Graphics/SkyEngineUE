#include "Exporter/StaticMeshExporter.h"
#include "Exporter/MaterialExporter.h"
#include "SkyEngineConvert.h"
#include "SkyEngineContext.h"
#include <render/resource/StaticMesh.h>
#include <render/adaptor/assets/MeshAsset.h>
#include <framework/asset/AssetDataBase.h>

namespace sky {


	bool StaticMeshExport::Gather(UStaticMesh* StaticMesh, SkyEngineExportContext& Context, Payload& OutPayload)
	{
		if (Context.Tasks.Find(FSoftObjectPath(StaticMesh).ToString()) != nullptr) {
			return false;
		}


		TArray<FStaticMaterial>& Materials = StaticMesh->GetStaticMaterials();
		for (auto& Material : Materials)
		{
			auto &UMat = Material.MaterialInterface;
			auto MatId = FSoftObjectPath(UMat).ToString();

			MaterialExporter::Payload Payload = {};
			Payload.Material = UMat;

			if (MaterialExporter::Gather(UMat, Context, Payload)) {

				auto* Task = new sky::MaterialExporter(Payload);
				Task->Init();

				Context.Tasks.Emplace(MatId, Task);
			}

			{
				auto* Task = Context.Tasks.Find(MatId);
				OutPayload.Materials.emplace_back((*Task)->GetUuid());
			}
		}

		return true;
	}

	void StaticMeshExport::Init()
	{
		std::string MeshName = TCHAR_TO_UTF8(*mPayload.StaticMesh->GetName());

		mPath.bundle = SourceAssetBundle::WORKSPACE;
		mPath.path = FilePath("Mesh") / FilePath(MeshName);
		mPath.path.ReplaceExtension(".mesh");
	}

	void StaticMeshExport::Run()
	{
		auto& StaticMesh = mPayload.StaticMesh;
		auto NumLods = StaticMesh->GetNumLODs();

		if (NumLods == 0)
		{
			return;
		}

		auto* RenderData = StaticMesh->GetRenderData();
		auto& Resource = RenderData->LODResources[0];

		// Vertices Num
		uint32_t NumVertices = Resource.VertexBuffers.PositionVertexBuffer.GetNumVertices();
		uint32_t NumTexCoord = Resource.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();
		uint32_t NumIndices  = Resource.IndexBuffer.GetNumIndices();
		rhi::IndexType IndexType = Resource.IndexBuffer.Is32Bit() ? rhi::IndexType::U32 : rhi::IndexType::U16;

		CounterPtr<sky::StaticMeshGeometry> OutMesh = new sky::StaticMeshGeometry();
		sky::StaticMeshGeometry::Config config = {};
		config.UVNum = 1; // TODO

		OutMesh->Init(NumVertices, NumIndices, IndexType, config);
		for (uint32 j = 0; j < NumVertices; ++j)
		{
			const FVector3f& Position = Resource.VertexBuffers.PositionVertexBuffer.VertexPosition(j);

			FVector2f UV0 = Resource.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(j, 0);
			FVector4f Normal = Resource.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(j);
			FVector4f Tangent = Resource.VertexBuffers.StaticMeshVertexBuffer.VertexTangentX(j);

			OutMesh->SetPosition(j, Vector3(Position.X, Position.Z, Position.Y));
			OutMesh->SetTangent(j, Vector3(Normal.X, Normal.Z, Normal.Y), FromUE(Tangent));
			OutMesh->SetUv0(j, Vector2(UV0.X, UV0.Y));
		}

		uint8_t* RawIndexPtr = OutMesh->GetIndexBuffer()->GetDataPointer();
		if (IndexType == rhi::IndexType::U16)
		{
			Convert<uint16_t>(RawIndexPtr, NumIndices, Resource.IndexBuffer.AccessStream16());
			// memcpy(RawIndexPtr, Resource.IndexBuffer.AccessStream16(), NumIndices * sizeof(uint16_t));
		}
		else
		{
			Convert<uint32_t>(RawIndexPtr, NumIndices, Resource.IndexBuffer.AccessStream32());
			// memcpy(RawIndexPtr, Resource.IndexBuffer.AccessStream32(), NumIndices * sizeof(uint32_t));
		}

#if ENGINE_MINOR_VERSION > 5
		auto Box = Resource.SourceMeshBounds.GetBox();
#else
		auto Box = RenderData->Bounds.GetBox();
#endif
		
		auto Center = Box.GetCenter();
		auto Ext = Box.GetExtent();

		std::vector<Uuid> UsedGuid;

		sky::MeshSubSection subMesh = {};
		for (int32 SectionIndex = 0; SectionIndex < Resource.Sections.Num(); SectionIndex++)
		{
			FStaticMeshSection& Section = Resource.Sections[SectionIndex];

			subMesh.firstVertex = Section.MinVertexIndex;
			subMesh.vertexCount = Section.MaxVertexIndex;
			subMesh.firstIndex = Section.FirstIndex;
			subMesh.indexCount = Section.NumTriangles * 3;
			subMesh.materialIndex = static_cast<uint32_t>(UsedGuid.size());
			subMesh.aabb.min = FromUE(Center - Ext);
			subMesh.aabb.max = FromUE(Center + Ext);

			UsedGuid.emplace_back(mPayload.Materials[Section.MaterialIndex]);
			OutMesh->AddSubMesh(subMesh);
		}


		sky::StaticMeshAsset staticMeshAsset;
		staticMeshAsset.geometry = OutMesh;
		staticMeshAsset.materials.swap(UsedGuid);

		sky::MeshAssetData meshData = staticMeshAsset.MakeMeshAssetData();

		{
			auto file = AssetDataBase::Get()->CreateOrOpenFile(mPath);
			auto archive = file->WriteAsArchive();
			BinaryOutputArchive bin(*archive);
			meshData.Save(bin);
		}

		auto Source = AssetDataBase::Get()->RegisterAsset(mPath, false);
		Source->category = AssetTraits<Mesh>::ASSET_TYPE;
		Source->dependencies = staticMeshAsset.materials;

#if 0
		auto& StaticMesh = mPayload.StaticMesh;

		// Materials
		TArray<FStaticMaterial>& Materials = StaticMesh->GetStaticMaterials();
		auto NumLods = StaticMesh->GetNumLODs();

		for (int32 i = 0; i < NumLods; ++i)
		{
			auto& Resource = StaticMesh->GetRenderData()->LODResources[i];
			auto name = StaticMesh->GetName();
			FString LodName = FString::Printf(TEXT("%s_LOD%d"), *name, i);

			// Vertices Num
			uint32_t NumVertices = Resource.VertexBuffers.PositionVertexBuffer.GetNumVertices();
			uint32_t NumTexCoord = Resource.VertexBuffers.StaticMeshVertexBuffer.GetNumTexCoords();
			uint32_t NumIndices = Resource.IndexBuffer.GetNumIndices();
			rhi::IndexType IndexType = Resource.IndexBuffer.Is32Bit() ? rhi::IndexType::U32 : rhi::IndexType::U16;

			CounterPtr<sky::StaticMeshGeometry> OutMesh = new sky::StaticMeshGeometry();
			sky::StaticMeshGeometry::Config config = {};
			config.UVNum = 1; // TODO

			OutMesh->Init(NumVertices, NumIndices, IndexType, config);;
			for (uint32 j = 0; j < NumVertices; ++j)
			{
				const FVector3f &Position = Resource.VertexBuffers.PositionVertexBuffer.VertexPosition(j);

				FVector2f UV0 = Resource.VertexBuffers.StaticMeshVertexBuffer.GetVertexUV(j, 0);
				FVector4f Normal = Resource.VertexBuffers.StaticMeshVertexBuffer.VertexTangentX(j);
				FVector4f Tangent = Resource.VertexBuffers.StaticMeshVertexBuffer.VertexTangentZ(j);

				OutMesh->SetPosition(j, FromUE(Position));
				OutMesh->SetTangent(j, Vector3(Normal.X, Normal.Y, Normal.Z), FromUE(Tangent));
				OutMesh->SetUv0(j, FromUE(UV0));
			}

			uint8_t* RawIndexPtr = OutMesh->GetIndexBuffer()->GetDataPointer();
			if (IndexType == rhi::IndexType::U16)
			{
				memcpy(RawIndexPtr, Resource.IndexBuffer.AccessStream16(), NumIndices * sizeof(uint16_t));
			}
			else
			{
				memcpy(RawIndexPtr, Resource.IndexBuffer.AccessStream32(), NumIndices * sizeof(uint32_t));
			}


			sky::MeshSubSection subMesh = {};


			for (int32 SectionIndex = 0; SectionIndex < Resource.Sections.Num(); SectionIndex++)
			{
				FStaticMeshSection& Section = Resource.Sections[SectionIndex];
				auto& Material = Materials[Section.MaterialIndex];

				subMesh.firstVertex = Section.MinVertexIndex;
				subMesh.vertexCount = Section.MaxVertexIndex - Section.MinVertexIndex;
				subMesh.firstIndex = Section.FirstIndex;
				subMesh.indexCount = Section.NumTriangles * 3;
				subMesh.materialIndex = Section.MaterialIndex;

				OutMesh->AddSubMesh(subMesh);
			}


			sky::StaticMeshAsset staticMeshAsset;
			staticMeshAsset.geometry = OutMesh;
			staticMeshAsset.materials = mPayload.Materials;

			sky::MeshAssetData meshData = staticMeshAsset.MakeMeshAssetData();

			AssetSourcePath sourcePath = {};
			sourcePath.bundle = SourceAssetBundle::WORKSPACE;
			sourcePath.path = FilePath("Mesh") / FilePath(*LodName);
			sourcePath.path.ReplaceExtension(".mesh");

			{
				auto file = AssetDataBase::Get()->CreateOrOpenFile(sourcePath);
				auto archive = file->WriteAsArchive();
				BinaryOutputArchive bin(*archive);
				meshData.Save(bin);
			}

			auto source = AssetDataBase::Get()->RegisterAsset(sourcePath);

			auto Box = Resource.SourceMeshBounds.GetBox();
			auto Center = Box.GetCenter();
			auto Ext = Box.GetExtent();

			UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Static Mesh %s, LOD_%d, SubMesh %d, Vertices %u, Indices %u, BoudingBox[(%f, %f, %f), (%f, %f, %f)]"), *name, i, Resource.Sections.Num(),
				NumVertices, NumIndices,
				Center.X, Center.Y, Center.Z, Ext.X, Ext.Y, Ext.Z
			);
		}

		UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Static Mesh %s, LOD %d"), *StaticMesh->GetFName().ToString(), NumLods);
#endif
	}

} // namespace sky
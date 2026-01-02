#include "Exporter/StaticMeshExporter.h"
#include "Exporter/MaterialExporter.h"
#include "SkyEngineConvert.h"
#include <render/resource/StaticMesh.h>
#include <render/adaptor/assets/MeshAsset.h>
#include <framework/asset/AssetDataBase.h>

DEFINE_LOG_CATEGORY_STATIC(LogSkyEngineExporter, Log, All);

namespace sky {

	void StaticMeshExport::Run()
	{
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
	}

} // namespace sky
#include "Exporter/StaticMeshExporter.h"

DEFINE_LOG_CATEGORY_STATIC(LogSkyEngineExporter, Log, All);

namespace sky {

	void StaticMeshExport::Run()
	{
		auto NumLods = StaticMesh->GetNumLODs();
		for (int32 i = 0; i < NumLods; ++i)
		{
			auto& Resource = StaticMesh->GetRenderData()->LODResources[i];

			// Vertices Num
			auto VerticesNum = Resource.VertexBuffers.PositionVertexBuffer.GetNumVertices();
			for (uint32 j = 0; j < VerticesNum; ++j)
			{
				const FVector3f &Position = Resource.VertexBuffers.PositionVertexBuffer.VertexPosition(j);
			}

			auto IndicesNum = Resource.IndexBuffer.GetNumIndices();


			auto Box = Resource.SourceMeshBounds.GetBox();
			auto Center = Box.GetCenter();
			auto Ext = Box.GetExtent();
			UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Static Mesh %s, LOD_%d, SubMesh %d, Vertices %u, Indices %u, BoudingBox[(%f, %f, %f), (%f, %f, %f)]"), *StaticMesh->GetFName().ToString(), i, Resource.Sections.Num(),
				VerticesNum, IndicesNum,
				Center.X, Center.Y, Center.Z, Ext.X, Ext.Y, Ext.Z
			);
		}

		UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Static Mesh %s, LOD %d"), *StaticMesh->GetFName().ToString(), NumLods);
	}

} // namespace sky
#include "Exporter/LevelExporter.h"
#include "Exporter/StaticMeshExporter.h"
#include "Exporter/MaterialExporter.h"

#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogSkyEngineExporter, Log, All);

namespace sky {

	void LevelExport::GatherStaticMesh(UWorld* World)
	{
		TSet<TObjectPtr<UStaticMesh>> UniqueMeshes;
		TSet<TObjectPtr<UMaterialInterface>> UniqueMaterials;
		TSet<AActor*> UniqueActors;
		
		for (TActorIterator<AActor> Iter(World); Iter; ++Iter)
		{
			AActor* Actor = *Iter;

			TArray<UStaticMeshComponent*> MeshComponents;
			Actor->GetComponents<UStaticMeshComponent>(MeshComponents);

			for (UStaticMeshComponent* MeshComponent : MeshComponents)
			{
				if (MeshComponent == nullptr)
				{
					continue;
				}

				auto StaticMesh = MeshComponent->GetStaticMesh();
				if (StaticMesh)
				{
					auto StaticMaterialArray = StaticMesh->GetStaticMaterials();
					for (auto& Mat : StaticMaterialArray)
					{
						UniqueMaterials.Emplace(Mat.MaterialInterface);
					}

					UniqueMeshes.Emplace(StaticMesh);
				}

				auto Position = MeshComponent->GetRelativeLocation();
				UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Actor MeshComponents Component[%f, %f, %f]"),
					Position.X, Position.Y, Position.Z);
			}

			UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Actor MeshComponents %d"), MeshComponents.Num());

			if (MeshComponents.Num() > 0)
			{
				UniqueActors.Emplace(Actor);
			}
		}

		for (auto& StaticMesh : UniqueMeshes)
		{
			StaticMeshExport MeshExport(StaticMesh);
			MeshExport.Run();
		}

		for (auto& Material : UniqueMaterials)
		{
			MaterialExporter MatExport(Material);
			MatExport.Run();
		}
	}

	void LevelExport::ExportWorldPartition(const FSkyEngineExportConfig& Config, UWorld* World)
	{
		UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported %d Actor Count"), World->GetActorCount());


		GatherStaticMesh(World);
	}

	void LevelExport::Run(const FSkyEngineExportConfig& Config)
	{
		auto World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		if (World == nullptr)
		{
			UE_LOG(LogSkyEngineExporter, Error, TEXT("No World Context"));
		}

		UE_LOG(LogSkyEngineExporter, Log, TEXT("Start World Export Tasks %s."), *World->GetName());

		ExportWorldPartition(Config, World);
	}

} // namespace sky
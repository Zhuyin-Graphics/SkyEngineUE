#include "Exporter/WorldPartitionExporter.h"
#include "Engine/World.h"
#include "Editor.h"
#include "EngineUtils.h"

#include "WorldPartition/WorldPartition.h"

DEFINE_LOG_CATEGORY(LogSkyEngineExporter);

namespace sky {

	static const TMap<EMaterialShadingModel, FString> ShadingModelToString = {
	{ MSM_Unlit, TEXT("Unlit") },
	{ MSM_DefaultLit, TEXT("DefaultLit") },
	{ MSM_Subsurface, TEXT("Subsurface") },
	{ MSM_PreintegratedSkin, TEXT("PreintegratedSkin") },
	{ MSM_ClearCoat, TEXT("ClearCoat") },
	{ MSM_SubsurfaceProfile, TEXT("SubsurfaceProfile") },
	{ MSM_TwoSidedFoliage, TEXT("TwoSidedFoliage") },
	{ MSM_Hair, TEXT("Hair") },
	{ MSM_Cloth, TEXT("Cloth") },
	{ MSM_Eye, TEXT("Eye") },
	{ MSM_SingleLayerWater, TEXT("SingleLayerWater") },
	{ MSM_ThinTranslucent, TEXT("ThinTranslucent") }
	};

	static void GatherMaterialInWorld(UWorld* World)
	{

		TSet<UMaterial*> Materials;

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

				int32 TotalMaterials = MeshComponent->GetNumMaterials();
				for (int32 Index = 0; Index < TotalMaterials; Index++)
				{
					UMaterialInterface* MaterialInterface = MeshComponent->GetMaterial(Index);
					if (MaterialInterface)
					{
						auto* Material = MaterialInterface->GetMaterial();
						Materials.Emplace(Material);
					}
				}
			}
		}


		for (auto* Material : Materials)
		{
			auto model = Material->GetShadingModels().GetFirstShadingModel();

			UE_LOG(LogSkyEngineExporter, Log, TEXT("Export Material Package %s, Name %s"), *Material->GetName(), *ShadingModelToString[model]);
		}

	}

	void WorldPartitionExport::Run(const FSkyEngineExportConfig& config)
	{
		auto World = GEditor ? GEditor->GetEditorWorldContext().World() : nullptr;
		if (World == nullptr)
		{
			UE_LOG(LogSkyEngineExporter, Error, TEXT("No World Context"));
		}

		UE_LOG(LogSkyEngineExporter, Log, TEXT("Start World Export Tasks %s."), *World->GetName());

		UWorldPartition* WorldPartition = World->GetWorldPartition();
		if (!WorldPartition)
		{
			UE_LOG(LogSkyEngineExporter, Error, TEXT("World does not have World Partition"));
			return;
		}

		UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported %d Actor Count"), World->GetActorCount());

		GatherMaterialInWorld(World);

#if 0
		int32 ContainerCount = 0;
		WorldPartition->ForEachActorDescContainerInstanceBreakable([&](UActorDescContainerInstance* ActorDescContainerInstance) -> bool
			{
				auto* Container = ActorDescContainerInstance->GetContainer();
				UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported %s Container"), *Container->GetContainerName());

				TArray<UPackage*> PackagesToSave;
				for (FActorDescList::TIterator<> ActorDescIterator(Container); ActorDescIterator; ++ActorDescIterator)
				{
					if (AActor* Actor = FindObject<AActor>(World->PersistentLevel, *ActorDescIterator->GetActorName().ToString()))
					{
						UPackage* Package = Actor->GetExternalPackage();
						UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Actor %s"), *Actor->GetName());
					}
				}
				

				
				++ContainerCount;
				return true;
			});
		UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported %d Containers from World Partition"), ContainerCount);
#endif
	}

} // namespace sky
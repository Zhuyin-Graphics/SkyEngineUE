#include "Exporter/LevelExporter.h"
#include "Exporter/StaticMeshExporter.h"
#include "Exporter/MaterialExporter.h"

#include "SkyEngineContext.h"
#include "SkyEngineUEExport.h"
#include "SkyEngineConvert.h"

#include <framework/asset/AssetDataBase.h>
#include <render/adaptor/assets/RenderPrefab.h>

#include "EngineUtils.h"

namespace sky {

	void LevelExport::Gather(UWorld* World, SkyEngineExportContext& Context, RenderPrefabAssetData& PrefabData, std::vector<Uuid>& Deps)
	{
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
					sky::StaticMeshExport::Payload Payload = {};
					Payload.StaticMesh = StaticMesh;
					if (sky::StaticMeshExport::Gather(StaticMesh, Context, Payload)) {
						auto* Task = new sky::StaticMeshExport(Payload);
						Task->Init();
						Context.Tasks.Emplace(FSoftObjectPath(StaticMesh).ToString(), Task);
					}

					auto &Task = Context.Tasks.FindChecked(FSoftObjectPath(StaticMesh).ToString());
					RenderPrefabNode Data = {};

					Data.name = TCHAR_TO_UTF8(*MeshComponent->GetOwner()->GetName());
					Data.mesh = Task->GetUuid();

					auto iter = std::find(Deps.begin(), Deps.end(), Data.mesh);
					if (iter == Deps.end())
					{
						Deps.emplace_back(Data.mesh);
					}

					auto Transform = MeshComponent->GetComponentTransform();
					Data.localTransform.translation = FromUE(Transform.GetTranslation());
					Data.localTransform.scale = FromUE(Transform.GetScale3D());
					Data.localTransform.rotation = FromUE(Transform.GetRotation());

					PrefabData.nodes.emplace_back(Data);
				}
			}
		}
	}

	struct ExportPacakge {
		std::vector<std::shared_ptr<ExporterBase>> Tasks;
	};

	void LevelExport::ExportWorldPartition(const FSkyEngineExportConfig& Config, UWorld* World)
	{
		UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported %d Actor Count"), World->GetActorCount());

		SkyEngineExportContext Context = {};
		RenderPrefabAssetData Data = {};
		std::vector<Uuid> Deps;
		Gather(World, Context, Data, Deps);

		AssetSourcePath Path = {};
		Path.bundle = SourceAssetBundle::WORKSPACE;
		Path.path = FilePath("Prefab") / FilePath(TCHAR_TO_UTF8(*World->GetName()));
		Path.path.ReplaceExtension(".prefab");

		{
			auto file = AssetDataBase::Get()->CreateOrOpenFile(Path);
			auto archive = file->WriteAsArchive();
			JsonOutputArchive json(*archive);
			Data.SaveJson(json);
		}

		auto Source = AssetDataBase::Get()->RegisterAsset(Path, false);
		Source->category = AssetTraits<RenderPrefab>::ASSET_TYPE;
		Source->dependencies.swap(Deps);

		auto SharedPackage = std::make_shared<ExportPacakge>();
		for (auto& [name, task] : Context.Tasks)
		{
			SharedPackage->Tasks.emplace_back(task);
		}

		for (auto& Task : SharedPackage->Tasks) {
			Task->Run();
		}
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
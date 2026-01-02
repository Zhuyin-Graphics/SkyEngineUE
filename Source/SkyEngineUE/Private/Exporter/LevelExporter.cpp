#include "Exporter/LevelExporter.h"
#include "Exporter/StaticMeshExporter.h"
#include "Exporter/MaterialExporter.h"

#include "Materials/Material.h"
#include "Materials/MaterialInstance.h"

#include "EngineUtils.h"

DEFINE_LOG_CATEGORY_STATIC(LogSkyEngineExporter, Log, All);

namespace sky {

	void GetAllInheritedTextures(UMaterialInstance* MaterialInstance, TArray<UTexture*>& OutTextures)
	{
		if (!MaterialInstance) return;

		TArray<UTexture*> CurrentTextures;
		MaterialInstance->GetUsedTextures(CurrentTextures, EMaterialQualityLevel::High, true, ERHIFeatureLevel::SM5, true);

		for (UTexture* Texture : CurrentTextures)
		{
			if (Texture && !OutTextures.Contains(Texture))
			{
				OutTextures.Add(Texture);
			}
		}
		UMaterialInterface* Parent = MaterialInstance->Parent;
		while (Parent)
		{
			TArray<UTexture*> ParentTextures;
			Parent->GetUsedTextures(ParentTextures, EMaterialQualityLevel::High, true, ERHIFeatureLevel::SM5, true);

			for (UTexture* Texture : ParentTextures)
			{
				if (Texture && !OutTextures.Contains(Texture))
				{
					OutTextures.Add(Texture);
				}
			}

			if (UMaterialInstance* ParentInstance = Cast<UMaterialInstance>(Parent))
			{
				Parent = ParentInstance->Parent;
			}
			else
			{
				break;
			}
		}
	}

	void LevelExport::GatherStaticMesh(UWorld* World)
	{
		TSet<TObjectPtr<UStaticMesh>> UniqueMeshes;
		//TSet<TObjectPtr<UMaterialInterface>> UniqueMaterials;
		TMap<FString, MaterialExporter> UniqueMaterials;
		
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
						FString ObjectPath = Mat.MaterialInterface->GetPathName();
						MaterialExporter::Payload payload = { Mat.MaterialInterface };
						UniqueMaterials.Emplace(ObjectPath, MaterialExporter(payload));

						TArray<UTexture*> Textures;
						Mat.MaterialInterface->GetUsedTextures(Textures, EMaterialQualityLevel::High, true, ERHIFeatureLevel::SM5, true);

						for (const auto& Tex : Textures)
						{
							TObjectPtr<UTexture> obj(Tex);

							Tex->GetPathName();
						}
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
			StaticMeshExport::Payload payload = { StaticMesh };

			auto StaticMaterialArray = StaticMesh->GetStaticMaterials();
			for (const auto& Mat : StaticMaterialArray)
			{
				auto& MatExport = UniqueMaterials[Mat.MaterialInterface->GetPathName()];
				payload.Materials.emplace_back(MatExport.GetGuid());
			}
			StaticMeshExport MeshExport(payload);
			MeshExport.Run();
		}

		for (auto& [Key, Mat] : UniqueMaterials)
		{
			Mat.Run();
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
#include "Exporter/MaterialExporter.h"
#include "Exporter/TextureExporter.h"
#include "SkyEngineContext.h"
#include <framework/asset/AssetDataBase.h>
#include <render/adaptor/assets/MaterialAsset.h>

DEFINE_LOG_CATEGORY_STATIC(LogSkyEngineExporter, Log, All);

namespace sky {

	MaterialExporter::MaterialExporter(const Payload& Payload)
		: mPayload(Payload)
	{
	}

	bool MaterialExporter::ProcessParameters()
	{
		UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(mPayload.Material);
		if (MaterialInstance == nullptr)
		{
			return false;
		}

		TArray<FMaterialParameterInfo> ParameterInfos;
		TArray<FGuid> Guids;

		// mPayload.Material->GetUsedTextures()

		mPayload.Material->GetAllTextureParameterInfo(ParameterInfos, Guids);
		for (int32 ParameterIdx = 0; ParameterIdx < ParameterInfos.Num(); ParameterIdx++)
		{
			const FMaterialParameterInfo& ParameterInfo = ParameterInfos[ParameterIdx];
			FName ParameterName = ParameterInfo.Name;
			UTexture* Value;
			//Query the material instance parameter overridden value
			if (MaterialInstance->GetTextureParameterValue(ParameterInfo, Value) && Value && Value->AssetImportData)
			{
				UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Material Parameter %s, %s"), *ParameterName.ToString(), *Value->AssetImportData.GetFullName());
			}
		}

		mPayload.Material->GetAllVectorParameterInfo(ParameterInfos, Guids);
		//Query all base material vector parameters.
		for (int32 ParameterIdx = 0; ParameterIdx < ParameterInfos.Num(); ParameterIdx++)
		{
			const FMaterialParameterInfo& ParameterInfo = ParameterInfos[ParameterIdx];
			FName ParameterName = ParameterInfo.Name;
			FLinearColor LinearColor;
			//Query the material instance parameter overridden value
			if (MaterialInstance->GetVectorParameterValue(ParameterInfo, LinearColor))
			{
				UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Material Parameter %s, [%f, %f, %f, %f]"), *ParameterName.ToString(), LinearColor.R, LinearColor.G, LinearColor.B, LinearColor.A);
			}
		}

		mPayload.Material->GetAllScalarParameterInfo(ParameterInfos, Guids);
		for (int32 ParameterIdx = 0; ParameterIdx < ParameterInfos.Num(); ParameterIdx++)
		{
			const FMaterialParameterInfo& ParameterInfo = ParameterInfos[ParameterIdx];
			FName ParameterName = ParameterInfo.Name;
			float Value;

			if (MaterialInstance->GetScalarParameterValue(ParameterInfo, Value))
			{
				UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Material Parameter %s, [%f]"), *ParameterName.ToString(), Value);
			}
		}

		return true;
	}

	bool MaterialExporter::ProcessBaseMaterialInfo()
	{
		struct MaterialDataExport
		{
			EBlendMode BlendMode;
			bool bTwoSided;
			bool bMasked;
		};

		MaterialDataExport MaterialData = {};
		auto* BaseMaterial = mPayload.Material->GetBaseMaterial();

		MaterialData.BlendMode = BaseMaterial->GetBlendMode();
		MaterialData.bTwoSided = BaseMaterial->IsTwoSided();
		MaterialData.bMasked = BaseMaterial->IsMasked();

		auto ShadingModel = BaseMaterial->GetShadingModels().GetFirstShadingModel();
		switch (ShadingModel)
		{
		case MSM_Unlit:
			break;
		case MSM_DefaultLit:
			break;
		default:
			UE_LOG(LogSkyEngineExporter, Error, TEXT("Exported Material %s Failed. Invalid ShadingModel %d"), *mPayload.Material->GetName(), ShadingModel);
			return false;
		}

		UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Material %s"), *mPayload.Material->GetName());
		return true;
	};

	bool MaterialExporter::Gather(UMaterialInterface* Material, SkyEngineExportContext& Context, Payload& Payload)
	{
		if (Context.Tasks.Find(Material->GetOutermost()->GetPersistentGuid()) != nullptr) {
			return false;
		}

		TArray<FMaterialParameterInfo> ParameterInfos;
		TArray<FGuid> Guids;
		Material->GetAllTextureParameterInfo(ParameterInfos, Guids);

		for (int32 ParameterIdx = 0; ParameterIdx < ParameterInfos.Num(); ParameterIdx++)
		{
			const FMaterialParameterInfo& ParameterInfo = ParameterInfos[ParameterIdx];
			FName ParameterName = ParameterInfo.Name;
			UTexture* Value;
			//Query the material instance parameter overridden value
			if (Material->GetTextureParameterValue(ParameterInfo, Value) && Value && Value->AssetImportData)
			{
				auto TexId = Value->GetOutermost()->GetPersistentGuid();

				if (Context.Tasks.Find(TexId) == nullptr) {
					auto* Task = new TextureExport(TextureExport::Payload{ Value });
					Task->Init();

					Context.Tasks.Emplace(TexId, Task);
				}

				auto* Task = Context.Tasks.Find(TexId);
				Payload.Textures.Emplace(ParameterName, (*Task)->GetUuid());
			}
		}

		return true;
	}

	void MaterialExporter::Init()
	{
		std::string MaterialName = TCHAR_TO_UTF8(*mPayload.Material->GetName());

		mPath.bundle = SourceAssetBundle::WORKSPACE;
		mPath.path = FilePath("Material") / FilePath(MaterialName);
		mPath.path.ReplaceExtension(".mati");
	}

	void MaterialExporter::Run()
	{
		// ProcessBaseMaterialInfo();
		// ProcessParameters();

		MaterialInstanceData Data = {};

		// default material type
		{
			AssetSourcePath engineMat = {};
			engineMat.bundle = SourceAssetBundle::ENGINE;
			engineMat.path = "materials/standard_pbr.mat";

			auto Mat = AssetDataBase::Get()->RegisterAsset(engineMat, false);
			Data.material = Mat->uuid;
		}

		{
			auto file = AssetDataBase::Get()->CreateOrOpenFile(mPath);
			auto archive = file->WriteAsArchive();
			JsonOutputArchive json(*archive);
			Data.SaveJson(json);
		}

		auto ClipSource = AssetDataBase::Get()->RegisterAsset(mPath, false);
		ClipSource->category = AssetTraits<MaterialInstance>::ASSET_TYPE;

		UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Material %s"), *mPayload.Material->GetFullName());
	}

} // namespace sky
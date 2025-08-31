#include "Exporter/MaterialExporter.h"

DEFINE_LOG_CATEGORY_STATIC(LogSkyEngineExporter, Log, All);

namespace sky {

	MaterialExporter::MaterialExporter(const TObjectPtr<UMaterialInterface>& InMaterial)
		: Material(InMaterial)
		, BaseMaterial(InMaterial->GetBaseMaterial())
	{
	}

	bool MaterialExporter::ProcessParameters()
	{
		UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(Material);
		if (MaterialInstance == nullptr)
		{
			return false;
		}

		TArray<FMaterialParameterInfo> ParameterInfos;
		TArray<FGuid> Guids;

		BaseMaterial->GetAllTextureParameterInfo(ParameterInfos, Guids);
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

		BaseMaterial->GetAllVectorParameterInfo(ParameterInfos, Guids);
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

		BaseMaterial->GetAllScalarParameterInfo(ParameterInfos, Guids);
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
			UE_LOG(LogSkyEngineExporter, Error, TEXT("Exported Material %s Failed. Invalid ShadingModel %d"), *Material->GetName(), ShadingModel);
			return false;
		}

		UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Material %s"), *Material->GetName());
		return true;
	};

	void MaterialExporter::Run()
	{
		ProcessBaseMaterialInfo();
		ProcessParameters();

		UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Material %s"), *Material->GetName());
	}

} // namespace sky
#include "Exporter/MaterialExporter.h"
#include "Exporter/TextureExporter.h"
#include "SkyEngineContext.h"
#include "SkyEngineUEExport.h"
#include "SkyEngineConvert.h"

#include <IMaterialBakingModule.h>
#include <MaterialBakingStructures.h>
#include <MaterialCompiler.h>
#include <Materials/MaterialExpressionCustomOutput.h>
#include <Materials/MaterialExpressionVectorParameter.h>
#include <Materials/MaterialExpressionScalarParameter.h>
#include <Materials/MaterialExpressionConstant.h>
#include <Materials/MaterialExpressionConstant2Vector.h>
#include <Materials/MaterialExpressionConstant3Vector.h>
#include <Materials/MaterialExpressionConstant4Vector.h>
#include <Materials/MaterialExpressionTextureSampleParameter2D.h>
#include <Materials/MaterialExpressionTextureCoordinate.h>

#include <framework/asset/AssetDataBase.h>


namespace sky {

	static EMaterialShadingModel GetShadingModel(const UMaterialInterface* Material)
	{
		const FMaterialShadingModelField Possibilities = Material->GetShadingModels();
		const int32 PossibilitiesCount = Possibilities.CountShadingModels();

		if (PossibilitiesCount == 0)
		{
			return MSM_DefaultLit;
		}

		return Possibilities.GetFirstShadingModel();
	}

	static FLinearColor GetMask(const FExpressionInput& ExpressionInput)
	{
		return FLinearColor(
			static_cast<float>(ExpressionInput.MaskR),
			static_cast<float>(ExpressionInput.MaskG),
			static_cast<float>(ExpressionInput.MaskB),
			static_cast<float>(ExpressionInput.MaskA)
		);
	}

	static uint32 GetMaskComponentCount(const FExpressionInput& ExpressionInput)
	{
		return ExpressionInput.MaskR + ExpressionInput.MaskG + ExpressionInput.MaskB + ExpressionInput.MaskA;
	}

	static UMaterialExpressionCustomOutput* GetCustomOutputByName(const UMaterialInterface* Material, const FString& FunctionName)
	{
		for (const TObjectPtr<UMaterialExpression>& Expression : Material->GetMaterial()->GetExpressions())
		{
			UMaterialExpressionCustomOutput* CustomOutput = Cast<UMaterialExpressionCustomOutput>(Expression);
			if (CustomOutput != nullptr && CustomOutput->GetFunctionName() == FunctionName)
			{
				return CustomOutput;
			}
		}

		return nullptr;
	}

	static const FExpressionInput* GetInputForProperty(const UMaterialInterface* Material, const FMaterialPropertyEx& Property)
	{
		if (Property.IsCustomOutput())
		{
			const FString FunctionName = Property.CustomOutput.ToString();
			const UMaterialExpressionCustomOutput* CustomOutput = GetCustomOutputByName(Material, FunctionName);
			if (CustomOutput == nullptr)
			{
				return nullptr;
			}

			return const_cast<UMaterialExpressionCustomOutput*>(CustomOutput)->GetInput(0);
		}

		UMaterial* UnderlyingMaterial = const_cast<UMaterial*>(Material->GetMaterial());
		return UnderlyingMaterial->GetExpressionInputForProperty(Property.Type);
	}

	template <class InputType>
	static const FMaterialInput<InputType>* GetInputForProperty(const UMaterialInterface* Material, const FMaterialPropertyEx& Property)
	{
		const FExpressionInput* ExpressionInput = GetInputForProperty(Material, Property);
		return static_cast<const FMaterialInput<InputType>*>(ExpressionInput);
	}

	static bool TryGetConstantColor(FLinearColor& OutValue, const FMaterialPropertyEx& Property, const UMaterialInterface* Material)
	{
		const bool bUseMaterialAttributes = Material->GetMaterial()->bUseMaterialAttributes;
		if (bUseMaterialAttributes)
		{
			return false;
		}

		const FMaterialInput<FColor>* MaterialInput = GetInputForProperty<FColor>(Material, Property);
		if (MaterialInput == nullptr)
		{
			return false;
		}

		if (MaterialInput->UseConstant)
		{
			OutValue = { MaterialInput->Constant };
			return true;
		}

		const UMaterialExpression* Expression = MaterialInput->Expression;
		if (Expression == nullptr)
		{
			// [FIXME]
			// OutValue = FLinearColor(FGLTFMaterialUtilities::GetPropertyDefaultValue(Property));
			return true;
		}

		if (const UMaterialExpressionVectorParameter* VectorParameter = ExactCast<UMaterialExpressionVectorParameter>(Expression))
		{
			FLinearColor Value = VectorParameter->DefaultValue;

			const UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(Material);
			if (MaterialInstance != nullptr)
			{
				const FHashedMaterialParameterInfo ParameterInfo(VectorParameter->GetParameterName());
				if (!MaterialInstance->GetVectorParameterValue(ParameterInfo, Value))
				{
					return false;
				}
			}

			const uint32 MaskComponentCount = GetMaskComponentCount(*MaterialInput);

			if (MaskComponentCount > 0)
			{
				const FLinearColor Mask = GetMask(*MaterialInput);

				Value *= Mask;

				if (MaskComponentCount == 1)
				{
					const float ComponentValue = Value.R + Value.G + Value.B + Value.A;
					Value = { ComponentValue, ComponentValue, ComponentValue, ComponentValue };
				}
			}

			OutValue = Value;
			return true;
		}

		if (const UMaterialExpressionScalarParameter* ScalarParameter = ExactCast<UMaterialExpressionScalarParameter>(Expression))
		{
			float Value = ScalarParameter->DefaultValue;

			const UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(Material);
			if (MaterialInstance != nullptr)
			{
				const FHashedMaterialParameterInfo ParameterInfo(ScalarParameter->GetParameterName());
				if (!MaterialInstance->GetScalarParameterValue(ParameterInfo, Value))
				{
					// TODO: how to handle this?
				}
			}

			OutValue = { Value, Value, Value, Value };
			return true;
		}

		if (const UMaterialExpressionConstant4Vector* Constant4Vector = ExactCast<UMaterialExpressionConstant4Vector>(Expression))
		{
			OutValue = Constant4Vector->Constant;
			return true;
		}

		if (const UMaterialExpressionConstant3Vector* Constant3Vector = ExactCast<UMaterialExpressionConstant3Vector>(Expression))
		{
			OutValue = Constant3Vector->Constant;
			return true;
		}

		if (const UMaterialExpressionConstant2Vector* Constant2Vector = ExactCast<UMaterialExpressionConstant2Vector>(Expression))
		{
			OutValue = { Constant2Vector->R, Constant2Vector->G, 0, 0 };
			return true;
		}

		if (const UMaterialExpressionConstant* Constant = ExactCast<UMaterialExpressionConstant>(Expression))
		{
			OutValue = { Constant->R, Constant->R, Constant->R, Constant->R };
			return true;
		}

		return false;
	}

	bool TryGetConstantScalar(float& OutValue, const FMaterialPropertyEx& Property, const UMaterialInterface* Material)
	{
		const bool bUseMaterialAttributes = Material->GetMaterial()->bUseMaterialAttributes;
		if (bUseMaterialAttributes)
		{
			return false;
		}

		const FMaterialInput<float>* MaterialInput = GetInputForProperty<float>(Material, Property);
		if (MaterialInput == nullptr)
		{
			return false;
		}

		if (MaterialInput->UseConstant)
		{
			OutValue = MaterialInput->Constant;
			return true;
		}

		const UMaterialExpression* Expression = MaterialInput->Expression;
		if (Expression == nullptr)
		{
			return true;
		}

		if (const UMaterialExpressionVectorParameter* VectorParameter = ExactCast<UMaterialExpressionVectorParameter>(Expression))
		{
			FLinearColor Value = VectorParameter->DefaultValue;

			const UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(Material);
			if (MaterialInstance != nullptr)
			{
				const FHashedMaterialParameterInfo ParameterInfo(VectorParameter->GetParameterName());
				if (!MaterialInstance->GetVectorParameterValue(ParameterInfo, Value))
				{
					// TODO: how to handle this?
				}
			}

			const uint32 MaskComponentCount = GetMaskComponentCount(*MaterialInput);

			if (MaskComponentCount > 0)
			{
				const FLinearColor Mask = GetMask(*MaterialInput);
				Value *= Mask;
			}

			OutValue = Value.GetMax();
			return true;
		}

		if (const UMaterialExpressionScalarParameter* ScalarParameter = ExactCast<UMaterialExpressionScalarParameter>(Expression))
		{
			float Value = ScalarParameter->DefaultValue;

			const UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(Material);
			if (MaterialInstance != nullptr)
			{
				const FHashedMaterialParameterInfo ParameterInfo(ScalarParameter->GetParameterName());
				if (!MaterialInstance->GetScalarParameterValue(ParameterInfo, Value))
				{
					// TODO: how to handle this?
				}
			}

			OutValue = Value;
			return true;
		}

		if (const UMaterialExpressionConstant4Vector* Constant4Vector = ExactCast<UMaterialExpressionConstant4Vector>(Expression))
		{
			OutValue = Constant4Vector->Constant.R;
			return true;
		}

		if (const UMaterialExpressionConstant3Vector* Constant3Vector = ExactCast<UMaterialExpressionConstant3Vector>(Expression))
		{
			OutValue = Constant3Vector->Constant.R;
			return true;
		}

		if (const UMaterialExpressionConstant2Vector* Constant2Vector = ExactCast<UMaterialExpressionConstant2Vector>(Expression))
		{
			OutValue = Constant2Vector->R;
			return true;
		}

		if (const UMaterialExpressionConstant* Constant = ExactCast<UMaterialExpressionConstant>(Expression))
		{
			OutValue = Constant->R;
			return true;
		}

		return false;
	}


	static void AnalyzeMaterialProperty(const UMaterialInterface* InMaterial, const FMaterialPropertyEx& InProperty, FMaterialAnalysisResult& OutAnalysis)
	{
		if (GetInputForProperty(InMaterial, InProperty) == nullptr)
		{
			OutAnalysis = FMaterialAnalysisResult();
			return;
		}

		UMaterial* BaseMaterial = const_cast<UMaterial*>(InMaterial->GetMaterial());

		// To extend and improve the analysis for glTF's specific use-case, we compile using FGLTFProxyMaterialCompiler
		BaseMaterial->AnalyzeMaterialCompilationInCallback([InProperty, BaseMaterial](FMaterialCompiler* Compiler)
			{
				Compiler->SetMaterialProperty(InProperty.Type);
				BaseMaterial->CompileProperty(Compiler, InProperty.Type);
			}, OutAnalysis);
	}

	static void GetAllTextureCoordinateIndices(const UMaterialInterface* Material, const FMaterialPropertyEx& Property, TArray<uint32>& OutTexCoords)
	{
		FMaterialAnalysisResult Analysis;
		AnalyzeMaterialProperty(Material, Property, Analysis);

		const TBitArray<>& TexCoords = Analysis.TextureCoordinates;
		for (int32 Index = 0; Index < TexCoords.Num(); Index++)
		{
			if (TexCoords[Index])
			{
				OutTexCoords.Add(Index);
			}
		}
	}

	static bool TryGetTextureCoordinateIndex(const UMaterialExpressionTextureSample* TextureSample, int32& OutTexCoord)
	{
		const UMaterialExpression* Expression = TextureSample->Coordinates.Expression;
		if (Expression == nullptr)
		{
			OutTexCoord = TextureSample->ConstCoordinate;
			return true;
		}

		if (const UMaterialExpressionTextureCoordinate* TextureCoordinate = Cast<UMaterialExpressionTextureCoordinate>(Expression))
		{
			OutTexCoord = TextureCoordinate->CoordinateIndex;
			return true;
		}

		return false;
	}

	static bool TryGetSourceTexture(UTexture2D*& OutTexture, int32& OutTexCoord, const FMaterialPropertyEx& Property, const UMaterialInterface* Material)
	{
		const FExpressionInput* MaterialInput = GetInputForProperty(Material, Property);
		if (MaterialInput == nullptr)
		{
			return false;
		}

		const UMaterialExpression* Expression = MaterialInput->Expression;
		if (Expression == nullptr)
		{
			return false;
		}

		if (const UMaterialExpressionTextureSampleParameter2D* TextureParameter = ExactCast<UMaterialExpressionTextureSampleParameter2D>(Expression))
		{
			UTexture* ParameterValue = TextureParameter->Texture;
			const UMaterialInstance* MaterialInstance = Cast<UMaterialInstance>(Material);
			if (MaterialInstance != nullptr)
			{
				const FHashedMaterialParameterInfo ParameterInfo(TextureParameter->GetParameterName());
				if (!MaterialInstance->GetTextureParameterValue(ParameterInfo, ParameterValue))
				{
					// TODO: how to handle this?
				}
			}
			OutTexture = Cast<UTexture2D>(ParameterValue);

			if (!TryGetTextureCoordinateIndex(TextureParameter, OutTexCoord))
			{
				return false;
			}
			return true;
		}

		return false;
	}

	static FLinearColor BakeMaterialProperty(const FMaterialPropertyEx& Property, const UMaterialInterface* Material)
	{
		const FBox2f DefaultTexCoordBounds = { { 0.0f, 0.0f }, { 1.0f, 1.0f } };

		FMeshData MeshSet = {};
		MeshSet.TextureCoordinateBox = FBox2D(DefaultTexCoordBounds);
		MeshSet.TextureCoordinateIndex = 0;

		FMaterialDataEx MatSet;
		MatSet.Material = const_cast<UMaterialInterface*>(Material);
		MatSet.bTangentSpaceNormal = true;

		TArray<FMeshData*> MeshSettings;
		MeshSettings.Add(&MeshSet);

		TArray<FMaterialDataEx*> MatSettings;
		MatSettings.Add(&MatSet);

		TArray<FBakeOutputEx> BakeOutputs;
		IMaterialBakingModule& Module = FModuleManager::Get().LoadModuleChecked<IMaterialBakingModule>("MaterialBaking");

		Module.SetLinearBake(true);
		Module.BakeMaterials(MatSettings, MeshSettings, BakeOutputs);
		const bool bIsLinearBake = Module.IsLinearBake(Property);
		Module.SetLinearBake(false);

		FBakeOutputEx& BakeOutput = BakeOutputs[0];

		auto color = BakeOutput.PropertyData.FindChecked(Property);
		return FLinearColor(color[0]);
	}


	MaterialExporter::MaterialExporter(const Payload& Payload)
		: mPayload(Payload)
	{
	}

	bool MaterialExporter::ProcessParameters()
	{
		// MSM_Unlit
		// MSM_DefaultLit
		auto ShadingModel = GetShadingModel(mPayload.Material);

		// process ShadingModel
		if (ShadingModel == MSM_DefaultLit)
		{
			AssetSourcePath engineMat = {};
			engineMat.bundle = SourceAssetBundle::ENGINE;
			engineMat.path = "materials/standard_pbr.mat";

			auto Mat = AssetDataBase::Get()->RegisterAsset(engineMat, false);
			mData.material = Mat->uuid;
		}
		else if (ShadingModel == MSM_Unlit)
		{
			AssetSourcePath engineMat = {};
			engineMat.bundle = SourceAssetBundle::ENGINE;
			engineMat.path = "materials/unlit.mat";

			auto Mat = AssetDataBase::Get()->RegisterAsset(engineMat, false);
			mData.material = Mat->uuid;
		}

		const FMaterialPropertyEx BaseColorProperty = ShadingModel == MSM_Unlit ? MP_EmissiveColor : MP_BaseColor;
		const auto AlphaMode = mPayload.Material->GetBlendMode();
		const FMaterialPropertyEx OpacityProperty = AlphaMode == BLEND_Masked ? MP_OpacityMask : MP_Opacity;

		FLinearColor BaseColorFactor = {1.f, 1.f, 1.f, 0.f};

		int32 TexCoord = 0;
		UTexture2D* BaseColorTexture = nullptr;

		if (AlphaMode == BLEND_Opaque)
		{
			TryGetConstantColor(BaseColorFactor, BaseColorProperty, mPayload.Material);

			if (auto *Uuid = mPayload.Textures.Find("AlbedoMap"); Uuid != nullptr) {
				mData.properties.valueMap.emplace("AlbedoMap", MaterialTexture{ *Uuid });
			}

			BaseColorFactor.A = 1.0f; // make sure base color is opaque
		}
		mData.properties.valueMap.emplace("Albedo", FromUE(BaseColorFactor));
		mData.properties.valueMap.emplace("AlphaCutoff", mPayload.Material->GetOpacityMaskClipValue());

		if (ShadingModel == MSM_DefaultLit)
		{
			float metallic = 0.1f;
			float roughness = 1.f;

			TryGetConstantScalar(metallic, MP_Metallic, mPayload.Material);
			TryGetConstantScalar(roughness, MP_Roughness, mPayload.Material);

			mData.properties.valueMap.emplace("Metallic", metallic);
			mData.properties.valueMap.emplace("Roughness", roughness);
		}

#if 0
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
#endif
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

		auto ShadingModel = GetShadingModel(Material);
		const FMaterialPropertyEx BaseColorProperty = ShadingModel == MSM_Unlit ? MP_EmissiveColor : MP_BaseColor;
		const auto AlphaMode = Material->GetBlendMode();
		const FMaterialPropertyEx OpacityProperty = AlphaMode == BLEND_Masked ? MP_OpacityMask : MP_Opacity;

		int32 TexCoord = 0;
		UTexture2D* BaseColorTexture = nullptr;
		UTexture2D* NormalTexture = nullptr;

		if (AlphaMode == BLEND_Opaque)
		{
			if (TryGetSourceTexture(BaseColorTexture, TexCoord, BaseColorProperty, Material) && BaseColorTexture != nullptr)
			{
				auto TexId = BaseColorTexture->GetOutermost()->GetPersistentGuid();
				if (Context.Tasks.Find(TexId) == nullptr) {
					auto* Task = new TextureExport(TextureExport::Payload{ BaseColorTexture });
					Task->Init();

					Context.Tasks.Emplace(TexId, Task);
				}

				auto* Task = Context.Tasks.Find(TexId);
				Payload.Textures.Emplace("AlbedoMap", (*Task)->GetUuid());
			}
		}

		if (ShadingModel != MSM_Unlit)
		{
			const FMaterialPropertyEx NormalProperty = FMaterialPropertyEx(MP_Normal);
			if (TryGetSourceTexture(NormalTexture, TexCoord, NormalProperty, Material) && NormalTexture != nullptr)
			{
				auto TexId = NormalTexture->GetOutermost()->GetPersistentGuid();
				if (Context.Tasks.Find(TexId) == nullptr) {
					auto* Task = new TextureExport(TextureExport::Payload{ NormalTexture });
					Task->Init();

					Context.Tasks.Emplace(TexId, Task);
				}

				auto* Task = Context.Tasks.Find(TexId);
				Payload.Textures.Emplace("NormalMap", (*Task)->GetUuid());
			}
		}

#if 0
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
#endif
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
		ProcessParameters();
		// BakeMaterialProperty();

		{
			auto file = AssetDataBase::Get()->CreateOrOpenFile(mPath);
			auto archive = file->WriteAsArchive();
			JsonOutputArchive json(*archive);
			mData.SaveJson(json);
		}

		auto MatSource = AssetDataBase::Get()->RegisterAsset(mPath, false);
		MatSource->category = AssetTraits<MaterialInstance>::ASSET_TYPE;
		for (auto& [name, tex] : mPayload.Textures)
		{
			MatSource->dependencies.emplace_back(tex);
		}

		UE_LOG(LogSkyEngineExporter, Log, TEXT("Exported Material %s"), *mPayload.Material->GetFullName());
	}

} // namespace sky
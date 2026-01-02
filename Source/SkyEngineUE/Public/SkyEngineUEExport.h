#pragma once

#include "CoreMinimal.h"
#include "SkyEngineUEExport.generated.h"

USTRUCT(BlueprintType)
struct FSkyEngineExportConfig
{
	GENERATED_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
	FString SkyEnginePath = TEXT("");

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
	FString SkyProjectpath = TEXT("");
	
	// 导出选项
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
	bool bExportGeometry = true;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
	bool bExportTextures = false;
    
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Export")
	bool bExportActors = true;
};

struct FSkyEngineExportContext {
	TSet<TObjectPtr<USkeleton>>		Skeletons;
	TSet<TObjectPtr<UAnimSequence>> Sequences;
};
#pragma once

namespace sky {

	class MaterialExporter {
	public:
		explicit MaterialExporter(const TObjectPtr<class UMaterialInterface>& InMaterial);
		~MaterialExporter() {}

		void Run();

	private:
		bool ProcessParameters();
		bool ProcessBaseMaterialInfo();

		TObjectPtr<class UMaterialInterface> Material;
		class UMaterial *BaseMaterial;
	};

} // namespace sky
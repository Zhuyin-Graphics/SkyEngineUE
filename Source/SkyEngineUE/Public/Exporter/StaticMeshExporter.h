#pragma once

namespace sky {

	class StaticMeshExport {
	public:
		explicit StaticMeshExport(const TObjectPtr<UStaticMesh>& InMesh) : StaticMesh(InMesh) {}
		~StaticMeshExport() {}

		void Run();

	private:
		TObjectPtr<UStaticMesh> StaticMesh;
	};

} // namespace sky
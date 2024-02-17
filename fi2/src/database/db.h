#pragma once

namespace nugget::db {
	using namespace nugget::identifier;
	bool AddAsset(std::string table, std::string path, std::string name, std::string typ, int64_t epoch);
	bool AddAssetMeta(const std::string& idName, const std::string& path, const std::string& type, const std::string& description);
	bool LookupAsset(IDType id, std::string& result);
	void StartTransaction();
	void CommitTransaction();
	void ClearTable(const std::string& table);
	void ReconcileAssetChanges();

	struct ReconcileInfo {
		int64_t id = 0;
		std::string path = "";
		std::string type = "";
	};

	bool GetNextToReconcile(ReconcileInfo& result);
	bool MarkReconciled(int64_t id);
}


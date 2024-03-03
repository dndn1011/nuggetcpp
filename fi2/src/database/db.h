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
		std::string state = "";
	};

	bool GetNextToReconcile(ReconcileInfo& result);
	bool MarkReconciled(int64_t id);
	bool IsAssetCacheDirty(IDType id);
	bool AddAssetCache(const std::string& path, const std::string& name, const std::string& type, IDType id);
	bool UpdateAssetEpoch(std::string table, std::string path, int64_t epoch);
	bool ReverseLookupAsset(const std::string& path, std::vector<IDType>& results);
	void ClearAssetMetaData();

	bool AddAssetMeta(const std::string& table, const std::string& idName, const std::string& path, const std::string& type, const std::string& description);

	bool CompareTables(
		const std::string& table1,
		const std::string& table2,
		const std::string& valueField,
		std::vector<IDType>& results
	);

	bool CopyTable(
		const std::string& table1,
		const std::string& table2);

}


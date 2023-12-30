#pragma once

namespace nugget::db {
	bool AddAsset(std::string path, std::string name, std::string type);
	bool AddAssetMeta(const std::string& idName, const std::string& path, const std::string& type, const std::string& description);
}

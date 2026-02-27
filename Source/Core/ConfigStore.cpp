#include "pch.h"
#include "Core/ConfigStore.h"

#include <fstream>
#include <sstream>
#include <algorithm>

// ══════════════════════════════════════════════════════════════
// Singleton
// ══════════════════════════════════════════════════════════════
ConfigStore& ConfigStore::Instance() {
	static ConfigStore instance;
	return instance;
}

// ══════════════════════════════════════════════════════════════
// Core API
// ══════════════════════════════════════════════════════════════
void ConfigStore::Set(const std::string& key, Value value) {
	Instance().entries_[key] = std::move(value);
}

bool ConfigStore::Has(const std::string& key) {
	return Instance().entries_.count(key) > 0;
}

void ConfigStore::Remove(const std::string& key) {
	Instance().entries_.erase(key);
}

void ConfigStore::Clear() {
	Instance().entries_.clear();
}

size_t ConfigStore::Size() {
	return Instance().entries_.size();
}

std::vector<std::string> ConfigStore::GetKeysWithPrefix(const std::string& prefix) {
	std::vector<std::string> result;
	for (const auto& [key, _] : Instance().entries_) {
		if (key.compare(0, prefix.size(), prefix) == 0)
			result.push_back(key);
	}
	std::sort(result.begin(), result.end());
	return result;
}

void ConfigStore::RemovePrefix(const std::string& prefix) {
	auto& entries = Instance().entries_;
	for (auto it = entries.begin(); it != entries.end(); ) {
		if (it->first.compare(0, prefix.size(), prefix) == 0)
			it = entries.erase(it);
		else
			++it;
	}
}


// ══════════════════════════════════════════════════════════════
// Disk I/O — Save
// ══════════════════════════════════════════════════════════════

bool ConfigStore::SaveToDisk(const std::filesystem::path& filepath) {
	NVIZ_PROFILE_FUNCTION();

	std::ofstream file(filepath);
	if (!file.is_open()) {
		NVIZ_ERROR("ConfigStore: Failed to open '{}' for writing.", filepath.string());
		return false;
	}

	file << "# NIRSViz Configuration\n";
	file << "# Auto-generated — edit at your own risk.\n\n";

	// Group entries by section (everything before the first '.')
	// so the file is organized and readable.
	auto& entries = Instance().entries_;

	// Collect and sort all keys
	std::vector<std::string> keys;
	keys.reserve(entries.size());
	for (const auto& [k, _] : entries)
		keys.push_back(k);
	std::sort(keys.begin(), keys.end());

	std::string current_section;

	for (const auto& key : keys) {
		// Extract section from "Section.rest.of.key"
		auto dot = key.find('.');
		std::string section = (dot != std::string::npos) ? key.substr(0, dot) : "General";

		// Write section header when it changes
		if (section != current_section) {
			if (!current_section.empty()) file << "\n";
			file << "[" << section << "]\n";
			current_section = section;
		}

		// Write: key = type_tag value
		const auto& value = entries.at(key);
		file << key << " = " << GetTypeTag(value) << " " << SerializeValue(value) << "\n";
	}

	file.flush();
	NVIZ_INFO("ConfigStore: Saved {} entries to '{}'", entries.size(), filepath.string());
	return true;
}


// ══════════════════════════════════════════════════════════════
// Disk I/O — Load
// ══════════════════════════════════════════════════════════════
bool ConfigStore::LoadFromDisk(const std::filesystem::path& filepath) {
	NVIZ_PROFILE_FUNCTION();

	if (!std::filesystem::exists(filepath)) {
		NVIZ_WARN("ConfigStore: '{}' not found. Starting with defaults.", filepath.string());
		return false;
	}

	std::ifstream file(filepath);
	if (!file.is_open()) {
		NVIZ_ERROR("ConfigStore: Failed to open '{}' for reading.", filepath.string());
		return false;
	}

	auto& entries = Instance().entries_;
	size_t count = 0;
	std::string line;

	while (std::getline(file, line)) {
		// Trim
		auto start = line.find_first_not_of(" \t\r\n");
		if (start == std::string::npos) continue;
		line = line.substr(start);

		// Skip comments and section headers (we use the full key, not sections)
		if (line[0] == '#' || line[0] == ';' || line[0] == '[') continue;

		// Parse: key = type_tag value
		auto eq = line.find('=');
		if (eq == std::string::npos) continue;

		std::string key = line.substr(0, eq);
		std::string rest = line.substr(eq + 1);

		// Trim key
		auto key_end = key.find_last_not_of(" \t");
		if (key_end != std::string::npos) key = key.substr(0, key_end + 1);

		// Trim rest
		auto rest_start = rest.find_first_not_of(" \t");
		if (rest_start != std::string::npos) rest = rest.substr(rest_start);

		// Split type_tag and value
		auto space = rest.find(' ');
		if (space == std::string::npos) continue;

		std::string type_tag = rest.substr(0, space);
		std::string value_str = rest.substr(space + 1);

		entries[key] = DeserializeValue(type_tag, value_str);
		count++;
	}

	NVIZ_INFO("ConfigStore: Loaded {} entries from '{}'", count, filepath.string());
	return true;
}


// ══════════════════════════════════════════════════════════════
// Serialization
// ══════════════════════════════════════════════════════════════
std::string ConfigStore::GetTypeTag(const Value& value) {
	return std::visit([](const auto& v) -> std::string {
		using T = std::decay_t<decltype(v)>;
		if constexpr (std::is_same_v<T, bool>)        return "bool";
		if constexpr (std::is_same_v<T, int>)         return "int";
		if constexpr (std::is_same_v<T, float>)       return "float";
		if constexpr (std::is_same_v<T, double>)      return "double";
		if constexpr (std::is_same_v<T, std::string>) return "string";
		if constexpr (std::is_same_v<T, glm::vec2>)   return "vec2";
		if constexpr (std::is_same_v<T, glm::vec3>)   return "vec3";
		if constexpr (std::is_same_v<T, glm::vec4>)   return "vec4";
		if constexpr (std::is_same_v<T, glm::ivec3>)  return "ivec3";
		return "unknown";
	}, value);
}

std::string ConfigStore::SerializeValue(const Value& value) {
	return std::visit([](const auto& v) -> std::string {
		using T = std::decay_t<decltype(v)>;
		if constexpr (std::is_same_v<T, bool>)        return v ? "true" : "false";
		if constexpr (std::is_same_v<T, int>)         return std::to_string(v);
		if constexpr (std::is_same_v<T, float>)       return fmt::format("{:.6f}", v);
		if constexpr (std::is_same_v<T, double>)      return fmt::format("{:.10f}", v);
		if constexpr (std::is_same_v<T, std::string>) return v;
		if constexpr (std::is_same_v<T, glm::vec2>)   return fmt::format("{:.6f}, {:.6f}", v.x, v.y);
		if constexpr (std::is_same_v<T, glm::vec3>)   return fmt::format("{:.6f}, {:.6f}, {:.6f}", v.x, v.y, v.z);
		if constexpr (std::is_same_v<T, glm::vec4>)   return fmt::format("{:.6f}, {:.6f}, {:.6f}, {:.6f}", v.x, v.y, v.z, v.w);
		if constexpr (std::is_same_v<T, glm::ivec3>)  return fmt::format("{}, {}, {}", v.x, v.y, v.z);
		return "???";
	}, value);
}

namespace {
	std::vector<std::string> SplitCSV(const std::string& s) {
		std::vector<std::string> tokens;
		std::istringstream stream(s);
		std::string token;
		while (std::getline(stream, token, ',')) {
			auto start = token.find_first_not_of(" \t");
			auto end = token.find_last_not_of(" \t");
			if (start != std::string::npos)
				tokens.push_back(token.substr(start, end - start + 1));
		}
		return tokens;
	}

	float  ParseFloat(const std::string& s)  { try { return std::stof(s); } catch (...) { return 0.0f; } }
	double ParseDouble(const std::string& s)  { try { return std::stod(s); } catch (...) { return 0.0;  } }
	int    ParseInt(const std::string& s)     { try { return std::stoi(s); } catch (...) { return 0;    } }
}

ConfigStore::Value ConfigStore::DeserializeValue(const std::string& tag, const std::string& val) {
	if (tag == "bool")   return (val == "true" || val == "1");
	if (tag == "int")    return ParseInt(val);
	if (tag == "float")  return ParseFloat(val);
	if (tag == "double") return ParseDouble(val);
	if (tag == "string") return val;

	auto t = SplitCSV(val);

	if (tag == "vec2" && t.size() >= 2)
		return glm::vec2(ParseFloat(t[0]), ParseFloat(t[1]));
	if (tag == "vec3" && t.size() >= 3)
		return glm::vec3(ParseFloat(t[0]), ParseFloat(t[1]), ParseFloat(t[2]));
	if (tag == "vec4" && t.size() >= 4)
		return glm::vec4(ParseFloat(t[0]), ParseFloat(t[1]), ParseFloat(t[2]), ParseFloat(t[3]));
	if (tag == "ivec3" && t.size() >= 3)
		return glm::ivec3(ParseInt(t[0]), ParseInt(t[1]), ParseInt(t[2]));

	NVIZ_WARN("ConfigStore: Unknown type '{}' for value '{}'", tag, val);
	return std::string(val);
}

#include "pch.h"
#include "Services/ConfigService.h"

#include <iomanip>
#include <algorithm>
#include <charconv>

// ══════════════════════════════════════════════════════════════
// ConfigSection
// ══════════════════════════════════════════════════════════════

void ConfigSection::Set(const std::string& key, ConfigValue value) {
	entries_[key] = std::move(value);
}

bool ConfigSection::Has(const std::string& key) const {
	return entries_.count(key) > 0;
}

// ── Typed getters ────────────────────────────────────────────
// All follow the same pattern: find key, try get<T>, return default on failure.

bool ConfigSection::GetBool(const std::string& key, bool default_val) const {
	auto it = entries_.find(key);
	if (it == entries_.end()) return default_val;
	if (auto* v = std::get_if<bool>(&it->second)) return *v;
	return default_val;
}

int ConfigSection::GetInt(const std::string& key, int default_val) const {
	auto it = entries_.find(key);
	if (it == entries_.end()) return default_val;
	if (auto* v = std::get_if<int>(&it->second)) return *v;
	return default_val;
}

float ConfigSection::GetFloat(const std::string& key, float default_val) const {
	auto it = entries_.find(key);
	if (it == entries_.end()) return default_val;
	if (auto* v = std::get_if<float>(&it->second)) return *v;
	return default_val;
}

double ConfigSection::GetDouble(const std::string& key, double default_val) const {
	auto it = entries_.find(key);
	if (it == entries_.end()) return default_val;
	if (auto* v = std::get_if<double>(&it->second)) return *v;
	return default_val;
}

std::string ConfigSection::GetString(const std::string& key, const std::string& def) const {
	auto it = entries_.find(key);
	if (it == entries_.end()) return def;
	if (auto* v = std::get_if<std::string>(&it->second)) return *v;
	return def;
}

glm::vec2 ConfigSection::GetVec2(const std::string& key, glm::vec2 def) const {
	auto it = entries_.find(key);
	if (it == entries_.end()) return def;
	if (auto* v = std::get_if<glm::vec2>(&it->second)) return *v;
	return def;
}

glm::vec3 ConfigSection::GetVec3(const std::string& key, glm::vec3 def) const {
	auto it = entries_.find(key);
	if (it == entries_.end()) return def;
	if (auto* v = std::get_if<glm::vec3>(&it->second)) return *v;
	return def;
}

glm::vec4 ConfigSection::GetVec4(const std::string& key, glm::vec4 def) const {
	auto it = entries_.find(key);
	if (it == entries_.end()) return def;
	if (auto* v = std::get_if<glm::vec4>(&it->second)) return *v;
	return def;
}

glm::ivec3 ConfigSection::GetIVec3(const std::string& key, glm::ivec3 def) const {
	auto it = entries_.find(key);
	if (it == entries_.end()) return def;
	if (auto* v = std::get_if<glm::ivec3>(&it->second)) return *v;
	return def;
}


// ══════════════════════════════════════════════════════════════
// ConfigService — Registration
// ══════════════════════════════════════════════════════════════

void ConfigService::Register(IConfigurable* configurable) {
	// Avoid duplicates
	auto it = std::find(configurables_.begin(), configurables_.end(), configurable);
	if (it != configurables_.end()) return;

	configurables_.push_back(configurable);
	NVIZ_INFO("ConfigService: Registered '{}'", configurable->GetConfigSectionName());
}

void ConfigService::Unregister(IConfigurable* configurable) {
	auto it = std::find(configurables_.begin(), configurables_.end(), configurable);
	if (it != configurables_.end()) {
		NVIZ_INFO("ConfigService: Unregistered '{}'", configurable->GetConfigSectionName());
		configurables_.erase(it);
	}
}


// ══════════════════════════════════════════════════════════════
// ConfigService — Save
// ══════════════════════════════════════════════════════════════

bool ConfigService::Save(const std::filesystem::path& filepath) const {
	NVIZ_PROFILE_FUNCTION();

	std::ofstream file(filepath);
	if (!file.is_open()) {
		NVIZ_ERROR("ConfigService::Save — failed to open: {}", filepath.string());
		return false;
	}

	file << "# NIRSViz Configuration\n";
	file << "# Auto-generated — do not edit while the application is running.\n\n";

	for (const auto* configurable : configurables_) {
		ConfigSection section;
		configurable->SaveConfig(section);

		const auto& entries = section.GetAll();
		if (entries.empty()) continue;

		file << "[" << configurable->GetConfigSectionName() << "]\n";

		for (const auto& [key, value] : entries) {
			file << key << " = " << GetTypeTag(value) << " " << SerializeValue(value) << "\n";
		}

		file << "\n";
	}

	file.flush();
	NVIZ_INFO("ConfigService: Saved config to '{}'", filepath.string());
	return true;
}


// ══════════════════════════════════════════════════════════════
// ConfigService — Load
// ══════════════════════════════════════════════════════════════

bool ConfigService::Load(const std::filesystem::path& filepath) {
	NVIZ_PROFILE_FUNCTION();

	if (!std::filesystem::exists(filepath)) {
		NVIZ_WARN("ConfigService::Load — file not found: {}. Using defaults.", filepath.string());
		return false;
	}

	std::ifstream file(filepath);
	if (!file.is_open()) {
		NVIZ_ERROR("ConfigService::Load — failed to open: {}", filepath.string());
		return false;
	}

	// ── Parse into section map ───────────────────────────────
	std::unordered_map<std::string, ConfigSection> sections;
	std::string current_section;
	std::string line;

	while (std::getline(file, line)) {
		// Trim whitespace
		auto start = line.find_first_not_of(" \t\r\n");
		if (start == std::string::npos) continue;
		line = line.substr(start);

		// Skip comments
		if (line[0] == '#' || line[0] == ';') continue;

		// Section header
		if (line[0] == '[') {
			auto end = line.find(']');
			if (end != std::string::npos) {
				current_section = line.substr(1, end - 1);
			}
			continue;
		}

		// Key = type value
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

		// Split "type_tag value_string"
		auto space = rest.find(' ');
		if (space == std::string::npos) continue;

		std::string type_tag = rest.substr(0, space);
		std::string value_str = rest.substr(space + 1);

		sections[current_section].Set(key, DeserializeValue(type_tag, value_str));
	}

	// ── Distribute to registered configurables ───────────────
	int loaded_count = 0;
	for (auto* configurable : configurables_) {
		auto it = sections.find(configurable->GetConfigSectionName());
		if (it != sections.end()) {
			configurable->LoadConfig(it->second);
			loaded_count++;
		}
	}

	NVIZ_INFO("ConfigService: Loaded config from '{}' ({}/{} sections matched)",
		filepath.string(), loaded_count, configurables_.size());
	return true;
}


// ══════════════════════════════════════════════════════════════
// ConfigService — Default Path
// ══════════════════════════════════════════════════════════════

std::filesystem::path ConfigService::GetDefaultConfigPath() {
	// TODO: Use Application::Get().GetSpecification().working_directory
	// For now, save next to the executable.
	return "nirsviz_config.ini";
}


// ══════════════════════════════════════════════════════════════
// Serialization Helpers
// ══════════════════════════════════════════════════════════════

std::string ConfigService::GetTypeTag(const ConfigValue& value) {
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

std::string ConfigService::SerializeValue(const ConfigValue& value) {
	return std::visit([](const auto& v) -> std::string {
		using T = std::decay_t<decltype(v)>;

		if constexpr (std::is_same_v<T, bool>) {
			return v ? "true" : "false";
		}
		if constexpr (std::is_same_v<T, int>) {
			return std::to_string(v);
		}
		if constexpr (std::is_same_v<T, float>) {
			return fmt::format("{:.6f}", v);
		}
		if constexpr (std::is_same_v<T, double>) {
			return fmt::format("{:.10f}", v);
		}
		if constexpr (std::is_same_v<T, std::string>) {
			return v;
		}
		if constexpr (std::is_same_v<T, glm::vec2>) {
			return fmt::format("{:.6f}, {:.6f}", v.x, v.y);
		}
		if constexpr (std::is_same_v<T, glm::vec3>) {
			return fmt::format("{:.6f}, {:.6f}, {:.6f}", v.x, v.y, v.z);
		}
		if constexpr (std::is_same_v<T, glm::vec4>) {
			return fmt::format("{:.6f}, {:.6f}, {:.6f}, {:.6f}", v.x, v.y, v.z, v.w);
		}
		if constexpr (std::is_same_v<T, glm::ivec3>) {
			return fmt::format("{}, {}, {}", v.x, v.y, v.z);
		}

		return "???";
	}, value);
}

// ── Parse helpers ────────────────────────────────────────────

namespace {
	// Split a comma-separated string into trimmed tokens
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

	float ParseFloat(const std::string& s) {
		try { return std::stof(s); }
		catch (...) { return 0.0f; }
	}

	double ParseDouble(const std::string& s) {
		try { return std::stod(s); }
		catch (...) { return 0.0; }
	}

	int ParseInt(const std::string& s) {
		try { return std::stoi(s); }
		catch (...) { return 0; }
	}
}

ConfigValue ConfigService::DeserializeValue(const std::string& type_hint, const std::string& value_str) {

	if (type_hint == "bool") {
		return value_str == "true" || value_str == "1";
	}
	if (type_hint == "int") {
		return ParseInt(value_str);
	}
	if (type_hint == "float") {
		return ParseFloat(value_str);
	}
	if (type_hint == "double") {
		return ParseDouble(value_str);
	}
	if (type_hint == "string") {
		return value_str;
	}
	if (type_hint == "vec2") {
		auto t = SplitCSV(value_str);
		if (t.size() >= 2)
			return glm::vec2(ParseFloat(t[0]), ParseFloat(t[1]));
		return glm::vec2(0.0f);
	}
	if (type_hint == "vec3") {
		auto t = SplitCSV(value_str);
		if (t.size() >= 3)
			return glm::vec3(ParseFloat(t[0]), ParseFloat(t[1]), ParseFloat(t[2]));
		return glm::vec3(0.0f);
	}
	if (type_hint == "vec4") {
		auto t = SplitCSV(value_str);
		if (t.size() >= 4)
			return glm::vec4(ParseFloat(t[0]), ParseFloat(t[1]), ParseFloat(t[2]), ParseFloat(t[3]));
		return glm::vec4(0.0f);
	}
	if (type_hint == "ivec3") {
		auto t = SplitCSV(value_str);
		if (t.size() >= 3)
			return glm::ivec3(ParseInt(t[0]), ParseInt(t[1]), ParseInt(t[2]));
		return glm::ivec3(0);
	}

	NVIZ_WARN("ConfigService: Unknown type tag '{}' for value '{}'", type_hint, value_str);
	return std::string(value_str);
}

#pragma once

#include "Core/Base.h"

#include <filesystem>
#include <string>
#include <unordered_map>
#include <variant>
#include <vector>
#include <mutex>

#include <glm/glm.hpp>

/// ============================================================================
/// ConfigStore — Global key-value settings store.
///
/// Any code anywhere can save and retrieve settings with one line:
///
///   ConfigStore::Set("ProbeSystem.spread_factor", 0.11f);
///   float sf = ConfigStore::Get<float>("ProbeSystem.spread_factor", 0.11f);
///
/// Or for whole structs, use the Bind helpers to connect directly to members:
///
///   ConfigStore::Save("ProbeSystem.position", settings.position);
///   ConfigStore::Load("ProbeSystem.position", settings.position);
///
/// On app startup:  ConfigStore::LoadFromDisk("nirsviz_config.ini");
/// On app shutdown:  ConfigStore::SaveToDisk("nirsviz_config.ini");
///
/// That's it. No interfaces, no registration, no inheritance.
/// ============================================================================

class ConfigStore {
public:

	// ── Supported types ──────────────────────────────────────

	using Value = std::variant<
		bool,
		int,
		float,
		double,
		std::string,
		glm::vec2,
		glm::vec3,
		glm::vec4,
		glm::ivec3
	>;

	// ── Write a value ────────────────────────────────────────

	static void Set(const std::string& key, Value value);

	// ── Read a value (returns default if key missing or wrong type) ──

	template<typename T>
	static T Get(const std::string& key, const T& default_value = T{}) {
		auto& store = Instance();
		auto it = store.entries_.find(key);
		if (it == store.entries_.end()) return default_value;
		if (auto* v = std::get_if<T>(&it->second)) return *v;
		return default_value;
	}

	// ── Bind: save a variable into the store ─────────────────
	//    ConfigStore::Save("key", my_struct.some_float);

	template<typename T>
	static void Save(const std::string& key, const T& value) {
		Set(key, Value(value));
	}

	// ── Bind: load from store into a variable ────────────────
	//    ConfigStore::Load("key", my_struct.some_float);
	//    (leaves the variable untouched if key is missing)

	template<typename T>
	static void Load(const std::string& key, T& out_value) {
		auto& store = Instance();
		auto it = store.entries_.find(key);
		if (it == store.entries_.end()) return;
		if (auto* v = std::get_if<T>(&it->second))
			out_value = *v;
	}

	// ── Check if a key exists ────────────────────────────────

	static bool Has(const std::string& key);

	// ── Remove a key ─────────────────────────────────────────

	static void Remove(const std::string& key);

	// ── Disk I/O ─────────────────────────────────────────────

	static bool SaveToDisk(const std::filesystem::path& filepath);
	static bool LoadFromDisk(const std::filesystem::path& filepath);

	// ── Utilities ────────────────────────────────────────────

	/// Returns all keys that start with the given prefix.
	/// e.g. GetKeysWithPrefix("ProbeSystem.") returns all probe settings.
	static std::vector<std::string> GetKeysWithPrefix(const std::string& prefix);

	/// Remove all keys with a given prefix (e.g. when a system is destroyed).
	static void RemovePrefix(const std::string& prefix);

	/// Clear everything.
	static void Clear();

	/// Total number of stored entries.
	static size_t Size();

private:
	ConfigStore() = default;
	static ConfigStore& Instance();

	std::unordered_map<std::string, Value> entries_;

	// ── Serialization helpers ────────────────────────────────
	static std::string SerializeValue(const Value& value);
	static std::string GetTypeTag(const Value& value);
	static Value DeserializeValue(const std::string& type_tag, const std::string& value_str);
};


/// ============================================================================
/// CONFIG_SAVE / CONFIG_LOAD macros (optional convenience)
///
/// If you have a settings struct and want to save/load all fields in one block:
///
///   void ProbeSystem::SaveSettings() {
///       CONFIG_SAVE("ProbeSystem", probe_3D_transform_settings_.position);
///       CONFIG_SAVE("ProbeSystem", probe_3D_transform_settings_.spread_factor);
///       CONFIG_SAVE("ProbeSystem", m_DrawProbes3D);
///   }
///
///   void ProbeSystem::LoadSettings() {
///       CONFIG_LOAD("ProbeSystem", probe_3D_transform_settings_.position);
///       CONFIG_LOAD("ProbeSystem", probe_3D_transform_settings_.spread_factor);
///       CONFIG_LOAD("ProbeSystem", m_DrawProbes3D);
///   }
///
/// The macro stringifies the variable name as the key, so the config file
/// will contain "ProbeSystem.probe_3D_transform_settings_.position" etc.
/// ============================================================================

#define CONFIG_SAVE(section, variable) \
	ConfigStore::Save(std::string(section) + "." + #variable, variable)

#define CONFIG_LOAD(section, variable) \
	ConfigStore::Load(std::string(section) + "." + #variable, variable)

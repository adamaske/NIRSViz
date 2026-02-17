#pragma once

#include "Core/Base.h"

#include <filesystem>
#include <string>
#include <fstream>
#include <sstream>
#include <unordered_map>
#include <functional>
#include <variant>
#include <vector>

#include <glm/glm.hpp>

/// ============================================================================
/// ConfigValue — A variant type that covers every field type in our structs.
/// ============================================================================

using ConfigValue = std::variant<
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

/// ============================================================================
/// ConfigSection — A flat key→value map representing one struct's data.
///
///   e.g. "ProbeSystem.Probe3DTransform" → { "position": vec3(...), ... }
///
/// ============================================================================

class ConfigSection {
public:
	void Set(const std::string& key, ConfigValue value);

	// Typed getters with defaults — never throw, always return something usable.
	bool        GetBool  (const std::string& key, bool default_val = false)    const;
	int         GetInt   (const std::string& key, int default_val = 0)         const;
	float       GetFloat (const std::string& key, float default_val = 0.0f)    const;
	double      GetDouble(const std::string& key, double default_val = 0.0)    const;
	std::string GetString(const std::string& key, const std::string& def = "") const;
	glm::vec2   GetVec2  (const std::string& key, glm::vec2 def = {})         const;
	glm::vec3   GetVec3  (const std::string& key, glm::vec3 def = {})         const;
	glm::vec4   GetVec4  (const std::string& key, glm::vec4 def = {})         const;
	glm::ivec3  GetIVec3 (const std::string& key, glm::ivec3 def = {})        const;

	bool Has(const std::string& key) const;

	const std::unordered_map<std::string, ConfigValue>& GetAll() const { return entries_; }

private:
	std::unordered_map<std::string, ConfigValue> entries_;
};

/// ============================================================================
/// IConfigurable — Interface that any system/service implements to participate
///                 in config save/load.
///
///   class ProbeSystem : public System, public IConfigurable {
///       void SaveConfig(ConfigSection& out) override;
///       void LoadConfig(const ConfigSection& in) override;
///       std::string GetConfigSectionName() override { return "ProbeSystem"; }
///   };
///
/// ============================================================================

class IConfigurable {
public:
	virtual ~IConfigurable() = default;

	/// Write your settings into the section. Called by ConfigService::Save().
	virtual void SaveConfig(ConfigSection& out) const = 0;

	/// Read your settings from the section. Called by ConfigService::Load().
	/// Missing keys should be silently ignored (keep current/default values).
	virtual void LoadConfig(const ConfigSection& in) = 0;

	/// A unique name for this section, e.g. "ProbeSystem", "ProjectionSettings".
	virtual std::string GetConfigSectionName() const = 0;
};

/// ============================================================================
/// ConfigService — Central config manager.
///
///   Holds a list of IConfigurable* registrants.
///   On Save(): iterates all registrants, collects their ConfigSections,
///              serializes to a .ini-style text file.
///   On Load(): parses the file, distributes sections to matching registrants.
///
///   The file format is deliberately simple (no external dependency):
///
///     [ProbeSystem]
///     position = 0.000000, -1.450000, -1.500000
///     spread_factor = 0.110000
///     draw_probes_3D = true
///
///     [ProjectionSettings]
///     Radius = 1.600000
///     wavelength = 1
///
/// ============================================================================

class ConfigService {
public:
	ConfigService() = default;
	~ConfigService() = default;

	/// Register a configurable object. Does NOT take ownership.
	/// Call during Application init after creating systems.
	void Register(IConfigurable* configurable);

	/// Unregister (e.g. before destroying a system).
	void Unregister(IConfigurable* configurable);

	/// Save all registered configurables to disk.
	bool Save(const std::filesystem::path& filepath) const;

	/// Load from disk and distribute to registered configurables.
	bool Load(const std::filesystem::path& filepath);

	/// Convenience: default config path next to the executable.
	static std::filesystem::path GetDefaultConfigPath();

private:
	std::vector<IConfigurable*> configurables_;

	// ── Serialization helpers ────────────────────────────────
	static std::string SerializeValue(const ConfigValue& value);
	static ConfigValue DeserializeValue(const std::string& type_hint, const std::string& value_str);

	// Type tag strings for the file format
	static std::string GetTypeTag(const ConfigValue& value);
};

#pragma once

#include "Core/Base.h"
#include "NIRS/NIRS.h"


// --- Projection ---
struct ProjectionData {
	uint32_t HitDataTextureID;
	uint32_t NumHits;

	std::map<NIRS::Probe::ChannelID, NIRS::Probe::Position3D> ChannelProjectionIntersections;
			 
	std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> HBOChannelValues;
	std::map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> HBRChannelValues;
};

struct ProjectionSettings {
	float StrengthMin = -2.0005f;
	float StrengthMax = 2.0005f;
	float FalloffPower = 0.5f;
	float Radius = 1.6f;
	float DecayPower = 7.0f;
};

enum class ProjectionMode {
	VERTEX_BASED
};

class ProjectionManager {
public:

	void StartProjectionMode(const ProjectionMode& mode);
	void StopProjection();

	bool IsProjection() { return is_projecting_; };

	const ProjectionMode& GetProjectionMode() { return mode_; };

	void SetProjectionWavelength(const NIRS::WavelengthType& wavelength);
	const NIRS::WavelengthType& GetProjectionWavelength() { return wavelength_; };


	const ProjectionData& GetProjectionData() { return data_; };
	ProjectionData& GetProjectionDataMutabel() { return data_; };

	const ProjectionSettings& GetProjectionSettings() { return settings_; };
	ProjectionSettings& GetProjectionSettingsMutable() { return settings_; };


private:

	bool is_projecting_ = false;

	ProjectionMode mode_ = ProjectionMode::VERTEX_BASED;
	NIRS::WavelengthType wavelength_ = NIRS::WavelengthType::HBO;

	ProjectionData data_;
	ProjectionSettings settings_;

};
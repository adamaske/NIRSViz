#pragma once
#include "Core/Base.h"

#include <string>
#include <filesystem>

#include <Eigen/Dense>

#include <highfive/H5Group.hpp>

#include "NIRS/NIRS.h"

#include "NIRS/SNIRFError.h"
#include "NIRS/SNIRFValidator.h"

class WingsDataParser;

class SNIRF {
public:
	SNIRF(const std::filesystem::path& filepath);

	void Print();

	void LoadFile(const std::filesystem::path& filepath);

	void ParseMetadataTags(const HighFive::Group& metadata);
	void ParseProbe(const HighFive::Group& probe);
	void ParseData1(const HighFive::Group& data1);
	void ParseStims(const HighFive::Group& nirs);
	void ParseWings(const HighFive::Group& nirs);
	std::string GetFilepath() { return filepath_.string(); };

	bool IsFileLoaded() { return !filepath_.empty(); };
	
	std::vector<int> GetWavelengths() { return m_Wavelengths; };

	// Get channels
	NIRS::Probe::ChannelMap GetChannels() { return probe_.channels; };

	// Get full probe, const and non-const
	NIRS::Probe::Probe& GetProbe() { return probe_; };
	const NIRS::Probe::Probe& GetProbe() const { return probe_; };

	int GetSourceAmount()	{ return probe_.sources.size(); };
	int GetDetectorAmount()	{ return probe_.detectors.size(); };


	// --- METADATA ---
	double GetSamplingRate() { return m_SamplingRate; };
	std::vector<double> GetTime() { return m_Time; };
	double GetDurationSeconds() { return m_DurationSeconds; };
	
	// --- WINGS ---
	const bool& HasWings() { return has_wings_; };
	WingsDataParser& GetWingsParser() const { return *wings_parser_; };

private:
	// --- File ---
	bool overwrite_allowed_ = false;

	std::filesystem::path filepath_;

	// --- Metadata ---
	double m_SamplingRate = 0.0;
	double m_DurationSeconds = 0.0;
	std::vector<double> m_Time = {};


	NIRS::Probe::Probe probe_;


	std::vector<int> m_Wavelengths = {};


	// --- Events ---
	std::vector <NIRS::Events::Event > events_;
	std::map<std::string, NIRS::Events::Event> event_map_;


	// --- Channel Data ---

	Eigen::Matrix<double,
		Eigen::Dynamic,
		Eigen::Dynamic,
		Eigen::RowMajor> m_ChannelData;

	bool has_wings_ = false;
	Ref<WingsDataParser> wings_parser_ = nullptr;
};
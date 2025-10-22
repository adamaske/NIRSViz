#pragma once
#include "Core/Base.h"

#include <string>
#include <filesystem>

#include <Eigen/Dense>

#include <highfive/H5Group.hpp>

#include "NIRS/NIRS.h"

class ChannelDataRegistry {
	using ChannelData = std::vector<double>;
public:
	ChannelDataRegistry() {
		NVIZ_ASSERT(!s_Instance, "ChannelDataRegistry instance already exists!");
		s_Instance = this;
	};

	int SubmitChannelData(const ChannelData& data) {
		std::size_t hash_val = HashChannelData(data);

		if (m_LookupMap.count(hash_val)) { 
			int potential_index = m_LookupMap.at(hash_val);
			if (m_DataStorage[potential_index] == data) {
				return potential_index;
			}
		}

		int new_index = static_cast<int>(m_DataStorage.size());
		m_DataStorage.push_back(data); 

		m_LookupMap[hash_val] = new_index;

		return new_index;
	}
	const ChannelData& GetChannelData(int index) const {
		if (index < 0 || index >= m_DataStorage.size()) {
			NVIZ_ERROR("Invalid channel data index: {}", index);
			throw std::out_of_range("Invalid channel data index.");
		}
		return m_DataStorage[index];
	}
	
	void Clear() {
		m_DataStorage.clear();
		m_LookupMap.clear();
	}

	static ChannelDataRegistry& Get() {
		return *s_Instance;
	}
private:
	std::vector<ChannelData> m_DataStorage;

	// Map to quickly check if a vector with the same content hash already exists.
	// Key: Hash of the ChannelData content. Value: Index in data_storage_.
	std::unordered_map<std::size_t, int> m_LookupMap;

	std::size_t HashChannelData(const ChannelData& data) const {
		// Simple hash combining the size and a few values.
		// For a more robust solution, a non-cryptographic polynomial rolling hash
		// (like FNV-1a or MurmurHash) is usually better. 
		// For demonstration, here's a basic size-and-checksum-based hash:

		std::size_t seed = data.size();
		for (double d : data) {
			// Combine hash of the double with the current seed
			// This is a common pattern for combining hashes in C++
			std::hash<double> double_hasher;
			seed ^= double_hasher(d) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}

	static ChannelDataRegistry* s_Instance;
};

class SNIRF {
public:
	SNIRF();
	SNIRF(const std::filesystem::path& filepath);

	void Print();

	void LoadFile(const std::filesystem::path& filepath);

	void ParseMetadataTags(const HighFive::Group& metadata);
	void ParseProbe(const HighFive::Group& probe);
	void ParseData1(const HighFive::Group& data1);

	std::string GetFilepath() { return m_Filepath.string(); };

	bool IsFileLoaded() { return !m_Filepath.empty(); };


	std::vector<NIRS::Landmark> GetLandmarks() { return m_Landmarks; };


	std::vector<NIRS::Probe2D> GetSources2D() { return m_Sources2D; };
	std::vector<NIRS::Probe3D> GetSources3D() { return m_Sources3D; };

	std::vector<NIRS::Probe2D> GetDetectors2D() { return m_Detectors2D; };
	std::vector<NIRS::Probe3D> GetDetectors3D() { return m_Detectors3D; };

	NIRS::Probe2D GetDetector2D(int index) { return m_Detectors2D[index]; };
	NIRS::Probe3D GetDetector3D(int index) { return m_Detectors3D[index]; };

	NIRS::Probe2D GetSource2D(int index) { return m_Sources2D[index]; };
	NIRS::Probe3D GetSource3D(int index) { return m_Sources3D[index]; };

	std::vector<NIRS::Channel> GetChannels() { return m_Channels; };

	std::vector<int> GetWavelengths() { return m_Wavelengths; };

	int GetSourceAmount()	{ return m_Sources2D.size(); };
	int GetDetectorAmount()	{ return m_Detectors2D.size(); };

	double GetSamplingRate() { return m_SamplingRate; };
	std::vector<double> GetTime() { return m_Time; };
private:
	std::filesystem::path m_Filepath = std::filesystem::path("");

	Eigen::Matrix<double,
		Eigen::Dynamic,
		Eigen::Dynamic,
		Eigen::RowMajor> m_ChannelData;

	double m_SamplingRate = 0.0;
	double m_DurationSeconds = 0.0;
	std::vector<double> m_Time = {};

	std::vector<NIRS::Probe2D> m_Sources2D	 = {};
	std::vector<NIRS::Probe2D> m_Detectors2D = {};
	std::vector<NIRS::Probe3D> m_Sources3D	 = {};
	std::vector<NIRS::Probe3D> m_Detectors3D = {};
	std::vector<NIRS::Landmark> m_Landmarks	 = {};
	std::vector<NIRS::Channel> m_Channels	 = {};
	std::vector<int> m_Wavelengths			 = {};

	ChannelDataRegistry m_ChannelDataRegistry;

};
#pragma once

#include "NIRS/NIRS.h"
#include <vector>
#include <map>
#include <Eigen/Dense>

struct ProbeData {
	std::vector<NIRS::Probe2D> sources2D;
	std::vector<NIRS::Probe2D> detectors2D;
	std::vector<NIRS::Probe3D> sources3D;
	std::vector<NIRS::Probe3D> detectors3D;
	//std::vector<NIRS::Landmark> landmarks;
};

struct MeasurementData {
	Eigen::Matrix<double,
		Eigen::Dynamic,
		Eigen::Dynamic,
		Eigen::RowMajor> channelData; // Rows are channels, columns are timepoints
	double samplingRate;
	std::vector<double> time;
};

struct MetadataMap {
	std::map<std::string, std::string> entries;
};


struct SNIRFData {
	ProbeData probe;
	MeasurementData measurements;
	MetadataMap metadata;
};
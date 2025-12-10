#pragma once

#include "Core/Base.h"
#include <map>

#include <highfive/H5Group.hpp>
#include <highfive/H5DataSet.hpp>

enum AuxilaryLabel {
	AUX1,
	AUX2, 
	AUX3, 
	AUX4, 
	AUX5,
	AUX6,
	AUX7, 
	AUX8,
	AUX9,
};

enum AuxlaryType {
	UNKNOWN, 
	RESPIRATION,
	GSR, 
	TEMPERATURE,
	EX_GA_1,
	EX_GA_2,
	EX_GA_3,
	PPG,
	SP_O2,
	HEARTRATE
};

static std::map< AuxilaryLabel, AuxlaryType> AuxLabelTypeMap = {
	{AUX1, RESPIRATION},
	{AUX2, EX_GA_1},
	{AUX3, GSR},
	{AUX4, TEMPERATURE},
	{AUX5, EX_GA_2},
	{AUX6, EX_GA_3},
	{AUX7, PPG},
	{AUX8, SP_O2},
	{AUX9, HEARTRATE},
};

struct AuxilaryData {
	AuxilaryLabel label;
	AuxlaryType type;

	std::string name;
	std::string unit;
	
	std::vector<double> data;
	std::vector<double> time;
};



class WingsDataParser {
public:
	WingsDataParser() = default;
	~WingsDataParser() = default;

	void Parse(const HighFive::Group& nirs);

	void Print();

	std::vector<AuxilaryData>& GetAuxilaryData() {
		return aux_data_;
	}

private:
	std::vector<AuxilaryData> aux_data_;
};

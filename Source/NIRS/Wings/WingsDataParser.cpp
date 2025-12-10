#include "pch.h"
#include "NIRS/Wings/WingsDataParser.h"


void WingsDataParser::Parse(const HighFive::Group& nirs)
{
	for (auto& [aux, type] : AuxLabelTypeMap) {


		std::string group_name;
		switch (aux) {
		case AUX1: group_name = "aux1"; break;
		case AUX2: group_name = "aux2"; break;
		case AUX3: group_name = "aux3"; break;
		case AUX4: group_name = "aux4"; break;
		case AUX5: group_name = "aux5"; break;
		case AUX6: group_name = "aux6"; break;
		case AUX7: group_name = "aux7"; break;
		case AUX8: group_name = "aux8"; break;
		case AUX9: group_name = "aux9"; break;
		default: continue;
		}


		if (nirs.exist(group_name)) {

			NVIZ_INFO("Parsing group: {}", group_name);

			HighFive::Group aux_group = nirs.getGroup(group_name);

			std::string name;
			aux_group.getDataSet("name").read(name);
			NVIZ_INFO("			name: {}", name);

			AuxlaryType type = UNKNOWN;
			if(name == "Respiration") type = RESPIRATION;
			else if(name == "GSR") type = GSR;
			else if(name == "Temperature") type = TEMPERATURE;
			else if(name == "ExGa_1") type = EX_GA_1;
			else if(name == "ExGa_2") type = EX_GA_2;
			else if(name == "ExGa_3") type = EX_GA_3;
			else if(name == "PPG") type = PPG;
			else if(name == "SpO2") type = SP_O2;
			else if (name == "Heartrate") type = HEARTRATE;
			
			std::string data_unit;
			aux_group.getDataSet("dataUnit").read(data_unit);
			NVIZ_INFO("			data_unit: {}", data_unit);

			// dataTimeSeries
			std::vector<double> data_time_series;
			aux_group.getDataSet("dataTimeSeries").read(data_time_series);
			NVIZ_INFO("			data_time_series size: {}", data_time_series.size());

			std::vector<double> time_;
			aux_group.getDataSet("time").read(time_);
			NVIZ_INFO("			time size: {}", time_.size());

			AuxilaryData aux_data;
			aux_data.data = data_time_series;
			aux_data.label = aux;
			aux_data.type = type;
			aux_data.time = time_;
			aux_data.unit = data_unit;
			
			aux_data_.push_back(aux_data);

		}
		else {
			NVIZ_INFO("{} does not exist in the nirs group.", group_name);
		}
	}
}

void WingsDataParser::Print()
{

	// What do I need to print ? 


}
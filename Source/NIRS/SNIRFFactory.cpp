#include "pch.h"
#include "NIRS/SNIRFFactory.h"

#include "Core/Time.h"


bool SNIRFFactory::CreateSNIRF(SNIRF& out_snirf, SNIRFType type, std::filesystem::path filepath, std::vector<SNIRFError>& out_errors)
{
	if(!std::filesystem::exists(filepath)) {
		out_errors.push_back(SNIRFError(SNIRFError::FILE_NOT_FOUND, "SNIRF Factory: File does not exist at path: " + filepath.string()));
		return false; // Return false indicating failure
	}

	SNIRF snirf(filepath);
	out_snirf = snirf;
	return true;
}

SNIRF_NEW SNIRFFactory::CreateSNIRF_NEW(SNIRFType type, std::filesystem::path filepath) {
	SNIRF_NEW snirf;
	snirf.channel_data = NIRS::Channels::LoadChannelData(filepath);

	snirf.time_data = NIRS::Time::LoadTimeData(filepath);

	snirf.optode_layout = NIRS::Probe::LoadOptodeLayout(filepath);

	snirf.events = NIRS::Events::LoadEvents(filepath);

	return snirf;

}

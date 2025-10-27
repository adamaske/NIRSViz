#include "pch.h"
#include "NIRS/Processing.h"




void NIRS::PreprocessHemodynamicData(const std::vector<NIRS::ChannelValue>& rawData, std::vector<NIRS::ChannelValue>& processedData, float samplingRate)
{
	// Convert to Optical Density
	double initial_intensity = rawData[0];
	const double EPSILON = 1e-9;

	if (initial_intensity < EPSILON) initial_intensity = EPSILON;

	processedData.resize(rawData.size());
	for (size_t i = 0; i < rawData.size(); i++) {
		double intensity = rawData[i];
		
		if (intensity < EPSILON) { // Cannot divide by zero or take log of zero
			processedData[i] = 0.0f;
			continue;
		}
		
		processedData[i] = std::log10(initial_intensity / intensity);

	}
	// Bandpass Filter
	// 0.01 to 0.5 Hz - Typical hemodynamic response range
	// High order Butterworth filter implementation would go here


    std::vector<double> signal;
    signal.reserve(processedData.size());
    for (auto& v : processedData) signal.push_back(v);

    ButterworthBandpassFilter(signal, samplingRate, 0.01f, 0.1f);
	// Optical Density to Hemoglobin Concentrations via Modified Beer-Lambert Law

    processedData = signal;

}

std::vector<double> zeroPhaseFilter(IIRFilter& filter, const std::vector<double>& input)
{
	auto forward = filter.process(input);
	std::vector<double> reversed(forward.rbegin(), forward.rend());
	filter = IIRFilter(filter); // reset state
	auto backward = filter.process(reversed);
	return std::vector<double>(backward.rbegin(), backward.rend());
}
void NIRS::ButterworthBandpassFilter(std::vector<double>& data, float sampleRate, float lowerCutoff, float higherCutoff)
{
	// Precalcuatled using Scipy's butter function
	std::vector<double> b = { 4.4568762392924175e-07, 0.0, -2.2284381196462087e-06, 0.0, 4.456876239292417e-06, 0.0, -4.456876239292417e-06, 0.0, 2.2284381196462087e-06, 0.0, -4.4568762392924175e-07, };

	std::vector<double> a = { 1.0, -9.632908071507657, 41.76951188974173, -107.36208543704004, 181.1533213783332, -209.6623428034052, 168.56546947437533, -92.96027493337579, 33.65376715250092, -7.222126070431862, 0.6976674208093903, };
	IIRFilter filter(b, a);
	data = zeroPhaseFilter(filter, data);
}

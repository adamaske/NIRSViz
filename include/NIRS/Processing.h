#pragma once

#include "Core/Base.h"
#include "NIRS/NIRS.h"

namespace NIRS
{

	void PreprocessHemodynamicData(const std::vector<NIRS::ChannelValue>& rawData,
		std::vector<NIRS::ChannelValue>& processedData,
		float samplingRate);


	void ButterworthBandpassFilter(std::vector<NIRS::ChannelValue>& data, float sampleRate, float lowerCutoff, float higherCutoff);

}


class IIRFilter {
public:
    IIRFilter(const std::vector<double>& b, const std::vector<double>& a)
        : b_(b), a_(a), z_(std::max(b.size(), a.size()) - 1, 0.0)
    {
        if (a_[0] != 1.0) {
            for (auto& coeff : b_) coeff /= a_[0];
            for (size_t i = 1; i < a_.size(); ++i) a_[i] /= a_[0];
            a_[0] = 1.0;
        }
    }

    double process(double x)
    {
        double y = b_[0] * x + z_[0];
        for (size_t i = 1; i < z_.size(); ++i)
            z_[i - 1] = b_[i] * x + z_[i] - a_[i] * y;
        z_.back() = b_.back() * x - a_.back() * y;
        return y;
    }

    std::vector<double> process(const std::vector<double>& input)
    {
        std::vector<double> output;
        output.reserve(input.size());
        for (double x : input)
            output.push_back(process(x));
        return output;
    }

private:
    std::vector<double> b_;
    std::vector<double> a_;
    std::vector<double> z_;
};
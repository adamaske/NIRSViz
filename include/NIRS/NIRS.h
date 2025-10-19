#pragma once
#include "Core/Base.h"

namespace NIRS {
	using ProbeID = uint32_t;

	static glm::vec4 SourceColor = glm::vec4(1.0f, 0.0f, 0.0f, 1.0f);
	static glm::vec4 DetectorColor = glm::vec4(0.0f, 0.0f, 1.0f, 1.0f);

	enum class WavelengthType {
		HBR = 0,
		HBO = 1,
		HBT = 2
	};

	static std::string WavelengthTypeToString(WavelengthType type) {
		switch (type) {
		case WavelengthType::HBR: return "HbR";
		case WavelengthType::HBO: return "HbO";
		}
		return "INVALID";
	}

	struct Channel {
		ProbeID SourceID;
		ProbeID DetectorID;
		WavelengthType Wavelength;
		std::vector<double> Data; // Time series data for this channel
	};

	enum ProbeType {
		SOURCE,
		DETECTOR
	};

	struct Probe2D {
		glm::vec2 Position;
		ProbeType Type;
		ProbeID ID;
	};
	struct Probe3D {
		glm::vec3 Position;
		ProbeType Type;
		ProbeID ID;
	};

	struct Landmark {
		std::string Name;
		glm::vec3 Position;
	};
}
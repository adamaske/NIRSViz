#pragma once

#include "NIRS/NIRS.h"
#include "NIRS/Snirf.h"

#include "Events/MouseEvent.h"

#include <vector>
#include <map>

class ISelectedChannelsProvider {
public:
	virtual const std::vector<NIRS::Probe::ChannelID>& GetSelectedChannels() = 0;
};

struct Channel2DVisual {
	glm::vec2 Start;
	glm::vec2 End;
	NIRS::Probe::ChannelID ChannelID;
};

struct Channel3DVisual {
	glm::vec3 Start;
	glm::vec3 End;

	NIRS::Probe::ChannelID ChannelID;
};
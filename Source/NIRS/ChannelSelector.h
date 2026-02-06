#pragma once

#include "Core/Base.h"
#include "Systems/System.h"

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
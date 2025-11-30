#pragma once

#include "Core/Base.h"
#include "NIRS/NIRS.h"

// EventBus is a singleton class that manages event subscriptions and publishing.
// Here we define the structs which represent different commands/events in the application.

struct ExitApplicationCommand {
	// No additional data needed for this event
};

// --- FILESYSTEM ---
struct OnSNIRFLoaded {
};

struct OnAnatomyLoaded {
	enum AnatomyTpye {
		Head,
		Cortex
	};
	AnatomyTpye Type;
};

// --- PROBE ---

struct OnProjectHemodynamicsToCortex {
	bool Enabled = true;
};

// --- ATLAS ---
struct OnCoordinateSystemGenerated {

	// No additional data needed for this command
};


// --- Projection ---
struct OnStartProjection {
	NIRS::WavelengthType Wavelength;
};
struct OnStopProjection {

};

struct OnChannelIntersectionsUpdated {
};

struct OnProjectionWavelengthChanged {
	NIRS::WavelengthType Wavelength;
};

struct OnProjectionSettingsChanged {
	NIRS::ProjectionSettings Settings;

};
struct OnProjectionDataChanged {
	NIRS::ProjectionData Data;
};


namespace NIRS {
	using ChannelID = uint32_t;
	using ChannelValue = double;
}
struct OnChannelValuesUpdated {
	std::map<NIRS::ChannelID, NIRS::ChannelValue> HBOValues;
	std::map<NIRS::ChannelID, NIRS::ChannelValue> HBRValues;
};

struct OnChannelsSelected {
	std::vector<uint32_t> selectedIDs;
};

// --- TIME ----
struct OnTimeChanged {
	size_t NewTimeIndex;
	double NewTime;
};
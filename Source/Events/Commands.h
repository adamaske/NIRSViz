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
	NIRS::Wavelength Wavelength;
};

struct OnStopProjection {

};


struct OnChannelIntersectionsUpdated {
};


struct OnUserStartProjectionCommand
{
};

struct OnProjectionWavelengthChanged {
	NIRS::Wavelength Wavelength;
};

struct OnProjectionSettingsChanged {
	NIRS::ProjectionSettings Settings;

};

// 
struct OnProjectionDataChanged {
	NIRS::ProjectionData data;
};

// From Plotting Layer -> To Projection Layer
struct OnProjectionTimeChanged {
	size_t time_index;
	double time_seconds;
};

struct OnChannelValuesUpdated {

	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> HBOValues;
	std::unordered_map<NIRS::Probe::ChannelID, NIRS::Probe::ChannelValue> HBRValues;
};

struct OnChannelsSelected {
	std::vector<NIRS::Probe::ChannelID> selectedIDs;
};


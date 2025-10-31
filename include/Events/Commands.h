#pragma once

#include "Core/Base.h"


// EventBus is a singleton class that manages event subscriptions and publishing.
// Here we define the structs which represent different commands/events in the application.

struct ExitApplicationCommand {
	// No additional data needed for this event
};

// --- FILESYSTEM ---
struct OnSNIRFLoaded {
};

struct HeadAnatomyLoadedEvent {

};

struct CortexAnatomyLoadedEvent {

};

// --- PROBE ---

struct OnProjectHemodynamicsToCortex {
	bool Enabled = true;
};

// --- ATLAS ---
struct CoordinateSystemGenerated {

	// No additional data needed for this command
};


// --- Channels ---
struct OnChannelIntersectionsUpdated {

};

struct OnChannelValuesUpdated {
	
};

struct OnChannelsSelected {
	std::vector<uint32_t> selectedIDs;
};
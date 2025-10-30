#pragma once

#include "Core/Base.h"


// EventBus is a singleton class that manages event subscriptions and publishing.
// Here we define the structs which represent different commands/events in the application.

struct CoordinateSystemGenerated {

	// No additional data needed for this command
};

struct OnSNIRFLoaded {
};

struct HeadAnatomyLoadedEvent {

};

struct CortexAnatomyLoadedEvent {

};

struct OnProjectHemodynamicsToCortex {
	bool Enabled = true;
};

struct ExitApplicationCommand {
	// No additional data needed for this event
};

struct OnChannelIntersectionsUpdated {

};

struct OnChannelValuesUpdated {
	
};
#pragma once
#include <string>
#include <vector>


namespace NIRS {
    namespace Events {

        struct EventMarker {
            double onset; // In seconds
            double duration; // In seconds
            double value; // Optional
        };

        struct Event {
            std::string name;

            std::vector<EventMarker> markers;

            // TODO : Data label support
        };

        struct EventsContainer {
            std::vector<Event> events;
        };
        inline EventsContainer LoadEvents(std::filesystem::path filepath) {
            EventsContainer events;

            return events;
        }

    }
}
#pragma once


#include "NIRS/Core/Probe.h"
#include "NIRS/Core/Events.h"
#include "NIRS/Core/Time.h"
#include "NIRS/Core/Channels.h"

struct SNIRF{

    // Metadata

    // Channel Data
    NIRS::Channels::ChannelData channel_data;

    // Time
    NIRS::Time::TimeData time_data;

    // Optode Layout
    NIRS::Probe::OptodeLayout optode_layout;

    // Events
    NIRS::Events::EventsContainer events;
    // TODO : EventMap where [1]->[vec]

};
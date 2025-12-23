#pragma once


#include "NIRS/Core/Probe.h"
#include "NIRS/Core/Events.h"
#include "NIRS/Core/Time.h"
#include "NIRS/Core/Channels.h"
#include "NIRS/Core/Metadata.h"
struct SNIRF_NEW{

    // Metadata
    NIRS::Metadata::Metadata metadata;

    // Channel Data
    NIRS::Channels::ChannelDataVector channel_data;

    // Time
    NIRS::Time::TimeData time_data;

    // Optode Layout
    NIRS::Probe::OptodeLayout optode_layout;

    // Events
    NIRS::Events::EventsContainer events;
    // TODO : EventMap where [1]->[vec]

};
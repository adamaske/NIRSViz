#pragma once
#include <iostream>
#include <functional>
#include <map>
#include <typeindex>
#include <memory>
#include <mutex>

class EventBus {
private:
	// Base class for all event handlers, allows storing them in a non-template map
	struct IEventHandler {
		virtual ~IEventHandler() = default;
	};
	// Concrete handler storing a list of std::function<void(const T&)> for an event type T
	template<typename T>
	struct EventHandler : IEventHandler {
		std::vector<std::function<void(const T&)>> listeners;
	};

	// Map to store a handler list for each event type (identified by std::type_index)
	std::map<std::type_index, std::unique_ptr<IEventHandler>> handlers;
	std::mutex busMutex; // Thread safety is important for a central bus

public:
public:
	// ------------------- Singleton Access -------------------
	// Use a singleton pattern to ensure a single, globally accessible EventBus
	static EventBus& Instance() {
		static EventBus instance;
		return instance;
	}
	EventBus(const EventBus&) = delete;
	EventBus& operator=(const EventBus&) = delete;

private:
	EventBus() = default;

public:
	// ------------------- Subscribe Method -------------------
	/**
	 * @brief Registers a listener function for a specific event type.
	 * @tparam T The event struct type (e.g., MouseClickEvent).
	 * @param callback The function to call when the event is published.
	 */
	template<typename T>
	void Subscribe(std::function<void(const T&)> callback) {
		std::lock_guard<std::mutex> lock(busMutex);
		std::type_index typeIndex = typeid(T);

		// Check if a handler list already exists for this event type
		if (handlers.find(typeIndex) == handlers.end()) {
			// Create a new handler if it doesn't exist
			handlers[typeIndex] = std::make_unique<EventHandler<T>>();
		}

		// Add the callback to the list
		static_cast<EventHandler<T>*>(handlers[typeIndex].get())->listeners.push_back(callback);
	}

	template<typename T>
	void Publish(const T& event) {
		// Lock only to ensure that the handler list isn't modified while we're reading it
		// A smarter approach would use a Read-Write lock, but a simple mutex is fine here.
		std::lock_guard<std::mutex> lock(busMutex);
		std::type_index typeIndex = typeid(T);

		// Check if any listeners exist for this event type
		if (handlers.find(typeIndex) != handlers.end()) {	
			// Get the concrete handler and iterate over its listeners
			auto handler = static_cast<EventHandler<T>*>(handlers[typeIndex].get());

			// Dispatch the event to all listeners
			for (const auto& listener : handler->listeners) {
				listener(event); // Call the listener function
			}
		}
	}
};

#include "Events/Commands.h"
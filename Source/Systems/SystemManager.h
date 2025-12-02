#pragma once

#include "Core/Base.h"
#include "Systems/System.h"

#include <vector>
#include <algorithm>
#include <memory>

// TODO : What happends if two systems of the same type are added?
// Should we allow that or prevent it?

class SystemManager {
public:
	SystemManager() = default;
	~SystemManager() = default;

	template<typename T, typename... args>
	T* AddSystem(args&&... parameters) {
		if (HasSystem<T>()) {
			NVIZ_RUNTIME_ERROR("SystemManager: System of type {} already exists.", typeid(T).name());
		}

		Ref<T> system = CreateRef<T>(std::forward<args>(parameters)...);
		systems_.push_back(system);
		system->OnAttach();

		NVIZ_INFO("    Attached {}", typeid(T).name());
		return system.get();
	}

	void RemoveSystem(Ref<System> system) {
		auto it = std::find(systems_.begin(), systems_.end(), system);
		if (it != systems_.end()) {
			(*it)->OnDetach();
			systems_.erase(it);
		}
	}

	void Clear() {
		for (auto& system : systems_) {
			system->OnDetach();
		}
		systems_.clear();
	}

	template<typename T>
	Ref<T> GetSystem() {
		for (auto& system : systems_) {
			if (auto casted = std::dynamic_pointer_cast<T>(system)) {
				return casted;
			}
		}

		NVIZ_RUNTIME_ERROR("SystemManager: Requested system of type {} not found.", typeid(T).name());
	}

	template<typename T>
	bool HasSystem() const {
		for (const auto& system : systems_) {
			if (std::dynamic_pointer_cast<T>(system)) {
				return true;
			}
		}
		return false;
	}

	size_t GetSystemCount() const { return systems_.size(); }

	// Iterator support for range-based for loops
	auto begin() { return systems_.begin(); }
	auto end() { return systems_.end(); }
	auto begin() const { return systems_.begin(); }
	auto end() const { return systems_.end(); }

	// Iterator overload, so we can do range based for loops on mSystems
private:
	std::vector<Ref<System>> systems_;
};

// --- INCLUDE ALL SYSTEM HEADERS HERE ---
#include "Systems/FileSystem.h"
#include "Systems/ProjectionSystem.h"
#include "Systems/ProbeSystem.h"
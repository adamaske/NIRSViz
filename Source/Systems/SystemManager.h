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

	void AddSystem(Ref<System> system) {
		m_Systems.push_back(system);
		system->OnAttach();
	}

	void RemoveSystem(Ref<System> system) {
		auto it = std::find(m_Systems.begin(), m_Systems.end(), system);
		if (it != m_Systems.end()) {
			(*it)->OnDetach();
			m_Systems.erase(it);
		}
	}

	void Clear() {
		for (auto& system : m_Systems) {
			system->OnDetach();
		}
		m_Systems.clear();
	}

	template<typename T>
	Ref<T> GetSystem() {
		for (auto& system : m_Systems) {
			if (auto casted = std::dynamic_pointer_cast<T>(system)) {
				return casted;
			}
		}
		return nullptr;
	}

	template<typename T>
	bool HasSystem() const {
		for (const auto& system : m_Systems) {
			if (std::dynamic_pointer_cast<T>(system)) {
				return true;
			}
		}
		return false;
	}

	size_t GetSystemCount() const { return m_Systems.size(); }

	// Iterator support for range-based for loops
	auto begin() { return m_Systems.begin(); }
	auto end() { return m_Systems.end(); }
	auto begin() const { return m_Systems.begin(); }
	auto end() const { return m_Systems.end(); }

	// Iterator overload, so we can do range based for loops on mSystems
private:
	std::vector<Ref<System>> m_Systems;
};

// --- INCLUDE ALL SYSTEM HEADERS HERE ---
#include "Systems/FileSystem.h"
#pragma once

#include "Core/Base.h"
#include "Plotting/Plot.h"

#include <vector>
#include <algorithm>
#include <memory>
// TODO : What happends if two Plots of the same type are added?
// Should we allow that or prevent it?

class PlotManager {
public:
	PlotManager() = default;
	~PlotManager() = default;

	template<typename T, typename... args>
	T* AddPlot(args&&... parameters) {

		if (!std::is_base_of<Plot, T>::value) {
			NVIZ_RUNTIME_ERROR("PlotManager: Type {} is not derived from Plot.", typeid(T).name());
		}

		if (HasPlot<T>()) {
			NVIZ_RUNTIME_ERROR("PlotManager: Plot of type {} already exists.", typeid(T).name());
		}

		Ref<T> Plot = CreateRef<T>(std::forward<args>(parameters)...);

		plots_.push_back(Plot);
		Plot->OnAttach();

		NVIZ_INFO("    Attached {}", typeid(T).name());
		return Plot.get();
	}

	void RemovePlot(Ref<Plot> Plot) {
		auto it = std::find(plots_.begin(), plots_.end(), Plot);
		if (it != plots_.end()) {
			(*it)->OnDetach();
			plots_.erase(it);
		}
	}

	void Clear() {
		for (auto& Plot : plots_) {
			Plot->OnDetach();
		}
		plots_.clear();
	}

	template<typename T>
	Ref<T> GetPlot() {
		for (auto& Plot : plots_) {
			if (auto casted = std::dynamic_pointer_cast<T>(Plot)) {
				return casted;
			}
		}

		NVIZ_RUNTIME_ERROR("PlotManager: Requested Plot of type {} not found.", typeid(T).name());
	}

	template<typename T>
	bool HasPlot() const {
		for (const auto& Plot : plots_) {
			if (std::dynamic_pointer_cast<T>(Plot)) {
				return true;
			}
		}
		return false;
	}

	size_t GetPlotCount() const { return plots_.size(); }

	// Iterator support for range-based for loops
	auto begin() { return plots_.begin(); }
	auto end() { return plots_.end(); }
	auto begin() const { return plots_.begin(); }
	auto end() const { return plots_.end(); }

	// Iterator overload, so we can do range based for loops on mPlots
private:
	std::vector<Ref<Plot>> plots_;
};

// --- INCLUDE ALL Plot HEADERS HERE ---
#include "Plotting/MultiPlot.h"

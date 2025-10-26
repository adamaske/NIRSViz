#pragma once


#include "Core/Base.h"
#include <unordered_map>
#include <string>
#include <memory>
#include <typeindex>
#include <stdexcept>

class AssetManager {
public:
    // --- 2. Lifecycle Methods ---
    static void Init() { /* Initialization logic */ }
    static void Shutdown() {
        s_Data.InstanceMap.clear(); // Clean up all stored resources
    }

    template<typename T>
    static Ref<T> Get(const std::string& name) {
        // 1. Get the container for type T
        auto& container = GetContainer<T>();

        // 2. Look up the asset by name
        auto it = container.Container.find(name);
        if (it == container.Container.end()) {
            // Handle error: Asset not found
            NVIZ_ERROR("AssetManager: Asset '" + name + "' not found for type.");
            return nullptr;
        }

        // 3. Cast the stored Ref<T> to the required Ref<T> and return it
        // Note: The stored type is already Ref<T>, so we just return the value.
        return it->second;
    }
    template<typename T>
    static void Register(const std::string& name, const Ref<T>& asset) {
        auto& container = GetContainer<T>();
        if (container.Container.count(name)) {
            // Handle warning: Asset already exists
            // NVIZ_WARN("AssetManager: Asset '{0}' of type already exists. Overwriting.", name);
        }
        container.Container[name] = asset;
    }

private:
    // --- 3. Internal Data Structures ---

    // Base type for type erasure: Allows storing different containers in one map.
    struct IAssetContainer {
        virtual ~IAssetContainer() = default;
    };

    // The templated container that holds actual Ref<T> assets.
    template<typename T>
    struct AssetContainer : public IAssetContainer {
        std::unordered_map<std::string, Ref<T>> Container;
    };

    // The central data structure
    struct AssetManagerData {
        // Map where the key is the type index (like the type's unique ID)
        // and the value is a unique pointer to the container for that type.
        std::unordered_map<std::type_index, std::unique_ptr<IAssetContainer>> InstanceMap;
    };

    // Static storage for the manager data (the "singleton" data)
    static AssetManagerData s_Data;


    // --- 4. Internal Helper Function ---

    /**
     * @brief Gets or creates the container map specific to type T.
     */
    template<typename T>
    static AssetContainer<T>& GetContainer() {
        std::type_index type_idx = std::type_index(typeid(T));

        // Find existing container
        auto it = s_Data.InstanceMap.find(type_idx);
        if (it != s_Data.InstanceMap.end()) {
            // Container exists, cast and return it
            return *static_cast<AssetContainer<T>*>(it->second.get());
        }

        // Container doesn't exist, create a new one
        auto new_container = std::make_unique<AssetContainer<T>>();
        AssetContainer<T>* raw_ptr = new_container.get();

        s_Data.InstanceMap[type_idx] = std::move(new_container);

        return *raw_ptr;
    }
};

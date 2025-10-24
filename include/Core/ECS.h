#pragma once

#include "Core/Base.h"

#include <iostream>
#include <map>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <vector>
#include <deque>
#include <array>
#include <string>

// --- ECS DEFINITIONS ---
using EntityID = std::uint32_t;
using ComponentTypeID = std::uint8_t;
constexpr EntityID MAX_ENTITIES = 5000;

// --- 1. Base Component and Type ID Generation ---
// Static counter to assign unique IDs to each component type
inline ComponentTypeID getNextComponentTypeID() {
    static ComponentTypeID lastID = 0;
    return lastID++;
}

/**
 * @brief Template function to get the unique ID for a component type.
 * @tparam T The component class (must be a data-only struct).
 * @return The unique ComponentTypeID for T.
 */
template <typename T>
ComponentTypeID getComponentTypeID() {
    static_assert(std::is_class<T>::value, "Component must be a class or struct.");
    static ComponentTypeID typeID = getNextComponentTypeID();
    return typeID;
}


// --- 2. Component Storage Interface (IComponentArray) ---

/**
 * @brief Base class for all component arrays, allowing them to be stored polymorphically.
 */
class IComponentArray {
public:
    virtual ~IComponentArray() = default;
    virtual void entityDestroyed(EntityID entity) = 0;
};

/**
 * @brief Templated array to store components of type T, mapped to EntityIDs.
 * This is the central storage for all component data.
 * @tparam T The component type.
 */
template <typename T>
class ComponentArray : public IComponentArray {
public:
    /**
     * @brief Adds a component object to an entity's record.
     */
    void insertData(EntityID entity, T component) {
        if (m_entityToIndex.count(entity)) {
            throw std::runtime_error("Component added to same entity more than once.");
        }

        // Add to array (data is contiguous)
        size_t newIndex = m_size;
        m_componentArray[newIndex] = std::move(component);

        // Map entity ID to its index in the array
        m_entityToIndex[entity] = newIndex;
        m_indexToEntity[newIndex] = entity;
        m_size++;
    }

    /**
     * @brief Removes a component from an entity by swapping it with the last element.
     */
    void removeData(EntityID entity) {
        if (!m_entityToIndex.count(entity)) {
            throw std::runtime_error("Removing non-existent component.");
        }

        // Get index of entity's component
        size_t indexOfRemoved = m_entityToIndex[entity];
        m_size--;

        // Swap the component to be removed with the last component in the array
        EntityID entityOfLast = m_indexToEntity[m_size];
        m_componentArray[indexOfRemoved] = m_componentArray[m_size];

        // Update map records to reflect the swap
        m_entityToIndex[entityOfLast] = indexOfRemoved;
        m_indexToEntity[indexOfRemoved] = entityOfLast;

        // Clean up records for the removed entity
        m_entityToIndex.erase(entity);
        m_indexToEntity.erase(m_size);
    }

    /**
     * @brief Gets a reference to an entity's component data.
     */
    T& getData(EntityID entity) {
        if (!m_entityToIndex.count(entity)) {
            throw std::runtime_error("Retrieving non-existent component.");
        }
        return m_componentArray[m_entityToIndex[entity]];
    }

    /**
     * @brief Notifies the component array that an entity has been destroyed.
     * If the entity had this component, it is removed.
     */
    void entityDestroyed(EntityID entity) override {
        if (m_entityToIndex.count(entity)) {
            removeData(entity);
        }
    }

private:
    // The main component storage (semi-contiguous)
    std::array<T, MAX_ENTITIES> m_componentArray{};
    size_t m_size{};

    // Maps for efficient lookup and removal
    std::map<EntityID, size_t> m_entityToIndex; // Entity ID -> Array Index
    std::map<size_t, EntityID> m_indexToEntity; // Array Index -> Entity ID
};


// --- 3. The Coordinator (The Central Hub) ---

/**
 * @brief The central coordinator manages entities, components, and the overall ECS state.
 * All systems will interact with the ECS through this class.
 */
class Coordinator {
public:
    Coordinator() {
        // Initialize the entity pool
        for (EntityID i = 0; i < MAX_ENTITIES; ++i) {
            m_availableEntities.push_back(i);
        }
    }

    // --- Entity Methods ---

    /**
     * @brief Creates and returns a new available Entity ID.
     */
    EntityID createEntity() {
        if (m_availableEntities.empty()) {
            throw std::runtime_error("Maximum entity limit reached!");
        }
        EntityID id = m_availableEntities.front();
        m_availableEntities.pop_front();
        return id;
    }

    /**
     * @brief Destroys an entity, removing all its components and making its ID available.
     */
    void destroyEntity(EntityID entity) {
        // Notify all component arrays to clean up the entity's data
        for (auto const& pair : m_componentArrays) {
            pair.second->entityDestroyed(entity);
        }

        // Return the ID to the pool
        m_availableEntities.push_back(entity);
    }

    // --- Component Methods ---

    /**
     * @brief Registers a component type with the coordinator.
     */
    template <typename T>
    void registerComponent() {
        ComponentTypeID typeID = getComponentTypeID<T>();
        if (m_componentArrays.count(typeID)) {
            std::cout << "Warning: Component type already registered.\n";
            return;
        }

        // Create the storage array for this component type
        m_componentArrays.insert({ typeID, std::make_shared<ComponentArray<T>>() });
        std::cout << "[Coordinator] Registered Component: " << typeid(T).name() << "\n";
    }

    /**
     * @brief Adds a new component to an existing entity.
     */
    template <typename T>
    void addComponent(EntityID entity, T component) {
        getComponentArray<T>()->insertData(entity, std::move(component));
    }

    /**
     * @brief Removes a component from an entity.
     */
    template <typename T>
    void removeComponent(EntityID entity) {
        getComponentArray<T>()->removeData(entity);
    }

    /**
     * @brief Gets a mutable reference to an entity's component.
     * This is the method your systems will use to READ and WRITE state.
     */
    template <typename T>
    T& getComponent(EntityID entity) {
        return getComponentArray<T>()->getData(entity);
    }

private:
    std::deque<EntityID> m_availableEntities{};

    // Key: Component Type ID
    // Value: The concrete storage array for that component type
    std::map<ComponentTypeID, std::shared_ptr<IComponentArray>> m_componentArrays{};

    /**
     * @brief Private helper to safely retrieve the component array.
     */
    template <typename T>
    std::shared_ptr<ComponentArray<T>> getComponentArray() {
        ComponentTypeID typeID = getComponentTypeID<T>();
        if (!m_componentArrays.count(typeID)) {
            throw std::runtime_error("Component not registered! Call registerComponent<T>() first.");
        }
        // Cast the generic base pointer to the specific component array type
        return std::static_pointer_cast<ComponentArray<T>>(m_componentArrays[typeID]);
    }
};

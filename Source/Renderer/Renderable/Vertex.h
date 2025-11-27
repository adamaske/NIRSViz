#pragma once
#include <glm/glm.hpp>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <unordered_map>
#include <limits>
#include <cmath> // For std::fabs

// Assuming your Vertex structure looks like this:
struct Vertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 tex_coords;

    // Fix 1: Define the operator directly inside the struct (or class).
    // Member functions defined inside a class/struct body are implicitly inline.
    bool operator==(const Vertex& other) const {
        const float EPSILON = 1e-6f;
        return glm::distance(position, other.position) < EPSILON;
    }
};

template <class T>
inline void hash_combine(std::size_t& seed, const T& v) {
    std::hash<T> hasher;
    seed ^= hasher(v) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
}

// Specializations for GLM vector hashing (since std::hash doesn't support them by default)
namespace std {
    template <> struct hash<glm::vec3> {
        size_t operator()(const glm::vec3& v) const {
            size_t seed = 0;
            // Hash the floats individually
            hash_combine(seed, v.x);
            hash_combine(seed, v.y);
            hash_combine(seed, v.z);
            return seed;
        }
    };
    template <> struct hash<glm::vec2> {
        size_t operator()(const glm::vec2& v) const {
            size_t seed = 0;
            hash_combine(seed, v.x);
            hash_combine(seed, v.y);
            return seed;
        }
    };

    // The main Vertex hash specialization
    template <> struct hash<Vertex> {
        size_t operator()(const Vertex& v) const {
            size_t seed = 0;
            // Combine the hashes of the internal vectors
            hash_combine(seed, v.position);
            return seed;
        }
    };
}
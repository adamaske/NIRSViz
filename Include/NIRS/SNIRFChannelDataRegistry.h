#pragma once
#include "Core/Base.h"
#include "NIRS/NIRS.h"

#include <vector>
#include <unordered_map>

class ChannelDataRegistry {
	using ChannelData = std::vector<double>;
public:
	ChannelDataRegistry() {
	};

	int SubmitChannelData(const ChannelData& data) {
		std::size_t hash_val = HashChannelData(data);

		if (m_LookupMap.count(hash_val)) {
			int potential_index = m_LookupMap.at(hash_val);
			if (m_DataStorage[potential_index] == data) {
				return potential_index;
			}
		}

		int new_index = static_cast<int>(m_DataStorage.size());
		m_DataStorage.push_back(data);

		m_LookupMap[hash_val] = new_index;

		return new_index;
	}

	const ChannelData& GetChannelData(int index) const {
		if (index < 0 || index >= m_DataStorage.size()) {
			NVIZ_ERROR("DataRegistry : Invalid channel data index: {}", index);
			return {};
		}
		return m_DataStorage[index];
	}

	void Clear() {
		m_DataStorage.clear();
		m_LookupMap.clear();
	}

private:
	std::vector<ChannelData> m_DataStorage;

	// Map to quickly check if a vector with the same content hash already exists.
	// Key: Hash of the ChannelData content. Value: Index in data_storage_.
	std::unordered_map<std::size_t, int> m_LookupMap;

	std::size_t HashChannelData(const ChannelData& data) const {
		// Simple hash combining the size and a few values.
		// For a more robust solution, a non-cryptographic polynomial rolling hash
		// (like FNV-1a or MurmurHash) is usually better. 
		// For demonstration, here's a basic size-and-checksum-based hash:

		std::size_t seed = data.size();
		for (double d : data) {
			// Combine hash of the double with the current seed
			// This is a common pattern for combining hashes in C++
			std::hash<double> double_hasher;
			seed ^= double_hasher(d) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		return seed;
	}

	static ChannelDataRegistry* s_Instance;
};

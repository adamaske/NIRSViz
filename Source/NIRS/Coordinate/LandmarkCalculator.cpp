#include "pch.h"
#include "NIRS/Coordinate/LandmarkCalculator.h"

namespace NIRS {

	std::map<Landmark, glm::vec3> LandmarkCalculator::CalculateLandmarksAlongPath(const std::vector<glm::vec3>& vertices, 
																						const std::vector<unsigned int>& pathIndices, 
																						const std::vector<Landmark>& labels, 
																						const std::vector<float>& percentages)
	{
		std::map<Landmark, glm::vec3> landmarkPositions;

		for(int i = 0; i < percentages.size(); i++)
		{
			auto label = labels[i];
			auto percentage = percentages[i];
			auto point = FindPointAtPercentage(vertices, pathIndices, percentage);
			
			landmarkPositions[label] = point;
		}


		return landmarkPositions;
	}

	glm::vec3 LandmarkCalculator::FindPointAtPercentage(const std::vector<glm::vec3>& vertices, 
														const std::vector<unsigned int>& pathIndices, 
														float percentage)
	{
		auto cumulativeDistances = CalculateCumulativeDistances(vertices, pathIndices);
		auto totalDistance = cumulativeDistances.back();

		auto GetDistanceByPercentage = [totalDistance](float percentage) -> float {
			if (percentage < 0.0f) percentage = 0.0f;
			if (percentage > 1.0f) percentage = 1.0f;

			return totalDistance * percentage;
		};

		float target_distance = GetDistanceByPercentage(percentage);	

		for (size_t i = 0; i < cumulativeDistances.size() - 1; i++) {
			// Check if target_distance falls within the current segment
			if (target_distance >= cumulativeDistances[i] && target_distance <= cumulativeDistances[i + 1]) {

				float start_dist = cumulativeDistances[i];
				float segment_length = cumulativeDistances[i + 1] - cumulativeDistances[i];
				float remaining_distance = target_distance - start_dist;
				float ratio = remaining_distance / segment_length;

				glm::vec3 v_start = vertices[pathIndices[i]];
				glm::vec3 v_end = vertices[pathIndices[i + 1]];

				return glm::mix(v_start, v_end, ratio);
			}
		}

		return glm::vec3(0.0f);
	}

	std::vector<float> LandmarkCalculator::CalculateCumulativeDistances(const std::vector<glm::vec3>& vertices, 
																		const std::vector<unsigned int>& pathIndices)
	{
		std::vector<float> cumulativeDistances;

		for(int i = 0; i < pathIndices.size(); i++)
		{
			if(i == 0)
			{
				cumulativeDistances.push_back(0.0f);
			}
			else
			{
				auto prevIndex = pathIndices[i - 1];
				auto currIndex = pathIndices[i];
				auto distance = glm::distance(vertices[prevIndex], vertices[currIndex]);
				cumulativeDistances.push_back(cumulativeDistances.back() + distance);
			}
		}

		return cumulativeDistances;
	}
}
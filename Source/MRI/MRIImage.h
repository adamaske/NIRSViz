#pragma once

#include "Core/Base.h"
#include <itkImage.h>
#include <vector>
#include <array>

namespace NVMRI {

	// ITK image type for 3D float images (common for MRI)
	using ImageType = itk::Image<float, 3>;
	using ImagePointer = ImageType::Pointer;

	struct MRIImage {
		ImagePointer itkImage = nullptr;

		// Cached metadata for quick access
		std::array<unsigned int, 3> dimensions = { 0, 0, 0 };
		std::array<double, 3> spacing = { 1.0, 1.0, 1.0 };
		std::array<double, 3> origin = { 0.0, 0.0, 0.0 };

		// NEW : The full voxel - to - physical affine (4 x4 , row - major )
		glm::dmat4 voxel_to_physical = glm::dmat4(1.0);

		// NEW : Physical - to - world ( accounts for RAS - >Y - up conversion )
		glm::mat4 physical_to_world = glm::mat4(1.0);

		// NEW : Combined voxel - to - world
		glm::mat4 GetVoxelToWorld() const {
			return physical_to_world * glm::mat4(voxel_to_physical);
		}

		// NEW : Intensity range for proper windowing
		float intensity_min = 0.0f;
		float intensity_max = 1.0f;

		bool IsValid() const { return itkImage != nullptr; }

		// Get total number of voxels
		size_t GetVoxelCount() const {
			return static_cast<size_t>(dimensions[0]) * dimensions[1] * dimensions[2];
		}
	};

	MRIImage CreateMRIImage(const std::filesystem::path& filepath);

	// Print image info for verification
	void PrintMRIInfo(const MRIImage& image);
}

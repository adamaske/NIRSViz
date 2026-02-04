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
		std::array<unsigned int, 3> dimensions = {0, 0, 0};
		std::array<double, 3> spacing = {1.0, 1.0, 1.0};
		std::array<double, 3> origin = {0.0, 0.0, 0.0};

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

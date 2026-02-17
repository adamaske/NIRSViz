#include "pch.h"
#include "MRI/MRIImage.h"
#include <filesystem>

#include <itkImageFileReader.h>
#include <itkNiftiImageIO.h>

namespace NVMRI
{

	MRIImage CreateMRIImage(const std::filesystem::path& filepath) {
		NVIZ_PROFILE_FUNCTION();

		MRIImage image;

		// Verify file exists
		if (!std::filesystem::exists(filepath)) {
			NVIZ_ERROR("MRIImage: File does not exist: {}", filepath.string());
			return image;
		}

		// Check extension
		std::string ext = filepath.extension().string();
		std::string stem = filepath.stem().string();
		bool isNifti = (ext == ".nii") ||
		               (ext == ".gz" && stem.size() > 4 && stem.substr(stem.size() - 4) == ".nii");

		if (!isNifti) {
			NVIZ_ERROR("MRIImage: Unsupported file format: {}", ext);
			return image;
		}

		try {
			// Create NIfTI IO
			auto niftiIO = itk::NiftiImageIO::New();

			// Create reader
			using ReaderType = itk::ImageFileReader<ImageType>;
			auto reader = ReaderType::New();
			reader->SetFileName(filepath.string());
			reader->SetImageIO(niftiIO);

			// Execute the reader
			reader->Update();

			// Get the image
			image.itkImage = reader->GetOutput();

			// Cache metadata
			auto region = image.itkImage->GetLargestPossibleRegion();
			auto size = region.GetSize();

			// Build voxel-to-physical from ITK metadata
			auto direction = image.itkImage->GetDirection();
			auto spacing = image.itkImage->GetSpacing();
			auto origin = image.itkImage->GetOrigin();

			// Construct the 4x4 affine: P = D * diag(S) * V + O
			glm::dmat4 affine(1.0);
			for (int r = 0; r < 3; r++) {
				for (int c = 0; c < 3; c++) {
					affine[c][r] = direction(r, c) * spacing[c];
				}
				affine[3][r] = origin[r];
			}
			image.voxel_to_physical = affine;

			// ITK uses LPS convention. Convert LPS -> Y-up world:
			//   LPS X (Left)      -> World -X (Right is +X)
			//   LPS Y (Posterior)  -> World -Z (Anterior is +Z)
			//   LPS Z (Superior)   -> World +Y (Superior is +Y)
			glm::mat4 lps_to_world(0.0f);
			lps_to_world[0][0] = -1.0f;  // LPS X -> -World X
			lps_to_world[1][2] = 1.0f;  // LPS Z -> +World Y
			lps_to_world[2][1] = -1.0f;  // LPS Y -> -World Z
			lps_to_world[3][3] = 1.0f;
			image.physical_to_world = lps_to_world;

			// Compute intensity range
			float* buffer = image.itkImage->GetBufferPointer();
			size_t count = image.GetVoxelCount();

			float minVal = buffer[0], maxVal = buffer[0];
			for (size_t i = 1; i < count; i++) {
				minVal = std::min(minVal, buffer[i]);
				maxVal = std::max(maxVal, buffer[i]);
			}
			image.intensity_min = minVal;
			image.intensity_max = maxVal;

			image.dimensions = {
				static_cast<unsigned int>(size[0]),
				static_cast<unsigned int>(size[1]),
				static_cast<unsigned int>(size[2])
			};

			image.spacing = {spacing[0], spacing[1], spacing[2]};
			image.origin = {origin[0], origin[1], origin[2]};

			NVIZ_INFO("MRIImage: Loaded MRI image from {}", filepath.string());
		}
		catch (const itk::ExceptionObject& ex) {
			NVIZ_ERROR("MRIImage: ITK error loading file: {}", ex.what());
			return image;
		}
		catch (const std::exception& ex) {
			NVIZ_ERROR("MRIImage: Error loading file: {}", ex.what());
			return image;
		}

		return image;
	}

	void PrintMRIInfo(const MRIImage& image) {
		if (!image.IsValid()) {
			NVIZ_WARN("MRIImage: Cannot print info - invalid image");
			return;
		}

		NVIZ_INFO("=== MRI Image Info ===");
		NVIZ_INFO("  Dimensions: {} x {} x {}",
			image.dimensions[0], image.dimensions[1], image.dimensions[2]);
		NVIZ_INFO("  Spacing: {:.4f} x {:.4f} x {:.4f} mm",
			image.spacing[0], image.spacing[1], image.spacing[2]);
		NVIZ_INFO("  Origin: ({:.2f}, {:.2f}, {:.2f})",
			image.origin[0], image.origin[1], image.origin[2]);
		NVIZ_INFO("  Total voxels: {}", image.GetVoxelCount());
		
		// Calculate min/max intensity
		auto buffer = image.itkImage->GetBufferPointer();
		size_t count = image.GetVoxelCount();

		float minVal = buffer[0];
		float maxVal = buffer[0];
		double sum = 0.0;

		for (size_t i = 0; i < count; ++i) {
			float val = buffer[i];
			minVal = std::min(minVal, val);
			maxVal = std::max(maxVal, val);
			sum += val;
		}

		NVIZ_INFO("  Intensity range: [{:.2f}, {:.2f}]", minVal, maxVal);
		NVIZ_INFO("  Mean intensity: {:.2f}", sum / count);
		NVIZ_INFO("======================");
	}
}
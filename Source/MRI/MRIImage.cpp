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
			auto spacing = image.itkImage->GetSpacing();
			auto origin = image.itkImage->GetOrigin();

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
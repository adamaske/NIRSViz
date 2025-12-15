#include <pch.h>
#include "Systems/MRISystem.h"

#include "itkImage.h"
void MRISystem::OnAttach() {
    ITKVerification();
}

void MRISystem::OnDetach() {

}

void MRISystem::OnUpdate(DeltaTime dt) {

}

void MRISystem::OnGUIRender() {

}

void MRISystem::OnEvent(Event &event) {

}

void MRISystem::RenderMenuBar() {

}

bool MRISystem::ITKVerification() {
    // 1. Define the image type (Pixel Type, Dimensions)
    using PixelType = unsigned char;
    using ImageType = itk::Image<PixelType, 3>;

    // 2. Create the image object
    auto image = ImageType::New();

    // 3. Define a small region (size)
    ImageType::IndexType start;
    start.Fill(0);

    ImageType::SizeType size;
    size.Fill(10); // 10x10x10 voxels

    ImageType::RegionType region;
    region.SetSize(size);
    region.SetIndex(start);

    // 4. Initialize the image
    image->SetRegions(region);
    try {
        image->Allocate();
        image->FillBuffer(255); // Fill with white
    } catch (const itk::ExceptionObject& err) {

        NVIZ_ERROR("ITK Exception: {}", err.what());
        return false;
    }

    NVIZ_INFO("ITK Verification Success!");
    auto size_ = image->GetLargestPossibleRegion().GetSize();
    NVIZ_INFO("Image size: [ {}, {}, {} ]", size_[0], size_[1], size_[2]);
    return true;
}

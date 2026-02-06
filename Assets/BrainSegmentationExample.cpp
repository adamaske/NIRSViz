/**
 * Brain Tissue Segmentation and Mesh Extraction using ITK
 * 
 * This example demonstrates:
 * 1. Loading a NIfTI MRI file
 * 2. Preprocessing (bias correction, smoothing)
 * 3. Segmenting grey matter and white matter using Otsu thresholding or K-means
 * 4. Extracting surface meshes using marching cubes
 * 5. Exporting meshes to OBJ format
 * 
 * Dependencies:
 * - ITK (with ITKVtkGlue module for mesh export, or use built-in mesh types)
 * - Optional: ITK with remote modules (ITKN4BiasCorrection)
 */

#include <itkImage.h>
#include <itkImageFileReader.h>
#include <itkImageFileWriter.h>
#include <itkNiftiImageIO.h>

// Preprocessing
#include <itkCurvatureFlowImageFilter.h>
#include <itkGradientAnisotropicDiffusionImageFilter.h>
#include <itkRescaleIntensityImageFilter.h>
#include <itkN4BiasFieldCorrectionImageFilter.h>

// Segmentation
#include <itkOtsuMultipleThresholdsImageFilter.h>
#include <itkConnectedComponentImageFilter.h>
#include <itkRelabelComponentImageFilter.h>
#include <itkBinaryThresholdImageFilter.h>
#include <itkMaskImageFilter.h>

// Morphological operations
#include <itkBinaryMorphologicalClosingImageFilter.h>
#include <itkBinaryMorphologicalOpeningImageFilter.h>
#include <itkBinaryBallStructuringElement.h>
#include <itkBinaryFillholeImageFilter.h>

// Mesh extraction
#include <itkBinaryMask3DMeshSource.h>
#include <itkMesh.h>
#include <itkMeshFileWriter.h>
#include <itkAntiAliasBinaryImageFilter.h>

// For marching cubes with smoother results
#include <itkCuberilleImageToMeshFilter.h>  // May need ITK remote module

#include <iostream>
#include <filesystem>

//------------------------------------------------------------------------------
// Type definitions
//------------------------------------------------------------------------------

using PixelType = float;
using LabelPixelType = unsigned char;
constexpr unsigned int Dimension = 3;

using ImageType = itk::Image<PixelType, Dimension>;
using LabelImageType = itk::Image<LabelPixelType, Dimension>;
using MeshType = itk::Mesh<float, Dimension>;

//------------------------------------------------------------------------------
// Helper: Load NIfTI image
//------------------------------------------------------------------------------
ImageType::Pointer LoadNiftiImage(const std::string& filepath)
{
    std::cout << "Loading: " << filepath << std::endl;

    auto reader = itk::ImageFileReader<ImageType>::New();
    reader->SetFileName(filepath);
    reader->SetImageIO(itk::NiftiImageIO::New());
    
    try {
        reader->Update();
    }
    catch (const itk::ExceptionObject& ex) {
        std::cerr << "Error loading image: " << ex.what() << std::endl;
        return nullptr;
    }

    auto image = reader->GetOutput();
    auto size = image->GetLargestPossibleRegion().GetSize();
    std::cout << "  Dimensions: " << size[0] << " x " << size[1] << " x " << size[2] << std::endl;

    return image;
}

//------------------------------------------------------------------------------
// Step 1: Preprocessing - Noise reduction and bias field correction
//------------------------------------------------------------------------------
ImageType::Pointer PreprocessImage(ImageType::Pointer input)
{
    std::cout << "Preprocessing image..." << std::endl;

    // 1. Rescale intensity to [0, 1000] for consistent processing
    auto rescaler = itk::RescaleIntensityImageFilter<ImageType, ImageType>::New();
    rescaler->SetInput(input);
    rescaler->SetOutputMinimum(0.0f);
    rescaler->SetOutputMaximum(1000.0f);
    rescaler->Update();

    // 2. Anisotropic diffusion for edge-preserving smoothing
    std::cout << "  Applying anisotropic diffusion..." << std::endl;
    auto diffusion = itk::GradientAnisotropicDiffusionImageFilter<ImageType, ImageType>::New();
    diffusion->SetInput(rescaler->GetOutput());
    diffusion->SetNumberOfIterations(5);
    diffusion->SetTimeStep(0.0625);  // Max stable for 3D
    diffusion->SetConductanceParameter(3.0);
    diffusion->Update();

    // 3. Optional: N4 Bias Field Correction (slower but better results)
    // Uncomment if you have ITK compiled with N4 support
    /*
    std::cout << "  Applying N4 bias correction..." << std::endl;
    auto n4 = itk::N4BiasFieldCorrectionImageFilter<ImageType, ImageType>::New();
    n4->SetInput(diffusion->GetOutput());
    n4->SetNumberOfFittingLevels(4);
    n4->SetMaximumNumberOfIterations({50, 40, 30, 20});
    n4->Update();
    return n4->GetOutput();
    */

    return diffusion->GetOutput();
}

//------------------------------------------------------------------------------
// Step 2: Brain tissue segmentation using Otsu multi-thresholding
//------------------------------------------------------------------------------
struct SegmentationResult {
    LabelImageType::Pointer grey_matter;
    LabelImageType::Pointer white_matter;
    LabelImageType::Pointer csf;  // Cerebrospinal fluid
    LabelImageType::Pointer labels;  // Combined label image
};

SegmentationResult SegmentBrainTissues(ImageType::Pointer input)
{
    std::cout << "Segmenting brain tissues..." << std::endl;

    SegmentationResult result;

    // Otsu multi-threshold to separate into 4 classes:
    // Background (0), CSF (1), Grey Matter (2), White Matter (3)
    std::cout << "  Computing Otsu thresholds (4 classes)..." << std::endl;
    
    auto otsu = itk::OtsuMultipleThresholdsImageFilter<ImageType, LabelImageType>::New();
    otsu->SetInput(input);
    otsu->SetNumberOfThresholds(3);  // Creates 4 classes (0, 1, 2, 3)
    otsu->SetNumberOfHistogramBins(256);
    otsu->SetLabelOffset(0);
    otsu->Update();

    result.labels = otsu->GetOutput();

    // Print computed thresholds
    auto thresholds = otsu->GetThresholds();
    std::cout << "  Thresholds: ";
    for (auto t : thresholds) {
        std::cout << t << " ";
    }
    std::cout << std::endl;

    // Extract individual tissue masks
    auto extractLabel = [&](LabelPixelType label) -> LabelImageType::Pointer {
        auto threshold = itk::BinaryThresholdImageFilter<LabelImageType, LabelImageType>::New();
        threshold->SetInput(result.labels);
        threshold->SetLowerThreshold(label);
        threshold->SetUpperThreshold(label);
        threshold->SetInsideValue(1);
        threshold->SetOutsideValue(0);
        threshold->Update();
        return threshold->GetOutput();
    };

    // Label assignments (typical for T1-weighted MRI):
    // 0 = background
    // 1 = CSF (darkest brain tissue)
    // 2 = Grey matter (mid intensity)
    // 3 = White matter (brightest)
    
    result.csf = extractLabel(1);
    result.grey_matter = extractLabel(2);
    result.white_matter = extractLabel(3);

    std::cout << "  Segmentation complete." << std::endl;

    return result;
}

//------------------------------------------------------------------------------
// Step 3: Clean up segmentation with morphological operations
//------------------------------------------------------------------------------
LabelImageType::Pointer CleanupSegmentation(LabelImageType::Pointer input, int radius = 1)
{
    std::cout << "  Cleaning up segmentation (radius=" << radius << ")..." << std::endl;

    using StructuringElementType = itk::BinaryBallStructuringElement<LabelPixelType, Dimension>;
    StructuringElementType element;
    element.SetRadius(radius);
    element.CreateStructuringElement();

    // Morphological closing (fill small holes)
    auto closing = itk::BinaryMorphologicalClosingImageFilter<LabelImageType, LabelImageType, StructuringElementType>::New();
    closing->SetInput(input);
    closing->SetKernel(element);
    closing->SetForegroundValue(1);
    closing->Update();

    // Morphological opening (remove small islands)
    auto opening = itk::BinaryMorphologicalOpeningImageFilter<LabelImageType, LabelImageType, StructuringElementType>::New();
    opening->SetInput(closing->GetOutput());
    opening->SetKernel(element);
    opening->SetForegroundValue(1);
    opening->Update();

    // Fill holes
    auto fillHoles = itk::BinaryFillholeImageFilter<LabelImageType>::New();
    fillHoles->SetInput(opening->GetOutput());
    fillHoles->SetForegroundValue(1);
    fillHoles->Update();

    return fillHoles->GetOutput();
}

//------------------------------------------------------------------------------
// Step 4: Keep only the largest connected component (remove noise)
//------------------------------------------------------------------------------
LabelImageType::Pointer KeepLargestComponent(LabelImageType::Pointer input)
{
    std::cout << "  Keeping largest connected component..." << std::endl;

    // Find connected components
    auto connected = itk::ConnectedComponentImageFilter<LabelImageType, LabelImageType>::New();
    connected->SetInput(input);
    connected->Update();

    // Relabel by size (largest = 1)
    auto relabel = itk::RelabelComponentImageFilter<LabelImageType, LabelImageType>::New();
    relabel->SetInput(connected->GetOutput());
    relabel->SetMinimumObjectSize(1000);  // Minimum voxels
    relabel->Update();

    std::cout << "    Found " << relabel->GetNumberOfObjects() << " components" << std::endl;

    // Keep only label 1 (largest)
    auto threshold = itk::BinaryThresholdImageFilter<LabelImageType, LabelImageType>::New();
    threshold->SetInput(relabel->GetOutput());
    threshold->SetLowerThreshold(1);
    threshold->SetUpperThreshold(1);
    threshold->SetInsideValue(1);
    threshold->SetOutsideValue(0);
    threshold->Update();

    return threshold->GetOutput();
}

//------------------------------------------------------------------------------
// Step 5: Extract mesh from binary mask using marching cubes
//------------------------------------------------------------------------------
MeshType::Pointer ExtractMeshFromMask(LabelImageType::Pointer mask)
{
    std::cout << "  Extracting mesh (marching cubes)..." << std::endl;

    // Optional: Anti-alias the binary image for smoother mesh
    // This converts binary to a signed distance-like image
    using AntiAliasType = itk::AntiAliasBinaryImageFilter<LabelImageType, ImageType>;
    auto antiAlias = AntiAliasType::New();
    antiAlias->SetInput(mask);
    antiAlias->SetMaximumRMSError(0.07);
    antiAlias->SetNumberOfIterations(50);
    antiAlias->Update();

    // Use BinaryMask3DMeshSource for mesh extraction
    // This uses marching cubes internally
    auto meshSource = itk::BinaryMask3DMeshSource<LabelImageType, MeshType>::New();
    meshSource->SetInput(mask);
    meshSource->SetObjectValue(1);
    meshSource->Update();

    auto mesh = meshSource->GetOutput();
    
    std::cout << "    Vertices: " << mesh->GetNumberOfPoints() << std::endl;
    std::cout << "    Triangles: " << mesh->GetNumberOfCells() << std::endl;

    return mesh;
}

//------------------------------------------------------------------------------
// Step 6: Write mesh to OBJ file
//------------------------------------------------------------------------------
void WriteMeshToOBJ(MeshType::Pointer mesh, const std::string& filepath)
{
    std::cout << "Writing mesh to: " << filepath << std::endl;

    // ITK's mesh writer can output OBJ directly
    auto writer = itk::MeshFileWriter<MeshType>::New();
    writer->SetFileName(filepath);
    writer->SetInput(mesh);
    
    try {
        writer->Update();
        std::cout << "  Success!" << std::endl;
    }
    catch (const itk::ExceptionObject& ex) {
        std::cerr << "  Error writing mesh: " << ex.what() << std::endl;
    }
}

//------------------------------------------------------------------------------
// Alternative: Manual OBJ export (if ITK MeshFileWriter not available)
//------------------------------------------------------------------------------
void WriteMeshToOBJManual(MeshType::Pointer mesh, const std::string& filepath)
{
    std::ofstream file(filepath);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filepath << std::endl;
        return;
    }

    file << "# OBJ exported from ITK mesh\n";
    file << "# Vertices: " << mesh->GetNumberOfPoints() << "\n";
    file << "# Faces: " << mesh->GetNumberOfCells() << "\n\n";

    // Write vertices
    for (auto it = mesh->GetPoints()->Begin(); it != mesh->GetPoints()->End(); ++it) {
        auto pt = it->Value();
        file << "v " << pt[0] << " " << pt[1] << " " << pt[2] << "\n";
    }

    file << "\n";

    // Write faces (triangles)
    for (auto it = mesh->GetCells()->Begin(); it != mesh->GetCells()->End(); ++it) {
        auto cell = it->Value();
        if (cell->GetNumberOfPoints() == 3) {
            file << "f";
            auto pointIdIterator = cell->PointIdsBegin();
            while (pointIdIterator != cell->PointIdsEnd()) {
                // OBJ indices are 1-based
                file << " " << (*pointIdIterator + 1);
                ++pointIdIterator;
            }
            file << "\n";
        }
    }

    file.close();
    std::cout << "Wrote mesh to: " << filepath << std::endl;
}

//------------------------------------------------------------------------------
// Alternative segmentation: Simple intensity thresholding
// (Use this if Otsu doesn't work well for your data)
//------------------------------------------------------------------------------
SegmentationResult SimpleThresholdSegmentation(ImageType::Pointer input,
                                                float gm_lower, float gm_upper,
                                                float wm_lower, float wm_upper)
{
    std::cout << "Segmenting with manual thresholds..." << std::endl;
    std::cout << "  Grey matter: [" << gm_lower << ", " << gm_upper << "]" << std::endl;
    std::cout << "  White matter: [" << wm_lower << ", " << wm_upper << "]" << std::endl;

    SegmentationResult result;

    // Grey matter
    auto gmThreshold = itk::BinaryThresholdImageFilter<ImageType, LabelImageType>::New();
    gmThreshold->SetInput(input);
    gmThreshold->SetLowerThreshold(gm_lower);
    gmThreshold->SetUpperThreshold(gm_upper);
    gmThreshold->SetInsideValue(1);
    gmThreshold->SetOutsideValue(0);
    gmThreshold->Update();
    result.grey_matter = gmThreshold->GetOutput();

    // White matter
    auto wmThreshold = itk::BinaryThresholdImageFilter<ImageType, LabelImageType>::New();
    wmThreshold->SetInput(input);
    wmThreshold->SetLowerThreshold(wm_lower);
    wmThreshold->SetUpperThreshold(wm_upper);
    wmThreshold->SetInsideValue(1);
    wmThreshold->SetOutsideValue(0);
    wmThreshold->Update();
    result.white_matter = wmThreshold->GetOutput();

    return result;
}

//------------------------------------------------------------------------------
// Utility: Get mesh as vertex/index arrays (for OpenGL)
//------------------------------------------------------------------------------
struct MeshData {
    std::vector<float> vertices;   // x, y, z, x, y, z, ...
    std::vector<float> normals;    // nx, ny, nz, ...
    std::vector<uint32_t> indices; // triangle indices
};

MeshData ConvertMeshToArrays(MeshType::Pointer mesh)
{
    MeshData data;
    
    // Extract vertices
    size_t numPoints = mesh->GetNumberOfPoints();
    data.vertices.reserve(numPoints * 3);
    
    for (auto it = mesh->GetPoints()->Begin(); it != mesh->GetPoints()->End(); ++it) {
        auto pt = it->Value();
        data.vertices.push_back(static_cast<float>(pt[0]));
        data.vertices.push_back(static_cast<float>(pt[1]));
        data.vertices.push_back(static_cast<float>(pt[2]));
    }

    // Extract indices
    for (auto it = mesh->GetCells()->Begin(); it != mesh->GetCells()->End(); ++it) {
        auto cell = it->Value();
        if (cell->GetNumberOfPoints() == 3) {
            auto pointIdIterator = cell->PointIdsBegin();
            while (pointIdIterator != cell->PointIdsEnd()) {
                data.indices.push_back(static_cast<uint32_t>(*pointIdIterator));
                ++pointIdIterator;
            }
        }
    }

    // Compute normals (simple per-vertex averaging)
    data.normals.resize(numPoints * 3, 0.0f);
    std::vector<int> counts(numPoints, 0);

    for (size_t i = 0; i < data.indices.size(); i += 3) {
        uint32_t i0 = data.indices[i];
        uint32_t i1 = data.indices[i + 1];
        uint32_t i2 = data.indices[i + 2];

        // Get vertices
        float v0[3] = { data.vertices[i0*3], data.vertices[i0*3+1], data.vertices[i0*3+2] };
        float v1[3] = { data.vertices[i1*3], data.vertices[i1*3+1], data.vertices[i1*3+2] };
        float v2[3] = { data.vertices[i2*3], data.vertices[i2*3+1], data.vertices[i2*3+2] };

        // Compute face normal (cross product)
        float e1[3] = { v1[0]-v0[0], v1[1]-v0[1], v1[2]-v0[2] };
        float e2[3] = { v2[0]-v0[0], v2[1]-v0[1], v2[2]-v0[2] };
        
        float n[3] = {
            e1[1]*e2[2] - e1[2]*e2[1],
            e1[2]*e2[0] - e1[0]*e2[2],
            e1[0]*e2[1] - e1[1]*e2[0]
        };

        // Accumulate to vertices
        for (uint32_t idx : {i0, i1, i2}) {
            data.normals[idx*3]   += n[0];
            data.normals[idx*3+1] += n[1];
            data.normals[idx*3+2] += n[2];
            counts[idx]++;
        }
    }

    // Normalize
    for (size_t i = 0; i < numPoints; ++i) {
        if (counts[i] > 0) {
            float len = std::sqrt(
                data.normals[i*3]*data.normals[i*3] +
                data.normals[i*3+1]*data.normals[i*3+1] +
                data.normals[i*3+2]*data.normals[i*3+2]
            );
            if (len > 1e-6f) {
                data.normals[i*3]   /= len;
                data.normals[i*3+1] /= len;
                data.normals[i*3+2] /= len;
            }
        }
    }

    return data;
}

//------------------------------------------------------------------------------
// Save label image for visualization
//------------------------------------------------------------------------------
void SaveLabelImage(LabelImageType::Pointer image, const std::string& filepath)
{
    auto writer = itk::ImageFileWriter<LabelImageType>::New();
    writer->SetFileName(filepath);
    writer->SetInput(image);
    writer->Update();
    std::cout << "Saved label image: " << filepath << std::endl;
}

//------------------------------------------------------------------------------
// Main processing pipeline
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <input.nii.gz> [output_directory]" << std::endl;
        std::cout << "\nThis program segments brain MRI into grey and white matter" << std::endl;
        std::cout << "and exports surface meshes as OBJ files." << std::endl;
        return 1;
    }

    std::string inputPath = argv[1];
    std::string outputDir = (argc > 2) ? argv[2] : ".";

    // Create output directory
    std::filesystem::create_directories(outputDir);

    // Step 1: Load image
    auto image = LoadNiftiImage(inputPath);
    if (!image) {
        return 1;
    }

    // Step 2: Preprocess
    auto preprocessed = PreprocessImage(image);

    // Step 3: Segment brain tissues
    auto segmentation = SegmentBrainTissues(preprocessed);

    // Step 4: Clean up segmentation masks
    auto gm_clean = CleanupSegmentation(segmentation.grey_matter, 2);
    auto wm_clean = CleanupSegmentation(segmentation.white_matter, 2);

    // Step 5: Keep largest connected components
    auto gm_final = KeepLargestComponent(gm_clean);
    auto wm_final = KeepLargestComponent(wm_clean);

    // Save label images for debugging
    SaveLabelImage(gm_final, outputDir + "/grey_matter_mask.nii.gz");
    SaveLabelImage(wm_final, outputDir + "/white_matter_mask.nii.gz");
    SaveLabelImage(segmentation.labels, outputDir + "/all_labels.nii.gz");

    // Step 6: Extract meshes
    std::cout << "\nExtracting grey matter mesh..." << std::endl;
    auto gm_mesh = ExtractMeshFromMask(gm_final);

    std::cout << "\nExtracting white matter mesh..." << std::endl;
    auto wm_mesh = ExtractMeshFromMask(wm_final);

    // Step 7: Write meshes
    WriteMeshToOBJManual(gm_mesh, outputDir + "/grey_matter.obj");
    WriteMeshToOBJManual(wm_mesh, outputDir + "/white_matter.obj");

    // Bonus: Get mesh data for OpenGL
    std::cout << "\nConverting meshes to OpenGL-ready arrays..." << std::endl;
    auto gm_data = ConvertMeshToArrays(gm_mesh);
    auto wm_data = ConvertMeshToArrays(wm_mesh);

    std::cout << "\nGrey matter mesh:" << std::endl;
    std::cout << "  Vertices: " << gm_data.vertices.size() / 3 << std::endl;
    std::cout << "  Triangles: " << gm_data.indices.size() / 3 << std::endl;

    std::cout << "\nWhite matter mesh:" << std::endl;
    std::cout << "  Vertices: " << wm_data.vertices.size() / 3 << std::endl;
    std::cout << "  Triangles: " << wm_data.indices.size() / 3 << std::endl;

    std::cout << "\nDone! Output files in: " << outputDir << std::endl;

    return 0;
}


//==============================================================================
// ALTERNATIVE: Using FreeSurfer-style approach with ITK
// If you want more accurate results, consider these approaches:
//==============================================================================

/*
 * For production-quality brain segmentation, consider:
 * 
 * 1. FreeSurfer (https://surfer.nmr.mgh.harvard.edu/)
 *    - Gold standard for cortical surface reconstruction
 *    - Run: recon-all -s subject -i input.nii.gz -all
 *    - Outputs: lh.pial, rh.pial, lh.white, rh.white meshes
 * 
 * 2. FSL FAST (https://fsl.fmrib.ox.ac.uk/fsl/)
 *    - fast -t 1 -n 3 -o output input.nii.gz
 *    - Outputs probability maps for CSF, GM, WM
 * 
 * 3. ANTs (http://stnava.github.io/ANTs/)
 *    - Atropos for tissue segmentation
 *    - N4BiasFieldCorrection for preprocessing
 * 
 * 4. SPM (https://www.fil.ion.ucl.ac.uk/spm/)
 *    - Unified segmentation approach
 *    - MATLAB-based
 * 
 * These tools provide much better results than simple thresholding,
 * but are external dependencies. The ITK code above is a simplified
 * approach suitable for visualization/prototyping.
 */

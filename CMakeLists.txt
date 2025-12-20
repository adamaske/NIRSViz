cmake_minimum_required(VERSION 3.15)

# Set CMake policies to suppress warnings
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW) # Use upper-case <PACKAGENAME>_ROOT variables
endif()
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD) # Continue using FindBoost module
endif()

project(NIRSViz VERSION 1.0 LANGUAGES CXX)

# =============================================================================
# Project Configuration
# =============================================================================
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# =============================================================================
# Directory Structure
# =============================================================================
set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Vendor")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Include")
set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Data")
set(ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Assets")

# =============================================================================
# Platform-Specific Configuration
# =============================================================================
if(MSVC)
    #add_compile_options(/source_charset:utf-8)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")

endif()


# =============================================================================
# External Dependencies
# =============================================================================

# Boost
# Set this to your Boost installation path after installing prebuilt binaries
# Common paths: C:/local/boost_1_84_0 or C:/Program Files/boost/boost_1_84_0
set(Boost_ROOT "C:/sdk/boost_1_90_0" CACHE PATH "Boost installation directory")
set(Boost_USE_STATIC_LIBS ON)
set(Boost_USE_MULTITHREADED ON)
set(Boost_USE_STATIC_RUNTIME OFF)
# Boost.Graph is header-only, so no COMPONENTS needed
find_package(Boost REQUIRED)


# Assimp 
CONFIGURE_FILE(
  ${VENDOR_DIR}/assimp/include/assimp/config.h.in
  ${VENDOR_DIR}/assimp/include/assimp/config.h
)
add_library(Assimp::Assimp SHARED IMPORTED)
set_target_properties(Assimp::Assimp PROPERTIES
    # IMPLIB (Import Library) points to the .lib file that the linker needs.
    IMPORTED_IMPLIB_DEBUG
    "${VENDOR_DIR}/assimp/build/lib/Debug/assimp-vc143-mtd.lib"

    # LOCATION points to the actual .dll file that the executable needs at runtime.
    IMPORTED_LOCATION_DEBUG
    "${VENDOR_DIR}/assimp/build/bin/Debug/assimp-vc143-mtd.dll"
)
# --- RELEASE Configuration ---
# NOTE: Ensure these paths and file names match your Release configuration!
set_target_properties(Assimp::Assimp PROPERTIES
        # IMPLIB for Release configuration
        IMPORTED_IMPLIB_RELEASE
        "${VENDOR_DIR}/assimp/build/lib/Release/assimp-vc143-mt.lib"

        # LOCATION for Release configuration
        IMPORTED_LOCATION_RELEASE
        "${VENDOR_DIR}/assimp/build/bin/Release/assimp-vc143-mt.dll"
)

# Madman91/BVH
add_subdirectory(${VENDOR_DIR}/bvh)

#jblancoc/nanoflann
add_subdirectory(${VENDOR_DIR}/nanoflann)

# Get the DLL path based on the build configuration
# We use generator expressions to select the correct path (.dll) at build time
set(ASSIMP_DLL_PATH_DEBUG "${VENDOR_DIR}/assimp/build/bin/Debug/assimp-vc143-mtd.dll")
set(ASSIMP_DLL_PATH_RELEASE "${VENDOR_DIR}/assimp/build/bin/Release/assimp-vc143-mt.dll")

# FMTlib
add_subdirectory(${VENDOR_DIR}/fmt)

# OpenGL
find_package(OpenGL REQUIRED)

# HDF5
set(HDF5_ROOT "C:/Program Files/HDF_Group/HDF5/1.14.6")
find_package(HDF5 REQUIRED)

# GLFW
add_subdirectory(${VENDOR_DIR}/glfw)

# spdlog
add_subdirectory(${VENDOR_DIR}/spdlog)

# Native File Dialog Extended
add_subdirectory(${VENDOR_DIR}/nativefiledialog-extended)

set(ITK_DIR "C:/sdk/vcpkg/installed/x64-windows/share/itk")
# ITK - Using pre-built installation
# use vcpkg - install itk:x64-windows, refresech cmake chache
find_package(ITK REQUIRED COMPONENTS
    ITKIOImageBase
    ITKIONIFTI
    ITKIOGDCM
    ITKWatersheds
    ITKSmoothing
    ITKImageGradient
)

# Set ITK binary directory for DLL location
if(WIN32 AND ITK_FOUND)
    # Determine the vcpkg toolchain directory for DLLs
    get_filename_component(ITK_BIN_DIR "${ITK_DIR}/../../bin" ABSOLUTE)
    message(STATUS "ITK binaries directory: ${ITK_BIN_DIR}")

    # Collect all ITK-related DLLs at configure time
    file(GLOB ITK_RUNTIME_DLLS
        "${ITK_BIN_DIR}/ITK*.dll"
        "${ITK_BIN_DIR}/gdcm*.dll"
        "${ITK_BIN_DIR}/jpeg*.dll"
        "${ITK_BIN_DIR}/libpng*.dll"
        "${ITK_BIN_DIR}/tiff*.dll"
        "${ITK_BIN_DIR}/zlib*.dll"
        "${ITK_BIN_DIR}/libexpat*.dll"
        "${ITK_BIN_DIR}/double-conversion*.dll"
        "${ITK_BIN_DIR}/aec*.dll"
    )
    list(LENGTH ITK_RUNTIME_DLLS ITK_DLL_COUNT)
    message(STATUS "Found ${ITK_DLL_COUNT} ITK runtime DLLs")
endif()

# CTD
#add_subdirectory(${VENDOR_DIR}/CDT/CDT)# In consuming CMakeLists.txt
# THe .lib is located in ${VENDOR_DIR}/CDT/CDT/build/Debug/CDT.lib
# and ${VENDOR_DIR}/CDT/CDT/build/Release/CDT.lib
add_library(CDT_Static STATIC IMPORTED)
set_target_properties(CDT_Static PROPERTIES
        IMPORTED_LOCATION_DEBUG "${VENDOR_DIR}/CDT/CDT/build/Debug/CDT.lib"
        IMPORTED_LOCATION_RELEASE "${VENDOR_DIR}/CDT/CDT/build/Release/CDT.lib"
        INTERFACE_INCLUDE_DIRECTORIES "${VENDOR_DIR}/CDT/CDT/include"
)

# SQLiteCpp
add_subdirectory(${VENDOR_DIR}/SQLiteCpp)

# =============================================================================
# ImGui Library
# =============================================================================
set(IMGUI_SOURCES
    ${VENDOR_DIR}/imgui/imgui.cpp
    ${VENDOR_DIR}/imgui/imgui_draw.cpp
    ${VENDOR_DIR}/imgui/imgui_tables.cpp
    ${VENDOR_DIR}/imgui/imgui_widgets.cpp
    ${VENDOR_DIR}/imgui/imgui_demo.cpp
    ${VENDOR_DIR}/imgui/backends/imgui_impl_glfw.cpp
    ${VENDOR_DIR}/imgui/backends/imgui_impl_opengl3.cpp
    ${VENDOR_DIR}/implot/implot.cpp
    ${VENDOR_DIR}/implot/implot_demo.cpp
    ${VENDOR_DIR}/implot/implot_items.cpp
)

# =============================================================================
# Main Executable
# =============================================================================

# Collect all source files recursively
file(GLOB_RECURSE PROJECT_SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE PROJECT_HEADERS "${SRC_DIR}/*.h")


# Create executable
add_executable(${PROJECT_NAME}
    ${PROJECT_SOURCES}
    ${IMGUI_SOURCES}
    ${VENDOR_DIR}/glad/src/glad.cpp
)
# Precompiled headers (if you have pch.h in src/)
target_precompile_headers(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR}/pch.h)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SRC_DIR}                           # Your source files can now find each other
    ${INCLUDE_DIR}                       # Your own include directory
    ${VENDOR_DIR}/glad/include
    ${VENDOR_DIR}/glm
    ${VENDOR_DIR}/imgui
    ${VENDOR_DIR}/implot
    ${VENDOR_DIR}/stb
    ${VENDOR_DIR}/entt/src
    ${VENDOR_DIR}/HighFive/include
    ${VENDOR_DIR}/Eigen/include
    ${VENDOR_DIR}/fmt/include            # Added include for FMTlib
    ${VENDOR_DIR}/assimp/include
    ${VENDOR_DIR}/bvh/include
    ${Boost_INCLUDE_DIRS}                # Boost headers
    ${VENDOR_DIR}/nanoflann/include
    ${VENDOR_DIR}/CDT/CDT/include
    ${VENDOR_DIR}/SQLiteCpp/include
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    spdlog::spdlog
    HDF5::HDF5
    nfd
    $<$<BOOL:${MINGW}>:ws2_32>
    fmt
    Assimp::Assimp
    bvh
    ${ITK_LIBRARIES}
    CDT_Static
    SQLiteCpp
)

# =============================================================================
# Installation
# =============================================================================
file(GLOB_RECURSE DATA_SHARE "${DATA_DIR}/*")
file(GLOB_RECURSE ASSET_SHARE "${ASSET_DIR}/*")

install(FILES ${DATA_SHARE}
    DESTINATION "share/${PROJECT_NAME}/Data"
    COMPONENT Data
)

install(FILES ${ASSET_SHARE}
    DESTINATION "share/${PROJECT_NAME}/Assets"
    COMPONENT Assets
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)

# Install ITK runtime DLLs (Windows only)
if(WIN32 AND ITK_FOUND AND ITK_RUNTIME_DLLS)
    install(FILES ${ITK_RUNTIME_DLLS}
        DESTINATION bin
        COMPONENT Runtime
    )
endif()

# Install HDF5 runtime DLLs (Windows only)
if(WIN32 AND HDF5_FOUND)
    get_filename_component(HDF5_BIN_DIR "${HDF5_ROOT}/bin" ABSOLUTE)
    if(EXISTS "${HDF5_BIN_DIR}")
        file(GLOB HDF5_RUNTIME_DLLS
            "${HDF5_BIN_DIR}/hdf5*.dll"
        )
        if(HDF5_RUNTIME_DLLS)
            install(FILES ${HDF5_RUNTIME_DLLS}
                DESTINATION bin
                COMPONENT Runtime
            )
        endif()
    endif()
endif()

# Install Assimp DLL (Windows only)
if(WIN32)
    install(FILES
        "$<IF:$<CONFIG:Debug>,${ASSIMP_DLL_PATH_DEBUG},${ASSIMP_DLL_PATH_RELEASE}>"
        DESTINATION bin
        COMPONENT Runtime
    )
endif()

# Use a POST_BUILD command to copy the DLL after the main executable is built.
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    # Generator expression to select the correct DLL path based on configuration
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<IF:$<CONFIG:Debug>,${ASSIMP_DLL_PATH_DEBUG},${ASSIMP_DLL_PATH_RELEASE}>"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"  # Updated to use ${PROJECT_NAME} for target directory
    COMMENT "Copying Assimp DLL to executable output directory"
)

# Copy ITK DLLs and dependencies to executable directory (Windows only)
if(WIN32 AND ITK_FOUND AND ITK_RUNTIME_DLLS)
    foreach(DLL_FILE ${ITK_RUNTIME_DLLS})
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${DLL_FILE}"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            VERBATIM
        )
    endforeach()

    add_custom_command(
        TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E echo "ITK runtime DLLs copied to executable directory"
        COMMENT "Copying ITK runtime dependencies"
    )
endif()

# Copy HDF5 DLLs to executable directory (Windows only)
if(WIN32 AND HDF5_FOUND)
    # Get HDF5 bin directory
    get_filename_component(HDF5_BIN_DIR "${HDF5_ROOT}/bin" ABSOLUTE)
    if(EXISTS "${HDF5_BIN_DIR}")
        add_custom_command(
            TARGET ${PROJECT_NAME} POST_BUILD
            COMMAND ${CMAKE_COMMAND} -E echo "Copying HDF5 DLLs to executable directory..."
            COMMAND ${CMAKE_COMMAND} -E copy_if_different
                "${HDF5_BIN_DIR}/hdf5.dll"
                "${HDF5_BIN_DIR}/hdf5_cpp.dll"
                "${HDF5_BIN_DIR}/hdf5_hl.dll"
                "${HDF5_BIN_DIR}/hdf5_hl_cpp.dll"
                "$<TARGET_FILE_DIR:${PROJECT_NAME}>"
            COMMAND ${CMAKE_COMMAND} -E echo "HDF5 DLLs copied successfully"
            COMMENT "Copying HDF5 runtime dependencies to executable directory"
            VERBATIM
        )
    endif()
endif()

# =============================================================================
# CPack Configuration for MSI Installer
# =============================================================================

# Basic package information
set(CPACK_PACKAGE_NAME "NIRSViz")
set(CPACK_PACKAGE_VENDOR "Your Organization")
set(CPACK_PACKAGE_DESCRIPTION_SUMMARY "NIRS Visualization and Analysis Tool")
set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION}")
set(CPACK_PACKAGE_VERSION_MAJOR "1")
set(CPACK_PACKAGE_VERSION_MINOR "0")
set(CPACK_PACKAGE_VERSION_PATCH "0")
set(CPACK_PACKAGE_INSTALL_DIRECTORY "NIRSViz")

# Windows-specific settings for WiX generator
if(WIN32)
    # Use WiX generator for MSI creation
    set(CPACK_GENERATOR "WIX")

    # WiX-specific settings
    set(CPACK_WIX_UPGRADE_GUID "A1B2C3D4-E5F6-7890-ABCD-EF1234567890")  # Generate a unique GUID for your app
    set(CPACK_WIX_PRODUCT_GUID "*")  # Use * to generate new GUID for each version

    # Optional: License file (RTF format required by WiX)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.rtf")
        set(CPACK_WIX_LICENSE_RTF "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.rtf")
    endif()

    # Product icon (if you have one)
    # set(CPACK_WIX_PRODUCT_ICON "${CMAKE_CURRENT_SOURCE_DIR}/icon.ico")

    # Program Menu folder
    set(CPACK_WIX_PROGRAM_MENU_FOLDER "NIRSViz")

    # Add to PATH (optional)
    # set(CPACK_WIX_PATCH_FILE "${CMAKE_CURRENT_SOURCE_DIR}/wix_patch.xml")

    # Desktop shortcut
    set(CPACK_CREATE_DESKTOP_LINKS ${PROJECT_NAME})

    # Start Menu shortcuts
    set(CPACK_PACKAGE_EXECUTABLES "${PROJECT_NAME}" "NIRSViz")

    # Install directory in Program Files
    set(CPACK_WIX_INSTALL_SCOPE "perMachine")  # Install for all users

    # UI settings (optional custom images)
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/installer_banner.bmp")
        set(CPACK_WIX_UI_BANNER "${CMAKE_CURRENT_SOURCE_DIR}/installer_banner.bmp")  # 493x58 pixels
    endif()
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/installer_dialog.bmp")
        set(CPACK_WIX_UI_DIALOG "${CMAKE_CURRENT_SOURCE_DIR}/installer_dialog.bmp")  # 493x312 pixels
    endif()
endif()

# Component installation (separate components for selective installation)
set(CPACK_COMPONENTS_ALL Runtime Data Assets)
set(CPACK_COMPONENT_RUNTIME_DISPLAY_NAME "NIRSViz Application")
set(CPACK_COMPONENT_RUNTIME_DESCRIPTION "Core application and runtime libraries")
set(CPACK_COMPONENT_RUNTIME_REQUIRED ON)
set(CPACK_COMPONENT_DATA_DISPLAY_NAME "Sample Data Files")
set(CPACK_COMPONENT_DATA_DESCRIPTION "Example NIRS data files")
set(CPACK_COMPONENT_ASSETS_DISPLAY_NAME "Application Assets")
set(CPACK_COMPONENT_ASSETS_DESCRIPTION "Shaders, models, and textures")
set(CPACK_COMPONENT_ASSETS_REQUIRED ON)

# Resource file location
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/LICENSE.txt")
set(CPACK_RESOURCE_FILE_README "${CMAKE_CURRENT_SOURCE_DIR}/README.md")

# Include CPack module (must be last)
include(CPack)
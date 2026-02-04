cmake_minimum_required(VERSION 3.15)

# Set CMake policies to suppress warnings
if(POLICY CMP0144)
    cmake_policy(SET CMP0144 NEW) # Use upper-case <PACKAGENAME>_ROOT variables
endif()
if(POLICY CMP0167)
    cmake_policy(SET CMP0167 OLD) # Continue using FindBoost module
endif()

# =============================================================================
# Project Configuration
# =============================================================================
project(NIRSViz VERSION 1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Vendor")
set(SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Source")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Include")
set(ASSET_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Assets")
set(FORMS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/Forms")

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

# QT 6
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 COMPONENTS Widgets OpenGL OpenGLWidgets GUI REQUIRED)


# OpenGL
set(GLFW_INSTALL OFF CACHE BOOL "" FORCE)
find_package(OpenGL REQUIRED)

# Boost
# Set this to your Boost installation path after installing prebuilt binaries
# Common paths: C:/local/boost_1_84_0 or C:/Program Files/boost/boost_1_84_0
set(Boost_ROOT "C:\\sdk\\boost_1_90_0" CACHE PATH "Boost installation directory")
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

# Get the DLL path based on the build configuration
# We use generator expressions to select the correct path (.dll) at build time
set(ASSIMP_DLL_PATH_DEBUG "${VENDOR_DIR}/assimp/build/bin/Debug/assimp-vc143-mtd.dll")
set(ASSIMP_DLL_PATH_RELEASE "${VENDOR_DIR}/assimp/build/bin/Release/assimp-vc143-mt.dll")

# Madman91/BVH - Bounding Volume Hierarchy library for fast spatial queries
add_subdirectory(${VENDOR_DIR}/bvh)

#jblancoc/nanoflann - KD-Tree library for Nearest Neighbor search
set(NANOFLANN_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(NANOFLANN_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(${VENDOR_DIR}/nanoflann)


# HDF5
set(HDF5_ROOT "C:/Program Files/HDF_Group/HDF5/2.0.0")
find_package(HDF5 REQUIRED)

# GLFW
add_subdirectory(${VENDOR_DIR}/glfw)

# FMTlib
add_subdirectory(${VENDOR_DIR}/fmt)

# spdlog - Logging library
add_definitions(-DSPDLOG_FMT_EXTERNAL)
add_subdirectory(${VENDOR_DIR}/spdlog)

# TODO : Fix this, we use external fmt loading to avoid using utf-8 charset
target_link_libraries(spdlog PUBLIC fmt)

# Native File Dialog Extended - Cross-platform file dialogs 
# TODO : Build as STATIC library to avoid DLL issues
add_subdirectory(${VENDOR_DIR}/nativefiledialog-extended)

# CTD
#add_subdirectory(${VENDOR_DIR}/CDT/CDT)# In consuming CMakeLists.txt
# THe .lib is located in ${VENDOR_DIR}/CDT/CDT/build/Debug/CDT.lib
# and ${VENDOR_DIR}/CDT/CDT/build/Release/CDT.lib
#add_library(CDT_Static STATIC IMPORTED)
#set_target_properties(CDT_Static PROPERTIES
#        IMPORTED_LOCATION_DEBUG "${VENDOR_DIR}/CDT/CDT/build/Debug/CDT.lib"
#        IMPORTED_LOCATION_RELEASE "${VENDOR_DIR}/CDT/CDT/build/Release/CDT.lib"
#        INTERFACE_INCLUDE_DIRECTORIES "${VENDOR_DIR}/CDT/CDT/include"
#)

# SQLiteCpp
#add_subdirectory(${VENDOR_DIR}/SQLiteCpp)

# ImGui
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
# ITK - Insight Toolkit for Medical Image Processing
# =============================================================================
set(ITK_DIR "${VENDOR_DIR}/ITK/build" CACHE PATH "ITK build directory")
find_package(ITK REQUIRED COMPONENTS
    ITKCommon
    ITKIOImageBase
    ITKIONIFTI
    ITKImageFilterBase
    ITKSmoothing
    ITKThresholding
    ITKConnectedComponents
    ITKMathematicalMorphology
    ITKBinaryMathematicalMorphology
    ITKLabelMap
    ITKWatersheds
    ITKRegionGrowing
)
include(${ITK_USE_FILE})

# =============================================================================
# Executable Target
# =============================================================================

# Collect all source files recursively
file(GLOB_RECURSE PROJECT_SOURCES "${SRC_DIR}/*.cpp")
file(GLOB_RECURSE PROJECT_HEADERS "${SRC_DIR}/*.h")
file(GLOB_RECURSE FORMS "${FORMS_DIR}/*.ui")
qt_wrap_ui(UI_HEADERS ${FORMS})

# Create executable
add_executable( ${PROJECT_NAME}
                ${PROJECT_SOURCES}
                ${IMGUI_SOURCES}
                ${VENDOR_DIR}/glad/src/glad.cpp
                ${UI_HEADERS}
)

# TODO : Eliminiate this hack
set_property(TARGET ${PROJECT_NAME}
        PROPERTY AUTOUIC_SEARCH_PATHS
        ${FORMS_DIR}
)

# Precompiled headers (if you have pch.h in src/)
target_precompile_headers(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR}/pch.h)

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${SRC_DIR}                           # Your source files can now find each other
    ${INCLUDE_DIR}                       # Your own include directory
    
    ${Boost_INCLUDE_DIRS}     

    ${VENDOR_DIR}/glad/include
    ${VENDOR_DIR}/glm
    ${VENDOR_DIR}/imgui
    ${VENDOR_DIR}/implot
    
    ${VENDOR_DIR}/stb #stb_image.h
    ${VENDOR_DIR}/HighFive/include # HDF5 C++ header-only wrapper
    ${VENDOR_DIR}/Eigen/include
    ${VENDOR_DIR}/fmt/include     
    ${VENDOR_DIR}/assimp/include
    ${VENDOR_DIR}/bvh/include
    ${VENDOR_DIR}/nanoflann/include
    ${VENDOR_DIR}/nativefiledialog-extended/src/include

    #${VENDOR_DIR}/entt/src
    #${VENDOR_DIR}/CDT/CDT/include
    #${VENDOR_DIR}/SQLiteCpp/include
)
# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE 
    Qt6::Widgets
    Qt6::OpenGL
    Qt6::Gui
    Qt6::OpenGLWidgets

    OpenGL::GL
    glfw
    
    HDF5::HDF5
    Assimp::Assimp

    spdlog::spdlog
    fmt::fmt

    ${ITK_LIBRARIES}
    
    nfd
    bvh

    $<$<BOOL:${MINGW}>:ws2_32>
    #CDT_Static
    #SQLiteCpp
)

# =============================================================================
# Installation
# =============================================================================
file(GLOB_RECURSE ASSET_SHARE "${ASSET_DIR}/*")

install(FILES ${ASSET_SHARE}
    DESTINATION "share/${PROJECT_NAME}/Assets"
    COMPONENT Assets
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
    COMPONENT Runtime
)


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



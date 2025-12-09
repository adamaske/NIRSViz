cmake_minimum_required(VERSION 3.15)
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
    add_compile_options(/source_charset:utf-8)
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup")

endif()


# =============================================================================
# External Dependencies
# =============================================================================

#Boost
#find_package(Boost REQUIRED COMPONENTS filesystem system)

#Assimp 

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



# FMTlib
add_subdirectory(${VENDOR_DIR}/fmt)

# OpenGL
find_package(OpenGL REQUIRED)

# HDF5
set(HDF5_ROOT "C:/Program Files/HDF_Group/HDF5/1.14.6")
find_package(HDF5 REQUIRED)

# GLFW
add_subdirectory(${VENDOR_DIR}/glfw)

# SQLiteCpp
add_subdirectory(${VENDOR_DIR}/SQLiteCpp)

# spdlog
add_subdirectory(${VENDOR_DIR}/spdlog)

# Native File Dialog Extended
add_subdirectory(${VENDOR_DIR}/nativefiledialog-extended)

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
)

# Link libraries
target_link_libraries(${PROJECT_NAME} PRIVATE
    OpenGL::GL
    glfw
    spdlog::spdlog
    HDF5::HDF5
    nfd
    SQLiteCpp
    $<$<BOOL:${MINGW}>:ws2_32>
    fmt
    Assimp::Assimp
)

# =============================================================================
# Installation
# =============================================================================
file(GLOB_RECURSE DATA_SHARE "${DATA_DIR}/*")
file(GLOB_RECURSE ASSET_SHARE "${ASSET_DIR}/*")

install(FILES ${DATA_SHARE}
    DESTINATION "share/${PROJECT_NAME}/Data"
)

install(FILES ${ASSET_SHARE}
    DESTINATION "share/${PROJECT_NAME}/Assets"  # Change destination for asset share
)

install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin
)

# Use a POST_BUILD command to copy the DLL after the main executable is built.
add_custom_command(
    TARGET ${PROJECT_NAME} POST_BUILD
    # Generator expression to select the correct DLL path based on configuration
    COMMAND ${CMAKE_COMMAND} -E copy
        "$<IF:$<CONFIG:Debug>,${ASSIMP_DLL_PATH_DEBUG},${ASSIMP_DLL_PATH_RELEASE}>"
        "$<TARGET_FILE_DIR:${PROJECT_NAME}>"  # Updated to use ${PROJECT_NAME} for target directory
    COMMENT "Copying Assimp DLL to executable output directory"
)
cmake_minimum_required(VERSION 3.15)

project(NIRSViz VERSION 1.0)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

set(VENDOR_DIR "${CMAKE_CURRENT_SOURCE_DIR}/vendor")
set(INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/include")
set(SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/src")


find_package( OpenGL REQUIRED )
include_directories( ${OPENGL_INCLUDE_DIRS} )

include_directories("${GLFW_SOURCE_DIR}/deps")
    set( GLAD_GL "${GLFW_SOURCE_DIR}/deps/glad/gl.h" )
if(MSVC)
    add_compile_options("/utf-8")
    SET( CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} /ENTRY:mainCRTStartup" )
endif()
add_subdirectory( vendor/glfw )

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

set(HDF5_ROOT "C:/Program Files/HDF_Group/HDF5/1.14.6")
find_package(HDF5 REQUIRED)

add_subdirectory(${VENDOR_DIR}/nativefiledialog-extended)
add_subdirectory( ${VENDOR_DIR}/spdlog)

set(DATA_DIR "${CMAKE_CURRENT_SOURCE_DIR}/data")
file(GLOB_RECURSE DATA_SHARE "${DATA_DIR}/*")
set(INSTALL_FILES ${DATA_SHARE})

install(
    FILES ${INSTALL_FILES}
    DESTINATION "share/my_project_name/data"
)


file(GLOB_RECURSE INCS "${INCLUDE_DIR}/*.h")
file(GLOB_RECURSE SRCS "${SOURCE_DIR}/*.cpp")

add_executable( ${PROJECT_NAME} 
                ${SRCS} 
                ${IMGUI_SOURCES} 
                ${VENDOR_DIR}/glad/src/glad.cpp
)

target_precompile_headers(${PROJECT_NAME} PRIVATE ${INCLUDE_DIR}/pch.h)

target_compile_options(${PROJECT_NAME} PRIVATE
    $<$<CXX_COMPILER_ID:MSVC>:/utf-8>
)


target_include_directories(${PROJECT_NAME} PUBLIC
    ${INCLUDE_DIR}
    ${VENDOR_DIR}/glfw/include
    ${VENDOR_DIR}/spdlog/include
    ${VENDOR_DIR}/glad/include
    ${VENDOR_DIR}/glm
    ${VENDOR_DIR}/imgui 
    ${VENDOR_DIR}/implot
    ${VENDOR_DIR}/HighFive/include
    ${VENDOR_DIR}/Eigen/include 
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    ${OPENGL_LIBRARIES} 
    glfw 
    $<$<BOOL:${MINGW}>:ws2_32> 
    spdlog::spdlog 
    HDF5::HDF5
    nfd
)
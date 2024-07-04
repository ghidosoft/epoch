include(FetchContent)

FetchContent_Declare(
    glfw
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://github.com/glfw/glfw/archive/3.4.zip
)
set(GLFW_BUILD_EXAMPLES OFF CACHE INTERNAL "Build the GLFW example programs")
set(GLFW_BUILD_TESTS OFF CACHE INTERNAL "Build the GLFW test programs")
set(GLFW_BUILD_DOCS OFF CACHE INTERNAL "Build the GLFW documentation")
set(GLFW_INSTALL OFF CACHE INTERNAL "Generate installation target")
FetchContent_MakeAvailable(glfw)

add_subdirectory(glad)

FetchContent_Declare(
    googletest
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://github.com/google/googletest/archive/refs/tags/v1.13.0.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

FetchContent_Declare(
    imgui
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://github.com/ocornut/imgui/archive/v1.90.9-docking.zip
)
FetchContent_GetProperties(imgui)
if(NOT imgui_POPULATED)
    FetchContent_Populate(imgui)
    set(IMGUI_INCLUDE_DIR ${imgui_SOURCE_DIR}/)
    file(GLOB IMGUI_SOURCES ${imgui_SOURCE_DIR}/*.cpp)
    file(GLOB IMGUI_HEADERS ${imgui_SOURCE_DIR}/*.h)
    add_library(imgui STATIC ${IMGUI_SOURCES} ${IMGUI_SOURCES})
    target_include_directories(imgui PUBLIC ${IMGUI_INCLUDE_DIR})
endif()

FetchContent_Declare(
    ImGuiFileDialog
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://github.com/aiekick/ImGuiFileDialog/archive/v0.6.7.zip
)
FetchContent_MakeAvailable(ImGuiFileDialog)
target_link_libraries(ImGuiFileDialog PUBLIC imgui)

FetchContent_Declare(
    portaudio
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://github.com/PortAudio/portaudio/archive/v19.7.0.zip
)
set(PA_BUILD_SHARED OFF CACHE BOOL "Build PortAudio shared library" FORCE)
FetchContent_MakeAvailable(portaudio)

FetchContent_Declare(
    yaml-cpp
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://github.com/jbeder/yaml-cpp/archive/0.8.0.zip
)
set(YAML_CPP_BUILD_TOOLS OFF CACHE INTERNAL "Enable parse tools")
FetchContent_MakeAvailable(yaml-cpp)

FetchContent_Declare(
    nlohmann_json
    DOWNLOAD_EXTRACT_TIMESTAMP TRUE
    URL https://github.com/nlohmann/json/releases/download/v3.11.3/json.tar.xz
)
FetchContent_MakeAvailable(nlohmann_json)

add_library(epoch_frontend
    "include/epoch/frontend.hpp"

    "src/Application.cpp" "src/Application.hpp"
    "src/Audio.cpp" "src/Audio.hpp"
    "src/CircularBuffer.hpp"
    "src/ConfigurableShader.cpp" "src/ConfigurableShader.hpp"
    "src/GraphicContext.cpp" "src/GraphicContext.hpp"
    "src/Gui.cpp" "src/Gui.hpp"
    "src/Settings.cpp" "src/Settings.hpp"
    "src/SettingsManager.cpp" "src/SettingsManager.hpp"
    "src/Shader.cpp" "src/Shader.hpp"
    "src/Shaders.cpp" "src/Shaders.hpp"
    "src/Window.cpp" "src/Window.hpp"
)

target_include_directories(epoch_frontend INTERFACE include)

target_link_libraries(epoch_frontend PRIVATE glad glfw ${GLFW_LIBRARIES} imgui ImGuiFileDialog portaudio_static yaml-cpp::yaml-cpp Epoch::Core)

add_library(Epoch::Frontend ALIAS epoch_frontend)

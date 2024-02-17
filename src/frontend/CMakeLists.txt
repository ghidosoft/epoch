add_library(epoch_frontend
    "include/epoch/frontend.hpp"

    "src/Application.cpp" "src/Application.hpp"
    "src/Audio.cpp" "src/Audio.hpp"
    "src/CircularBuffer.hpp"
    "src/ConfigurableShader.cpp" "src/ConfigurableShader.hpp"
    "src/GraphicContext.cpp" "src/GraphicContext.hpp"
    "src/Gui.cpp" "src/Gui.hpp"
    "src/Platform.cpp" "src/Platform.hpp"
    "src/Shader.cpp" "src/Shader.hpp"
    "src/Window.cpp" "src/Window.hpp"
)

target_include_directories(epoch_frontend INTERFACE include)

target_link_libraries(epoch_frontend PRIVATE glad glfw ${GLFW_LIBRARIES} imgui nfd portaudio_static Epoch::Core)

add_library(Epoch::Frontend ALIAS epoch_frontend)

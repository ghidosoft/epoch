add_library(epoch_frontend
    "include/epoch/frontend.h"

    "src/Application.cpp" "src/Application.h"
    "src/Audio.cpp" "src/Audio.h"
    "src/CircularBuffer.h"
    "src/GraphicContext.cpp" "src/GraphicContext.h"
    "src/Gui.cpp" "src/Gui.h"
    "src/Platform.cpp" "src/Platform.h"
    "src/Shader.cpp" "src/Shader.h"
    "src/Window.cpp" "src/Window.h"
)

target_include_directories(epoch_frontend INTERFACE include)

target_link_libraries(epoch_frontend PRIVATE glad glfw ${GLFW_LIBRARIES} imgui nfd portaudio_static Epoch::Core)

add_library(Epoch::Frontend ALIAS epoch_frontend)

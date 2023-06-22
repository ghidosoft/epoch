#ifndef SRC_FRONTEND_GRAPHICCONTEXT_H_
#define SRC_FRONTEND_GRAPHICCONTEXT_H_

#include <memory>
#include <glad/glad.h>

namespace epoch::frontend
{
    class Shader;

    class GraphicContext final
    {
    public:
        GraphicContext();
        ~GraphicContext();

        void init(float screenWidth, float screenHeight);
        void renderScreen();

    private:
        GLuint m_vao{};
        GLuint m_vertexBuffer{}, m_indexBuffer{};
        GLuint m_screenTexture{};
        std::unique_ptr<Shader> m_shader{};
    };
}

#endif

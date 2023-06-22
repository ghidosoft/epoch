#ifndef SRC_FRONTEND_GRAPHICCONTEXT_H_
#define SRC_FRONTEND_GRAPHICCONTEXT_H_

#include <glad/glad.h>

namespace epoch::frontend
{
    class GraphicContext final
    {
    public:
        GraphicContext();
        ~GraphicContext();

        void init(int screenWidth, int screenHeight);
        void renderScreen();

    private:
        GLuint m_vao{};
        GLuint m_vertexBuffer{}, m_indexBuffer{};
        GLuint m_screenTexture{};
    };
}

#endif

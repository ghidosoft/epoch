#ifndef SRC_FRONTEND_GRAPHICCONTEXT_H_
#define SRC_FRONTEND_GRAPHICCONTEXT_H_

#include <cstdint>
#include <memory>
#include <span>
#include <glad/glad.h>

namespace epoch::frontend
{
    class Shader;

    class GraphicContext final
    {
    public:
        GraphicContext();
        ~GraphicContext();

    public:
        GraphicContext(const GraphicContext& other) = delete;
        GraphicContext(GraphicContext&& other) noexcept = delete;
        GraphicContext& operator=(const GraphicContext& other) = delete;
        GraphicContext& operator=(GraphicContext&& other) noexcept = delete;

    public:
        void init(int screenWidth, int screenHeight);
        void updateScreen(std::span<const uint32_t> buffer);
        void renderScreen();

    private:
        int m_screenWidth{}, m_screenHeight{};
        int m_screenTextureWidth{}, m_screenTextureHeight{};

        GLuint m_vao{};
        GLuint m_vertexBuffer{}, m_indexBuffer{};
        GLuint m_screenTexture{};
        std::unique_ptr<Shader> m_shader{};
    };
}

#endif

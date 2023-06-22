#include "GraphicContext.h"

#include <cstdint>

namespace epoch::frontend
{
    struct Vertex
    {
        float position[2];
        float uv[2];
    };

    GraphicContext::GraphicContext()
    {
        glGenVertexArrays(1, &m_vao);
        glGenBuffers(1, &m_vertexBuffer);
        glGenBuffers(1, &m_indexBuffer);
        glGenTextures(1, &m_screenTexture);

        glBindTexture(GL_TEXTURE_2D, m_screenTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        // TODO: shader
    }

    GraphicContext::~GraphicContext()
    {
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        if (m_vertexBuffer) glDeleteBuffers(1, &m_vertexBuffer);
        if (m_indexBuffer) glDeleteBuffers(1, &m_indexBuffer);
        if (m_screenTexture) glDeleteTextures(1, &m_screenTexture);
    }

    void GraphicContext::init(const int screenWidth, const int screenHeight)
    {
        glClearColor(0.f, 0.f, 0.f, 0.f);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        const Vertex quadVertices[] = {
            { {-1, -1}, {0.f, static_cast<float>(screenWidth) / 512.f} },
            { { 1, -1}, {1.f, static_cast<float>(screenHeight) / 512.f} },
            { { 1,  1}, {1.f, 0.f} },
            { {-1,  1}, {0.f, 0.f} },
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        const uint8_t quadIndices[] = {
            0, 1, 3,
            1, 2, 3,
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void GraphicContext::renderScreen()
    {
        glDisable(GL_BLEND);
        glDisable(GL_SCISSOR_TEST);

        glClear(GL_COLOR_BUFFER_BIT);

        glBindVertexArray(m_vao);
        // TODO: bind shader
        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_screenTexture);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
    }
}

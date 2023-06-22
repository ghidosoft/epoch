#include "GraphicContext.h"

#include <cassert>
#include <cstdint>

#include "Shader.h"

static const char* VERTEX_SHADER = R"GLSL(
#version 330 core

layout (location = 0) in vec2 inPos;
layout (location = 1) in vec2 inTexCoords;

out vec2 passTexCoords;

void main()
{
    gl_Position = vec4(inPos.x, inPos.y, 0.0, 1.0);
    passTexCoords = inTexCoords;
}
)GLSL";

static const char* FRAGMENT_SHADER = R"GLSL(
#version 330 core

uniform sampler2D mainTexture;

in vec2 passTexCoords;

out vec4 outFragColor;

void main()
{
    outFragColor = texture(mainTexture, passTexCoords);
}
)GLSL";

namespace epoch::frontend
{
    struct Vertex
    {
        float position[2];
        float uv[2];
    };

    GraphicContext::GraphicContext()
    {
        glClearColor(0.f, 0.f, 0.f, 0.f);

        glGenVertexArrays(1, &m_vao);
        assert(m_vao);
        glGenBuffers(1, &m_vertexBuffer);
        assert(m_vertexBuffer);
        glGenBuffers(1, &m_indexBuffer);
        assert(m_indexBuffer);
        glGenTextures(1, &m_screenTexture);
        assert(m_screenTexture);

        glBindTexture(GL_TEXTURE_2D, m_screenTexture);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glBindTexture(GL_TEXTURE_2D, 0);

        m_shader = std::make_unique<Shader>(VERTEX_SHADER, FRAGMENT_SHADER);
        m_shader->bind();
        m_shader->setUniformTexture("mainTexture", 0);
    }

    GraphicContext::~GraphicContext()
    {
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        if (m_vertexBuffer) glDeleteBuffers(1, &m_vertexBuffer);
        if (m_indexBuffer) glDeleteBuffers(1, &m_indexBuffer);
        if (m_screenTexture) glDeleteTextures(1, &m_screenTexture);
        m_shader = nullptr;
    }

    void GraphicContext::init(const int screenWidth, const int screenHeight)
    {
        assert(screenWidth > 0);
        assert(screenHeight > 0);
        m_screenWidth = screenWidth;
        m_screenHeight = screenHeight;

        // TODO: calculate from screen size
        m_screenTextureWidth = 512;
        m_screenTextureHeight = 512;

        glBindTexture(GL_TEXTURE_2D, m_screenTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, m_screenTextureWidth, m_screenTextureHeight,
            0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        const Vertex quadVertices[] = {
            { {-1, -1}, {0.f, static_cast<float>(screenHeight) / static_cast<float>(m_screenTextureHeight)} },
            { { 1, -1}, {static_cast<float>(screenWidth) / static_cast<float>(m_screenTextureWidth), static_cast<float>(screenHeight) / static_cast<float>(m_screenTextureHeight)} },
            { { 1,  1}, {static_cast<float>(screenWidth) / static_cast<float>(m_screenTextureWidth), 0.f} },
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

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void GraphicContext::updateScreen(const std::span<const uint8_t> buffer)
    {
        assert(buffer.size() == static_cast<std::size_t>(m_screenWidth) * m_screenHeight * 4);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_screenTexture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, m_screenWidth, m_screenHeight, GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
        glBindTexture(GL_TEXTURE_2D, 0);
    }

    void GraphicContext::renderScreen()
    {
        glDisable(GL_BLEND);
        glDisable(GL_SCISSOR_TEST);

        glClear(GL_COLOR_BUFFER_BIT);

        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_screenTexture);

        glBindVertexArray(m_vao);
        m_shader->bind();
        // glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        glBindVertexArray(0);
    }
}

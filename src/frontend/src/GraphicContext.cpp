/* This file is part of Epoch, Copyright (C) 2023 Andrea Ghidini.
 *
 * Epoch is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Epoch is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Epoch.  If not, see <https://www.gnu.org/licenses/>.
 */

#include "GraphicContext.hpp"

#include <bit>
#include <cassert>
#include <cstdint>

#include "Shader.hpp"

#include "ConfigurableShader.hpp"

namespace epoch::frontend
{
    struct Vertex
    {
        float position[4];
        float color[4];
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
    }

    GraphicContext::~GraphicContext()
    {
        if (m_vao) glDeleteVertexArrays(1, &m_vao);
        if (m_vertexBuffer) glDeleteBuffers(1, &m_vertexBuffer);
        if (m_indexBuffer) glDeleteBuffers(1, &m_indexBuffer);
        if (m_screenTexture) glDeleteTextures(1, &m_screenTexture);
        m_shader = nullptr;
    }

    void GraphicContext::init(const unsigned screenWidth, const unsigned screenHeight)
    {
        assert(screenWidth > 0);
        assert(screenHeight > 0);
        m_screenWidth = screenWidth;
        m_screenHeight = screenHeight;

        m_screenTextureWidth = std::bit_ceil(m_screenWidth);
        m_screenTextureHeight = std::bit_ceil(m_screenHeight);

        glBindTexture(GL_TEXTURE_2D, m_screenTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
            static_cast<GLint>(m_screenTextureWidth), static_cast<GLint>(m_screenTextureHeight),
            0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
        glBindTexture(GL_TEXTURE_2D, 0);

        glBindVertexArray(m_vao);

        glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        constexpr auto uMin = .0f;
        constexpr auto vMin = .0f;
        const auto uMax = static_cast<float>(screenWidth) / static_cast<float>(m_screenTextureWidth);
        const auto vMax = static_cast<float>(screenHeight) / static_cast<float>(m_screenTextureHeight);
        const Vertex quadVertices[] = {
            { {-1, -1, 0, 1}, {1, 1, 1, 1}, {uMin, vMax} },
            { { 1, -1, 0, 1}, {1, 1, 1, 1}, {uMax, vMax} },
            { { 1,  1, 0, 1}, {1, 1, 1, 1}, {uMax, vMin} },
            { {-1,  1, 0, 1}, {1, 1, 1, 1}, {uMin, vMin} },
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        const uint8_t quadIndices[] = {
            0, 1, 3,
            1, 2, 3,
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 4, GL_FLOAT, GL_FALSE, 10 * sizeof(float), reinterpret_cast<void*>(4 * sizeof(float)));
        glEnableVertexAttribArray(1);
        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 10 * sizeof(float), reinterpret_cast<void*>(8 * sizeof(float)));
        glEnableVertexAttribArray(2);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    void GraphicContext::updateScreen(const std::span<const uint32_t> buffer)
    {
        assert(buffer.size() == static_cast<std::size_t>(m_screenWidth) * m_screenHeight);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, m_screenTexture);
        glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, static_cast<GLsizei>(m_screenWidth), static_cast<GLsizei>(m_screenHeight), GL_RGBA, GL_UNSIGNED_BYTE, buffer.data());
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
        m_shader->setUniformInt("FrameCount", m_frameCount);
        // glBindBuffer(GL_ARRAY_BUFFER, m_vertexBuffer);
        // glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_BYTE, 0);
        glBindTexture(GL_TEXTURE_2D, 0);
        glUseProgram(0);
        glBindVertexArray(0);

        m_frameCount++;
    }

    void GraphicContext::viewport(const int x, const int y, const int width, const int height)
    {
        m_viewportWidth = width;
        m_viewportHeight = height;
        glViewport(x, y, width, height);

        m_shader->bind();
        float vec2[2];
        vec2[0] = static_cast<float>(m_viewportWidth);
        vec2[1] = static_cast<float>(m_viewportHeight);
        m_shader->setUniformVec2("OutputSize", vec2);
    }

    void GraphicContext::updateShader(ConfigurableShader &configurableShader)
    {
        m_shader = nullptr;

        std::string vertexSource = "#version 330 core\n#define VERTEX\n\n" + configurableShader.source();
        std::string fragmentSource = "#version 330 core\n#define FRAGMENT\n\n" + configurableShader.source();

        m_shader = std::make_unique<Shader>(vertexSource, fragmentSource);
        m_shader->bind();
        m_shader->setUniformTexture("MainTexture", 0);
        float vec2[2];
        vec2[0] = static_cast<float>(m_screenWidth);
        vec2[1] = static_cast<float>(m_screenHeight);
        m_shader->setUniformVec2("InputSize", vec2);
        vec2[0] = static_cast<float>(m_screenTextureWidth);
        vec2[1] = static_cast<float>(m_screenTextureHeight);
        m_shader->setUniformVec2("TextureSize", vec2);
        vec2[0] = static_cast<float>(m_viewportWidth);
        vec2[1] = static_cast<float>(m_viewportHeight);
        m_shader->setUniformVec2("OutputSize", vec2);
        float mat4x4[16] = {
            1, 0, 0, 0,
            0, 1, 0, 0,
            0, 0, 1, 0,
            0, 0, 0, 1,
        };
        m_shader->setUniformMat4("MVPMatrix", mat4x4);
    }
}

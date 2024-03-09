/* This file is part of Epoch, Copyright (C) 2024 Andrea Ghidini.
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

#ifndef SRC_FRONTEND_GRAPHICCONTEXT_HPP_
#define SRC_FRONTEND_GRAPHICCONTEXT_HPP_

#include <cstdint>
#include <memory>
#include <span>

#include <glad/glad.h>

namespace epoch::frontend
{
    class ConfigurableShader;
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
        void init(unsigned screenWidth, unsigned screenHeight);
        void updateScreen(std::span<const uint32_t> buffer);
        void renderScreen();
        void viewport(int x, int y, int width, int height);

        void updateShader(ConfigurableShader& configurableShader);
        void updateShaderParameters(ConfigurableShader& configurableShader);

    private:
        unsigned m_screenWidth{}, m_screenHeight{};
        unsigned m_screenTextureWidth{}, m_screenTextureHeight{};
        int m_viewportWidth{}, m_viewportHeight{};
        int m_frameCount{};

        GLuint m_vao{};
        GLuint m_vertexBuffer{}, m_indexBuffer{};
        GLuint m_screenTexture{};
        std::unique_ptr<Shader> m_shader{};
    };
}  // namespace epoch::frontend

#endif

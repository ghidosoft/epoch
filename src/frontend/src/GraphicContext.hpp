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

#ifndef SRC_FRONTEND_GRAPHICCONTEXT_HPP_
#define SRC_FRONTEND_GRAPHICCONTEXT_HPP_

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
        void viewport(int width, int height);

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

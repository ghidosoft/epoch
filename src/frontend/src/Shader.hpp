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

#ifndef SRC_FRONTEND_SHADER_HPP_
#define SRC_FRONTEND_SHADER_HPP_

#include <glad/glad.h>

#include <map>
#include <string>
#include <string_view>

namespace epoch::frontend
{
    class ShaderModule final
    {
    public:
        ShaderModule(GLenum shaderType, std::string_view source);
        ~ShaderModule();

    public:
        ShaderModule(const ShaderModule& other) = delete;
        ShaderModule(ShaderModule&& other) noexcept = delete;
        ShaderModule& operator=(const ShaderModule& other) = delete;
        ShaderModule& operator=(ShaderModule&& other) noexcept = delete;

    public:
        [[nodiscard]] GLuint handle() const;

    private:
        GLuint m_handle{};

        void compile(std::string_view source) const;
    };

    class Shader final
    {
    public:
        Shader(std::string_view vertex, std::string_view fragment);
        ~Shader();

    public:
        Shader(const Shader& other) = delete;
        Shader(Shader&& other) noexcept = delete;
        Shader& operator=(const Shader& other) = delete;
        Shader& operator=(Shader&& other) noexcept = delete;

    public:
        void bind() const;

        void setUniformFloat(const std::string& name, GLfloat data);
        void setUniformInt(const std::string& name, GLint data);
        void setUniformMat4(const std::string& name, const GLfloat* data);
        void setUniformTexture(const std::string& name, GLint data);
        void setUniformVec2(const std::string& name, const GLfloat* data);
        void setUniformVec3(const std::string& name, const GLfloat* data);
        void setUniformVec4(const std::string& name, const GLfloat* data);

    private:
        GLuint m_handle{};

        std::map<std::string, GLint> m_uniformLocations{};

        GLint getUniformLocation(const std::string& name);

        void link(const ShaderModule& shaderModule1, const ShaderModule& shaderModule2) const;
    };
}  // namespace epoch::frontend

#endif

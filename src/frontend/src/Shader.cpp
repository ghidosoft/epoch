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

#include "Shader.hpp"

#include <cassert>
#include <sstream>
#include <stdexcept>

namespace epoch::frontend
{
    ShaderModule::ShaderModule(const GLenum shaderType, const std::string_view source)
    {
        m_handle = glCreateShader(shaderType);
        assert(m_handle);
        compile(source);
    }

    ShaderModule::~ShaderModule()
    {
        if (m_handle)
        {
            glDeleteShader(m_handle);
        }
    }

    GLuint ShaderModule::handle() const { return m_handle; }

    void ShaderModule::compile(const std::string_view source) const
    {
        const auto sourcePtr = source.data();
        glShaderSource(m_handle, 1, &sourcePtr, nullptr);
        glCompileShader(m_handle);

        GLint success;
        glGetShaderiv(m_handle, GL_COMPILE_STATUS, &success);

        std::string infoLog;
        GLint infoLen = 0;
        glGetShaderiv(m_handle, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0)
        {
            infoLog.resize(infoLen);
            glGetShaderInfoLog(m_handle, infoLen, nullptr, infoLog.data());
        }
        else
        {
            infoLog = "<NO LOG>";
        }

        if (!success)
        {
            std::stringstream ss;
            ss << "Error while compiling shader: " << infoLog;
            throw std::runtime_error(ss.str());
        }
    }

    Shader::Shader(const std::string_view vertex, const std::string_view fragment)
    {
        m_handle = glCreateProgram();
        assert(m_handle);
        const ShaderModule vertexModule{GL_VERTEX_SHADER, vertex};
        const ShaderModule fragmentModule{GL_FRAGMENT_SHADER, fragment};
        link(vertexModule, fragmentModule);
    }

    Shader::~Shader()
    {
        if (m_handle)
        {
            glDeleteProgram(m_handle);
        }
    }

    void Shader::bind() const { glUseProgram(m_handle); }

    void Shader::setUniformFloat(const std::string& name, const GLfloat data)
    {
        const auto location = getUniformLocation(name);
        glUniform1f(location, data);
    }

    void Shader::setUniformInt(const std::string& name, const GLint data)
    {
        const auto location = getUniformLocation(name);
        glUniform1i(location, data);
    }

    void Shader::setUniformMat4(const std::string& name, const GLfloat* data)
    {
        const auto location = getUniformLocation(name);
        glUniformMatrix4fv(location, 1, GL_FALSE, data);
    }

    void Shader::setUniformTexture(const std::string& name, const GLint data)
    {
        const auto location = getUniformLocation(name);
        glUniform1i(location, data);
    }

    void Shader::setUniformVec2(const std::string& name, const GLfloat* data)
    {
        const auto location = getUniformLocation(name);
        glUniform2fv(location, 1, data);
    }

    void Shader::setUniformVec3(const std::string& name, const GLfloat* data)
    {
        const auto location = getUniformLocation(name);
        glUniform3fv(location, 1, data);
    }

    void Shader::setUniformVec4(const std::string& name, const GLfloat* data)
    {
        const auto location = getUniformLocation(name);
        glUniform4fv(location, 1, data);
    }

    GLint Shader::getUniformLocation(const std::string& name)
    {
        const auto i(m_uniformLocations.lower_bound(name));
        if (i == m_uniformLocations.end() || name < i->first)
        {
            auto value = glGetUniformLocation(m_handle, name.data());
            m_uniformLocations.insert(i, std::make_pair(name, value));
            return value;
        }
        else
        {
            return i->second;
        }
    }

    void Shader::link(const ShaderModule& shaderModule1, const ShaderModule& shaderModule2) const
    {
        glAttachShader(m_handle, shaderModule1.handle());
        glAttachShader(m_handle, shaderModule2.handle());
        glLinkProgram(m_handle);

        GLint success;
        glGetProgramiv(m_handle, GL_LINK_STATUS, &success);
        std::string infoLog;
        GLint infoLen = 0;
        glGetProgramiv(m_handle, GL_INFO_LOG_LENGTH, &infoLen);
        if (infoLen > 0)
        {
            infoLog.resize(infoLen);
            glGetProgramInfoLog(m_handle, infoLen, nullptr, infoLog.data());
        }
        else
        {
            infoLog = "<NO LOG>";
        }

        if (!success)
        {
            std::stringstream ss;
            ss << "Error while linking shader: " << infoLog;
            throw std::runtime_error(ss.str());
        }
    }
}  // namespace epoch::frontend

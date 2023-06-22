#include "Shader.h"

#include <sstream>
#include <stdexcept>

namespace epoch::frontend
{
    ShaderModule::ShaderModule(const GLenum shaderType, const std::string_view source)
    {
        m_handle = glCreateShader(shaderType);
        compile(source);
    }

    ShaderModule::~ShaderModule()
    {
        if (m_handle)
        {
            glDeleteShader(m_handle);
        }
    }

    GLuint ShaderModule::handle() const
    {
        return m_handle;
    }

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
        const ShaderModule vertexModule{ GL_VERTEX_SHADER, vertex };
        const ShaderModule fragmentModule{ GL_FRAGMENT_SHADER, fragment };
        link(vertexModule, fragmentModule);
    }

    Shader::~Shader()
    {
        if (m_handle)
        {
            glDeleteProgram(m_handle);
        }
    }

    void Shader::bind() const
    {
        glUseProgram(m_handle);
    }

    void Shader::setUniformTexture(const std::string& name, const GLint data)
    {
        const auto location = getUniformLocation(name);
        glUniform1i(location, data);
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
        else {
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
}

#ifndef SRC_FRONTEND_SHADER_H_
#define SRC_FRONTEND_SHADER_H_

#include <map>
#include <string_view>

#include <glad/glad.h>

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

        void setUniformTexture(const std::string& name, GLint data);

    private:
        GLuint m_handle{};

        std::map<std::string, GLint> m_uniformLocations{};

        GLint getUniformLocation(const std::string& name);

        void link(const ShaderModule& shaderModule1, const ShaderModule& shaderModule2) const;
    };
}

#endif

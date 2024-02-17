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

// Shader ported from https://github.com/libretro/glsl-shaders/blob/master/crt/shaders/crt-easymode.glsl

#define SHARPNESS_H 0.5
#define SHARPNESS_V 1.0
#define MASK_STRENGTH 0.3
#define MASK_DOT_WIDTH 1.0
#define MASK_DOT_HEIGHT 1.0
#define MASK_STAGGER 0.0
#define MASK_SIZE 1.0
#define SCANLINE_STRENGTH 1.0
#define SCANLINE_BEAM_WIDTH_MIN 1.5
#define SCANLINE_BEAM_WIDTH_MAX 1.5
#define SCANLINE_BRIGHT_MIN 0.35
#define SCANLINE_BRIGHT_MAX 0.65
#define SCANLINE_CUTOFF 400.0
#define GAMMA_INPUT 2.0
#define GAMMA_OUTPUT 1.8
#define BRIGHT_BOOST 1.2
#define DILATION 1.0

#define ENABLE_LANCZOS 1

in vec2 passTexCoords;

out vec4 outFragColor;

uniform sampler2D MainTexture;
uniform vec2 InputSize;
uniform vec2 TextureSize;
uniform vec2 OutputSize;

#define FIX(c) max(abs(c), 1e-5)
#define PI 3.141592653589
#define TEX2D(c) dilate(texture(MainTexture, c))
#define SourceSize vec4(TextureSize, 1.0 / TextureSize)
#define outsize vec4(OutputSize, 1.0 / OutputSize)

vec4 dilate(vec4 col)
{
    vec4 x = mix(vec4(1.0), col, DILATION);
    return col * x;
}

float curve_distance(float x, float sharp)
{
    float x_step = step(0.5, x);
    float curve = 0.5 - sqrt(0.25 - (x - x_step) * (x - x_step)) * sign(0.5 - x);
    return mix(x, curve, sharp);
}

mat4 get_color_matrix(vec2 co, vec2 dx)
{
    return mat4(TEX2D(co - dx), TEX2D(co), TEX2D(co + dx), TEX2D(co + 2.0 * dx));
}

vec3 filter_lanczos(vec4 coeffs, mat4 color_matrix)
{
    vec4 col        = color_matrix * coeffs;
    vec4 sample_min = min(color_matrix[1], color_matrix[2]);
    vec4 sample_max = max(color_matrix[1], color_matrix[2]);

    col = clamp(col, sample_min, sample_max);

    return col.rgb;
}

void main()
{
    vec2 dx     = vec2(SourceSize.z, 0.0);
    vec2 dy     = vec2(0.0, SourceSize.w);
    vec2 pix_co = passTexCoords * SourceSize.xy - vec2(0.5, 0.5);
    vec2 tex_co = (floor(pix_co) + vec2(0.5, 0.5)) * SourceSize.zw;
    vec2 dist   = fract(pix_co);
    float curve_x;
    vec3 col, col2;

#if ENABLE_LANCZOS
    curve_x = curve_distance(dist.x, SHARPNESS_H * SHARPNESS_H);

    vec4 coeffs = PI * vec4(1.0 + curve_x, curve_x, 1.0 - curve_x, 2.0 - curve_x);

    coeffs = FIX(coeffs);
    coeffs = 2.0 * sin(coeffs) * sin(coeffs * 0.5) / (coeffs * coeffs);
    coeffs /= dot(coeffs, vec4(1.0));

    col  = filter_lanczos(coeffs, get_color_matrix(tex_co, dx));
    col2 = filter_lanczos(coeffs, get_color_matrix(tex_co + dy, dx));
#else
    curve_x = curve_distance(dist.x, SHARPNESS_H);

    col  = mix(TEX2D(tex_co).rgb,      TEX2D(tex_co + dx).rgb,      curve_x);
    col2 = mix(TEX2D(tex_co + dy).rgb, TEX2D(tex_co + dx + dy).rgb, curve_x);
#endif

    col = mix(col, col2, curve_distance(dist.y, SHARPNESS_V));
    col = pow(col, vec3(GAMMA_INPUT / (DILATION + 1.0)));

    float luma        = dot(vec3(0.2126, 0.7152, 0.0722), col);
    float bright      = (max(col.r, max(col.g, col.b)) + luma) * 0.5;
    float scan_bright = clamp(bright, SCANLINE_BRIGHT_MIN, SCANLINE_BRIGHT_MAX);
    float scan_beam   = clamp(bright * SCANLINE_BEAM_WIDTH_MAX, SCANLINE_BEAM_WIDTH_MIN, SCANLINE_BEAM_WIDTH_MAX);
    float scan_weight = 1.0 - pow(cos(passTexCoords.y * 2.0 * PI * SourceSize.y) * 0.5 + 0.5, scan_beam) * SCANLINE_STRENGTH;

    float mask   = 1.0 - MASK_STRENGTH;    
    vec2 mod_fac = floor(passTexCoords * outsize.xy * SourceSize.xy / (InputSize.xy * vec2(MASK_SIZE, MASK_DOT_HEIGHT * MASK_SIZE)));
    int dot_no   = int(mod((mod_fac.x + mod(mod_fac.y, 2.0) * MASK_STAGGER) / MASK_DOT_WIDTH, 3.0));
    vec3 mask_weight;

    if      (dot_no == 0) mask_weight = vec3(1.0,  mask, mask);
    else if (dot_no == 1) mask_weight = vec3(mask, 1.0,  mask);
    else                  mask_weight = vec3(mask, mask, 1.0);

    if (InputSize.y >= SCANLINE_CUTOFF) 
        scan_weight = 1.0;

    col2 = col.rgb;
    col *= vec3(scan_weight);
    col  = mix(col, col2, scan_bright);
    col *= mask_weight;
    col  = pow(col, vec3(1.0 / GAMMA_OUTPUT));

    outFragColor = vec4(col * BRIGHT_BOOST, 1.0);
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
        m_shader->setUniformTexture("MainTexture", 0);
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
            { {-1, -1}, {uMin, vMax} },
            { { 1, -1}, {uMax, vMax} },
            { { 1,  1}, {uMax, vMin} },
            { {-1,  1}, {uMin, vMin} },
        };
        glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_indexBuffer);
        const uint8_t quadIndices[] = {
            0, 1, 3,
            1, 2, 3,
        };
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(quadIndices), &quadIndices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), nullptr);
        glEnableVertexAttribArray(0);
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), reinterpret_cast<void*>(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        glBindVertexArray(0);
        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

        m_shader->bind();
        float vec2[2];
        vec2[0] = static_cast<float>(m_screenWidth);
        vec2[1] = static_cast<float>(m_screenHeight);
        m_shader->setUniformVec2("InputSize", vec2);
        vec2[0] = static_cast<float>(m_screenTextureWidth);
        vec2[1] = static_cast<float>(m_screenTextureHeight);
        m_shader->setUniformVec2("TextureSize", vec2);
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
}

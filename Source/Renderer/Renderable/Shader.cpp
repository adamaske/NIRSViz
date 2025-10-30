#include "pch.h"
#include "Renderer/Renderable/Shader.h"

#include "Core/Base.h"

#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

Shader::Shader(const std::string& vertex_path, const std::string& fragment_path)
{
    if (!std::filesystem::exists(vertex_path)) {
        NVIZ_ERROR("Vertex shader file does not exist: {0}", vertex_path.c_str());
        return;
    }
    if (!std::filesystem::exists(fragment_path)) {
        NVIZ_ERROR("Fragment shader file does not exist: {0}", fragment_path.c_str());
        return;
    }
    m_FragmentFilepath = fragment_path;
    m_VertexFilepath = vertex_path;
    std::ifstream vertex_stream(vertex_path);
    std::stringstream vss;
    vss << vertex_stream.rdbuf();

    std::ifstream fragment_stream(fragment_path);
    std::stringstream fss;
    fss << fragment_stream.rdbuf();

    m_RendererID = CreateShader(vss.str(), fss.str());
}

Shader::~Shader()
{
    glDeleteProgram(m_RendererID);
}

unsigned int Shader::CompileShader(unsigned int type, const std::string& source)
{
    unsigned int id = glCreateShader(type);
    const char* src = source.c_str();
    glShaderSource(id, 1, &src, nullptr);
    glCompileShader(id);

    int result;
    glGetShaderiv(id, GL_COMPILE_STATUS, &result);
    if (result == GL_FALSE)
    {
        int length;
        glGetShaderiv(id, GL_INFO_LOG_LENGTH, &length);
        char* message = (char*)_malloca(length * sizeof(char));
        glGetShaderInfoLog(id, length, &length, message); 
        std::string stype = GL_VERTEX_SHADER ? "vertex" : "fragment";
        NVIZ_ERROR("Failed to compile shader  : {}", stype.c_str());
        NVIZ_ERROR("Error message : {}", message);
        glDeleteShader(id);
        return 0;
    }

    return id;
}

int Shader::GetUniformLocation(const std::string& name)
{
    if (m_UniformLocationCache.find(name) != m_UniformLocationCache.end())
        return m_UniformLocationCache[name];

    // If not in the cache, retrieve it and store it.
    int location = glGetUniformLocation(m_RendererID, name.c_str());
    if (location == -1) {
        NVIZ_ERROR("{}: uniform {} doesn't exist!", m_FragmentFilepath, name);
        NVIZ_ERROR("Warning: uniform {} doesn't exist!", name);
    }

    m_UniformLocationCache[name] = location;
    return location;
}

unsigned int Shader::CreateShader(const std::string& vertex_shader, const std::string& fragment_shader)
{
    unsigned int program = glCreateProgram();

    unsigned int vs = CompileShader(GL_VERTEX_SHADER, vertex_shader);
    unsigned int fs = CompileShader(GL_FRAGMENT_SHADER, fragment_shader);

    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    glValidateProgram(program);

    glDeleteShader(vs);
    glDeleteShader(fs);

    return program;
}

void Shader::Bind() const
{
    glUseProgram(m_RendererID);
}

void Shader::Unbind() const
{
    glUseProgram(0);
}

void Shader::SetUniform1i(const std::string& name, int value)
{
    (glUniform1i(GetUniformLocation(name), value));
}

void Shader::SetUniform1f(const std::string& name, float value)
{
    glUniform1f(GetUniformLocation(name), value);
}
void Shader::SetUniform2f(const std::string& name, float x, float y)
{
    glUniform2f(GetUniformLocation(name), x, y);
}
void Shader::SetUniform2f(const std::string& name, glm::vec2 u)
{
    glUniform2f(GetUniformLocation(name), u.x, u.y);
}
void Shader::SetUniform3f(const std::string& name, float v0, float v1, float v2)
{
    glUniform3f(GetUniformLocation(name), v0, v1, v2);
}

void Shader::SetUniform3f(const std::string& name, glm::vec3 xyz)
{
    glUniform3f(GetUniformLocation(name), xyz.x, xyz.y, xyz.z);
}

void Shader::SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3)
{
    glUniform4f(GetUniformLocation(name), v0, v1, v2, v3);
}

void Shader::SetUniform4f(const std::string& name, glm::vec4 u)
{
    glUniform4f(GetUniformLocation(name), u.x, u.y, u.z, u.w);
}

void Shader::SetUniformMat4f(const std::string& name, const glm::mat4& matrix)
{
    glUniformMatrix4fv(GetUniformLocation(name), 1, GL_FALSE, &matrix[0][0]);
}


#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <filesystem>

class Shader {
private:
	unsigned int m_RendererID = 0;
	std::unordered_map<std::string, int> m_UniformLocationCache;

public:
	Shader(const std::string& vertex_path, const std::string& fragment_path);
	~Shader();


	void Bind() const;
	void Unbind() const;


    void SetUniform1i(const std::string& name, int value);
    void SetUniform1f(const std::string& name, float value);
    void SetUniform2f(const std::string& name, float x, float y);
    void SetUniform2f(const std::string& name, glm::vec2 u);
    void SetUniform3f(const std::string& name, float v0, float v1, float v2);
    void SetUniform3f(const std::string& name, glm::vec3 xyz);
    void SetUniform4f(const std::string& name, float v0, float v1, float v2, float v3);
    void SetUniform4f(const std::string& name, glm::vec4 u);
    void SetUniformMat4f(const std::string& name, const glm::mat4& matrix);

private:
    int GetUniformLocation(const std::string& name);

    unsigned int CreateShader(const std::string& vertex_shader, const std::string& fragment_shader);
    unsigned int CompileShader(unsigned int type, const std::string& source);

};
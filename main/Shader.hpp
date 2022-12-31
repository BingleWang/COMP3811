#pragma once

#include <cstdint>
#include <string>

#include <glad.h>

#include "vec2.hpp"
#include "vec3.hpp"
#include "vec4.hpp"

#include "mat44.hpp"

#include <glm/glm.hpp>

enum class ShaderType : int32_t {
	VERTEX = GL_VERTEX_SHADER,
	FRAGMENT = GL_FRAGMENT_SHADER,
	GEOMETRY = GL_GEOMETRY_SHADER,
};

class Shader
{
public:
	Shader(const std::string& inName)
	: name(inName) {
	}

	void create();

	bool compileShaderFromFile(const std::string& fileName, ShaderType type);
	bool compileShaderFromString(const char* source, ShaderType type);

	bool link();

	void use();

	int get() const;

	bool isLinked() const;

	void bindAttribLocation(uint32_t location, const std::string& name);
	void bindFragDataLocation(uint32_t location, const std::string& name);
	void setUniform(const std::string& name, float x, float y, float z);
	void setUniform(const std::string& name, const glm::vec3& v);
	void setUniform(const std::string& name, const glm::vec4& v);
	void setUniform(const std::string& name, const glm::mat3& v);
	void setUniform(const std::string& name, const glm::mat4& v);
	void setUniform(const std::string& name, const Vec3f& v);
	void setUniform(const std::string& name, const Vec4f& v);
	void setUniform(const std::string& name, const Mat44f& v);
	void setUniform(const std::string& name, float value);
	void setUniform(const std::string& name, int32_t value);
	void setUniform(const std::string& name, bool value);

	void printActiveAttributes();
	void printActiveUniforms();

private:

	int32_t getUniformLocation(const std::string& name);
	bool fileExists(const std::string& fileName);

private:

	int32_t program = -1;
	bool linked = false;
	//std::string log = "";
	std::string name;
};


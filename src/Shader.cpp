#include "Shader.h"
#include <gl3w.h>
#include <string>
#include <fstream>
#include <streambuf>
#include <stdexcept>

namespace yamc
{
	uint32_t compileShader(const std::string& path, GLenum type)
	{
		std::ifstream fileStream(path);
		if (!fileStream.is_open()) {
			throw std::runtime_error("Cannot open shader file " + path);
		}

		std::string str;
		fileStream.seekg(0, std::ios::end);
		str.reserve(fileStream.tellg());
		fileStream.seekg(0, std::ios::beg);

		str.assign((std::istreambuf_iterator<char>(fileStream)), std::istreambuf_iterator<char>());

		const char* srcPtr = str.c_str();
		int srcLength = str.length();
		uint32_t id = glCreateShader(type);
		glShaderSource(id, 1, &srcPtr, &srcLength);
		glCompileShader(id);
		
		int isCompiled = 0;
		glGetShaderiv(id, GL_COMPILE_STATUS, &isCompiled);
		if (!isCompiled) {
			int logSize = 0;
			glGetShaderiv(id, GL_INFO_LOG_LENGTH, &logSize);
			std::string log(logSize, '\0');
			glGetShaderInfoLog(id, logSize, &logSize, &log[0]);
			throw std::runtime_error("Cannot compile shader " + path + "\n" + log);
		}

		return id;
	}

	uint32_t linkProgram(uint32_t vertexShaderID, uint32_t fragmentShaderID)
	{
		uint32_t id = glCreateProgram();
		glAttachShader(id, vertexShaderID);
		glAttachShader(id, fragmentShaderID);

		glLinkProgram(id);

		int isLinked = 0;
		glGetProgramiv(id, GL_LINK_STATUS, &isLinked);
		if (!isLinked) {
			int logSize = 0;
			glGetProgramiv(id, GL_INFO_LOG_LENGTH, &logSize);
			std::string log(logSize, '\0');
			glGetProgramInfoLog(id, logSize, &logSize, &log[0]);
			throw std::runtime_error("Cannot link program \n" + log);
		}

		return id;
	}

	Shader::Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath)
	{
		uint32_t vertexShaderID = compileShader(vertexShaderPath, GL_VERTEX_SHADER);
		uint32_t fragmentShaderID = compileShader(fragmentShaderPath, GL_FRAGMENT_SHADER);

		programID = linkProgram(vertexShaderID, fragmentShaderID);

		glDeleteShader(vertexShaderID);
		glDeleteShader(fragmentShaderID);
	}

	Shader::Shader(Shader&& other) noexcept
	{
		programID = other.programID;
		other.programID = 0;
	}

	uint32_t Shader::getID() const
	{
		return programID;
	}

	void Shader::use() const
	{
		glUseProgram(programID);
	}

	Shader::~Shader()
	{
		if (programID > 0) {
			glDeleteProgram(programID);
		}
	}
}


#ifndef YAMC_SHADER_H
#define YAMC_SHADER_H

#include <stdint.h>
#include <string>

namespace yamc
{
	class Shader
	{
	public:
		Shader(const std::string& vertexShaderPath, const std::string& fragmentShaderPath);
		Shader(const Shader&) = delete;
		Shader(Shader&&) noexcept;

		uint32_t getID() const;
		void use() const;

		~Shader();
	private:
		uint32_t programID;
	};
}

#endif
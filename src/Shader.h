#ifndef YAMC_SHADER_H
#define YAMC_SHADER_H

#include <stdint.h>
#include <string>
#include <glm/glm.hpp>

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
		void setMVP(const glm::mat4& mvp) const;
		void setColor(const glm::vec3& color) const;
		void setColor(const glm::vec4& color) const;
		void setTextureOffset(const glm::vec2& offset) const;
		void setTextureScale(const glm::vec2& scale) const;

		~Shader();
	private:
		void setupUniformLocations();

		uint32_t programID;
		int mvpUniformLocation;
		int textureOffsetUniformLocation;
		int textureScaleUniformLocation;
		int colorUniformLocation;
	};
}

#endif
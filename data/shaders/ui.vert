#version 410 core

uniform mat4 mvp;
uniform vec2 textureOffset;
uniform vec2 textureScale;

layout (location = 0) in vec3 position;
layout (location = 1) in vec2 in_uv;

layout (location = 0) out vec2 out_uv;

void main()
{
    gl_Position = mvp * vec4(position, 1.0);
	out_uv = in_uv * textureScale + textureOffset;
}
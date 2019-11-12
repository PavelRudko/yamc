#version 410 core

uniform mat4 mvp;

layout (location = 0) in vec4 position;
layout (location = 1) in vec2 in_uv;

layout (location = 0) out float illuminance;
layout (location = 1) out vec2 out_uv;

void main()
{
    gl_Position = mvp * vec4(position.xyz, 1.0);
	illuminance = position.w;
	out_uv = in_uv;
}
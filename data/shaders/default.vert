#version 150 core

uniform mat4 mvp;
uniform vec2 textureOffset;

in vec4 position;
in vec2 in_uv;

out vec3 frag_uv;

void main()
{
    gl_Position = mvp * vec4(position.xyz, 1.0);
	frag_uv = vec3(in_uv + textureOffset,  position.w);
}
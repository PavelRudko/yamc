#version 150

uniform mat4 mvp;
uniform vec2 textureOffset;
uniform vec2 textureScale;

in vec3 position;
in vec2 in_uv;

out vec2 frag_uv;

void main()
{
    gl_Position = mvp * vec4(position, 1.0);
	frag_uv = in_uv * textureScale + textureOffset;
}
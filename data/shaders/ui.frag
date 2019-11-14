#version 410 core

uniform sampler2D tex;
uniform vec3 color;

layout (location = 0) in vec2 uv;

out vec4 out_color;

void main()
{
    vec4 textureColor = texture(tex, uv);
    if(textureColor.r == 1.0 && textureColor.b == 1.0 && textureColor.g == 0.0) {
	    discard;
	}
    out_color = vec4(textureColor.xyz, 1);
}
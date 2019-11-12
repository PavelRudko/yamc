#version 410 core

uniform sampler2D tex;
uniform vec3 color;

layout (location = 0) in vec2 uv;

out vec4 out_color;

void main()
{
    float alpha = 1.0 - texture(tex, uv).r;
    out_color = vec4(color, alpha);
}
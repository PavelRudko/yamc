#version 410 core

uniform sampler2D tex;

layout (location = 0) in float illuminance;
layout (location = 1) in vec2 uv;

out vec4 out_color;

void main()
{
    vec4 color = texture(tex, uv);
    out_color = vec4(color.xyz * illuminance, 1.0);
}
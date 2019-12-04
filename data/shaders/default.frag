#version 150 core

uniform sampler2D tex;

in vec3 frag_uv;

out vec4 out_color;

void main()
{
    vec4 color = texture(tex, frag_uv.xy);
    out_color = vec4(color.xyz * frag_uv.z, 1.0);
}
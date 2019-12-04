#version 150

uniform sampler2D tex;

in vec3 frag_uv;

out vec4 out_color;

void main()
{
    vec4 color = texture(tex, frag_uv.xy);
    out_color = vec4(color.xyz * frag_uv.z, 1.0);
    //out_color = vec4(1.0, 0.0, 1.0, 1.0);
}

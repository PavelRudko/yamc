#version 150

uniform sampler2D tex;
uniform vec3 color;

in vec2 frag_uv;

out vec4 out_color;

void main()
{
    vec4 textureColor = texture(tex, frag_uv);
    if(textureColor.r == 1.0 && textureColor.b == 1.0 && textureColor.g == 0.0) {
	    discard;
	}
    out_color = vec4(textureColor.xyz * color, 1);
}
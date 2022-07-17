#version 330 core
out vec4 frag_color;
in vec2  texture_coords;

uniform int       texture_index = 0;
uniform sampler2D texture1;
uniform vec4      color;

void main()
{
	vec4 texture_result = texture(texture1, texture_coords);
	if(texture_index == 0)
		frag_color = texture_result.r * color;
	else
		frag_color = texture_result * color;
}
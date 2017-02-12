#version 130

in vec4 coord;

uniform mat4 transform;
uniform mat4 camera;

out vec4 out_coord;

void main()
{
	out_coord = coord;
	gl_Position = camera * vec4(coord.xyz, 1.0f);
}

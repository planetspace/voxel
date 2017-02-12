#version 130

in vec4 out_coord;
uniform sampler2D tex;

const vec4 fog_color = vec4(0.6, 0.8, 1.0, 1.0);
const float fog_density = 0.00003;

void main()
{
	vec4 color;
	float num_tex = 128;
	
	if(out_coord.w < 0)
		color = texture2D(tex, vec2((fract(out_coord.x) + (out_coord.w - 1)) / num_tex, out_coord.z));
	else
		color = texture2D(tex, vec2((fract(out_coord.x + out_coord.z) + (out_coord.w - 1)) / num_tex, -out_coord.y)) * 0.85;
	
	if(color.a < 0.5)
		discard;
	
	float z = gl_FragCoord.z / gl_FragCoord.w;
	float fog = clamp(exp(-fog_density * z * z), 0.2, 1.0);
	
	gl_FragColor = mix(fog_color, color, fog);
}
//color = texture2D(tex, vec2((fract(out_coord.x) + (out_coord.w - 1)) / num_tex, out_coord.z));
//	color = texture2D(tex, vec2((fract(out_coord.x + out_coord.z) + (out_coord.w - 1)) / num_tex, -out_coord.y)) * 0.85;

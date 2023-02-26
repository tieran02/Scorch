#version 450

//shader input
layout (location = 0) in vec2 texCoord;

//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D diffuseTex;

void main()
{
	vec3 color = texture(diffuseTex,texCoord).rgb;
	outFragColor = vec4(color,1.0f);
}

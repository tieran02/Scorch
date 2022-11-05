#version 450

//shader input
layout (location = 0) in vec3 inColor;
layout (location = 1) in vec2 texCoord;

//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 1, binding = 0) uniform sampler2D diffuseTex;
layout(set = 1, binding = 1) uniform sampler2D specularTex;
layout(set = 1, binding = 2) uniform sampler2D normalTex;
layout(set = 1, binding = 3) uniform sampler2D maskTex;

void main()
{
	vec3 color = texture(diffuseTex,texCoord).xyz;
	
	//return color
	outFragColor = vec4(color,1.0f);
}

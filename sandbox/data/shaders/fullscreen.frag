#version 450

//shader input
layout (location = 0) in vec2 texCoord;

//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D diffuseTex;
layout(set = 0, binding = 1) uniform sampler2D normalTex;


void main()
{
	vec3 color = texture(diffuseTex,texCoord).rgb;
	vec3 normalcolor = texture(normalTex,texCoord).rgb;

    vec3 result = color + normalcolor;
	//vec3 result = vec3(0.0,1.0,0.0);

	outFragColor = vec4(result,1.0f);
}

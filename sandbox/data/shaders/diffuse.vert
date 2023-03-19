#version 450

layout (location = 0) in vec3 vPosition;
layout (location = 1) in vec3 vNormal;
layout (location = 2) in vec2 vTexCoord;

layout (location = 0) out vec2 outTexCoord;
layout (location = 1) out vec3 outNormal;
layout (location = 2) out vec3 outFragPos;


//push constants block
layout( push_constant ) uniform constants
{
	vec4 data;
	mat4 render_matrix;
} PushConstants;

struct Light
{
	vec4 position;	//w == 0 pointlight
	vec4 intensities;  //w is intensity
};

layout(set = 1, binding = 0) uniform  SceneBuffer{
	mat4 view;
	vec4 eyePos;
	int lightCount;
	Light lightData[8];
} sceneBuffer;

void main()
{
	gl_Position = sceneBuffer.view * PushConstants.render_matrix * vec4(vPosition, 1.0f);
	outTexCoord = vTexCoord;
	outNormal = mat3(transpose(inverse(PushConstants.render_matrix))) * vNormal;  
	outFragPos = vec3(PushConstants.render_matrix * vec4(vPosition, 1.0));
}

#version 450

//shader input
layout (location = 0) in vec2 texCoord;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec3 inFragPos;

//output write
layout (location = 0) out vec4 outFragColor;

layout(set = 0, binding = 0) uniform sampler2D diffuseTex;
layout(set = 0, binding = 1) uniform sampler2D specTex;
layout(set = 0, binding = 2) uniform sampler2D alphaTex;
layout(set = 0, binding = 3) uniform  ShaderData{
	float shininess;
	float specularStrength;
} shaderData;

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
	if(texture(alphaTex,texCoord).r > 0.2)
		discard;

	float ambientStrength = 0.1;
	vec3 color = texture(diffuseTex,texCoord).rgb;
	vec3 norm = normalize(inNormal);
	float specularStrength = shaderData.specularStrength;
	vec3 viewDir = normalize(sceneBuffer.eyePos.xyz - inFragPos);
	vec3 specMask = texture(specTex,texCoord).bbb;

	vec3 finalLightColour = vec3(0.0);
	for(int i = 0; i < sceneBuffer.lightCount; i++)
	{
		vec4 lightPos = sceneBuffer.lightData[i].position;
		vec4 lightColour = sceneBuffer.lightData[i].intensities;

		
		vec3 ambient = ambientStrength * lightColour.rgb; //TODO only supports directional lights right now

		float diff = max(dot(norm, lightPos.xyz), 0.0);
		vec3 diffuse = diff * lightColour.rgb * lightColour.w;

		vec3 reflectDir = reflect(-lightPos.xyz, norm); 
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(shaderData.shininess,1.0));
		vec3 specular = specularStrength * spec * lightColour.rgb * specMask;  

		finalLightColour += ambient + diffuse + specular;
	}


    vec3 result = finalLightColour * color;

	outFragColor = vec4(result,1.0f);
}

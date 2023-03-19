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


layout(set = 1, binding = 0) uniform  SceneBuffer{
	vec4 directionalLightDir;
	vec4 directionalLightColor;
	mat4 view;
	vec4 eyePos;
} sceneBuffer;

void main()
{
	if(texture(alphaTex,texCoord).r > 0.2)
		discard;

	float ambientStrength = 0.1;
    vec3 ambient = ambientStrength * sceneBuffer.directionalLightColor.rgb;

	vec3 color = texture(diffuseTex,texCoord).rgb;

	vec3 norm = normalize(inNormal);
	float diff = max(dot(norm, sceneBuffer.directionalLightDir.xyz), 0.0);
	vec3 diffuse = diff * sceneBuffer.directionalLightColor.rgb * sceneBuffer.directionalLightColor.w;

	float specularStrength = shaderData.specularStrength;
	vec3 viewDir = normalize(sceneBuffer.eyePos.xyz - inFragPos);
	vec3 reflectDir = reflect(-sceneBuffer.directionalLightDir.xyz, norm); 
	float spec = pow(max(dot(viewDir, reflectDir), 0.0), max(shaderData.shininess,1.0));
	vec3 specMask = texture(specTex,texCoord).bbb;
	vec3 specular = specularStrength * spec * sceneBuffer.directionalLightColor.rgb * specMask;  


    vec3 result = (ambient + diffuse + specular) * color;

	outFragColor = vec4(result,1.0f);
}

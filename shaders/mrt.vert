#version 450

layout(location = 0) in vec3 position;
layout(location = 1) in vec3 color;
layout(location = 2) in vec3 normal;
layout(location = 3) in vec2 uv;

layout (binding = 0) uniform UBO 
{
	mat4 projection;
	mat4 model;
	mat4 view;
} ubo;

layout (location = 0) out vec3 outNormal;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outColor;
layout (location = 3) out vec3 outWorldPos;

void main() 
{
	vec4 tmpPos = vec4(position,0.f);

	gl_Position = ubo.projection * ubo.view * ubo.model * tmpPos;
	
	outUV = uv;

	// Vertex position in world space
	outWorldPos = vec3(ubo.model * tmpPos);
	
	// Normal in world space
	mat3 mNormal = transpose(inverse(mat3(ubo.model)));
	outNormal = mNormal * normalize(normal);	
	//outTangent = mNormal * normalize(inTangent);
	
	// Currently just vertex color
	outColor = color;
}

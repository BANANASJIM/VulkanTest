#version 450

//layout (binding = 1) uniform sampler2D samplerColor;
//layout (binding = 2) uniform sampler2D samplerNormalMap;

layout (location = 0) in vec3 inNormal;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inColor;
layout (location = 3) in vec3 inWorldPos;

layout (location = 0) out vec4 outPosition;
layout (location = 1) out vec4 outNormal;
layout (location = 2) out vec4 outAlbedo;

void main() 
{
	outPosition = vec4(1.0,1.0,1.0, 1.0);

	// Calculate normal in tangent space
	vec3 N = normalize(inNormal);
	vec3 T = vec3(1,0,0);
	vec3 B = cross(N, T);
	//mat3 TBN = mat3(T, B, N);
	//vec3 tnorm = TBN * normalize(texture(samplerNormalMap, inUV).xyz * 2.0 - vec3(1.0));
	outNormal = vec4(1.0,1.0,0.0, 1.0);
	outAlbedo = vec4(1.0,0.0,0.0,1.0);
}
#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBinormal;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec2 inTexcoord;

uniform mat4 worldMatrix;
uniform mat4 viewMatrix;
uniform mat4 projectionMatrix;
uniform mat4 normalMatrix;
uniform mat4 mvpMatrix;
uniform mat4 lightSpaceMatrix;
uniform mat4 projectorTransform;
uniform vec3 eye;
uniform bool drawSkybox = false;

out vec3 worldNormal;
out vec3 worldViewDirection;
out vec3 worldPosition;
out vec3 tangentToWorld1;
out vec3 tangentToWorld2;
out vec3 tangentToWorld3;
out vec2 texcoord;
out vec3 reflectionDirection;	// Reflected direction
out vec3 refractionDirection;	// Transmitted direction
out vec4 projectorTexcoord;
out vec3 fragPos;
out vec4 fragPosLightSpace;

struct Material{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
	vec3 Ke;
	float shininess;
	float reflectionFactor;
	float refractionFactor;
	// relative index of refraction(n1/n2)
	float ior;
	float eta;
	bool hasNormalMap;
};

uniform Material material;

void main() {
	if (drawSkybox) {
		// inPosition - origin(0, 0, 0) = inPosition
		reflectionDirection = inPosition;
	}
	else {
		worldPosition = (worldMatrix * vec4(inPosition, 1.0)).xyz;
		worldNormal = normalize((transpose(inverse(worldMatrix)) * vec4(inNormal, 0.0)).xyz);
		worldViewDirection = normalize(eye - worldPosition);
		reflectionDirection = reflect(-worldViewDirection, worldNormal);
		refractionDirection = refract(-worldViewDirection, worldNormal, material.eta);

		projectorTexcoord = projectorTransform * vec4(worldPosition, 1.0);
		projectorTexcoord = vec4(projectorTexcoord.xyz * 0.5 + 0.5 * projectorTexcoord.w, projectorTexcoord.w);

		fragPos = vec3(worldMatrix * vec4(inPosition, 1.0));
		fragPosLightSpace = lightSpaceMatrix * vec4(fragPos, 1.0);
	}

	vec3 worldTangent = normalize((worldMatrix * vec4(inTangent, 0.0)).xyz);
	vec3 worldBinormal = normalize(cross(worldNormal, worldTangent)); // normalize(worldMatrix * vec4(inBinormal, 0.0)).xyz);

	worldTangent = normalize(worldTangent - dot(worldTangent, worldNormal) * worldNormal);
	worldBinormal = cross(worldNormal, worldTangent);

	// Transform tangent, binormal, normal from object space to world space
	// tangentToWorld1, tangentToWorld2 and tangentToWorld3 construct the
	// matrix from tangent space to world space
	tangentToWorld1 = vec3(worldTangent.x, worldBinormal.x, worldNormal.x);
	tangentToWorld2 = vec3(worldTangent.y, worldBinormal.y, worldNormal.y);
	tangentToWorld3 = vec3(worldTangent.z, worldBinormal.z, worldNormal.z);

	texcoord = inTexcoord;

	// gl_Position = viewMatrix * worldMatrix * vec4(inPosition, 1.0);
	// gl_Position = worldMatrix * vec4(inPosition, 1.0);
	// gl_Position = vec4(inPosition, 1.0);
	// gl_Position = projectionMatrix * vec4(inPosition, 1.0);
	gl_Position = mvpMatrix * vec4(inPosition, 1.0);
}
#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBinormal;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec2 inTexcoord;

uniform mat4 worldMatrix;
uniform mat4 projectionMatrix;
uniform mat4 mvpMatrix;

out vec3 worldNormal;
out vec3 worldPosition;
out vec2 texcoord;

void main() {
	worldNormal = normalize(worldMatrix * vec4(inNormal, 0.0)).xyz;
	worldPosition = (worldMatrix * vec4(inPosition, 1.0)).xyz;
	texcoord = inTexcoord;

	gl_Position = mvpMatrix * vec4(inPosition, 1.0);
}
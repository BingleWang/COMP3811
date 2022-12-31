#version 400
layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

uniform mat4 worldMatrix;
uniform mat4 mvpMatrix;

out vec3 worldNormal;
out vec3 worldPosition;

void main() {
	worldNormal = normalize(worldMatrix * vec4(inNormal, 0.0)).xyz;
	worldPosition = (worldMatrix * vec4(inPosition, 1.0)).xyz;

	gl_Position = mvpMatrix * vec4(inPosition, 1.0);
}
#version 330 core

layout (location = 0) in vec3 inPosition;

uniform mat4 model;
uniform mat4 lightSpaceMatrix;

void main() {
	gl_Position = lightSpaceMatrix * model * vec4(inPosition, 1.0);
}
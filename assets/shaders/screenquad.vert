#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inTexcoord;

out vec2 texcoord;

uniform mat4 projectionMatrix;

void main() {
	texcoord = inTexcoord;
	gl_Position = projectionMatrix * vec4(inPosition, 1.0);
}
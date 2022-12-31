#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inTangent;
layout (location = 2) in vec3 inBinormal;
layout (location = 3) in vec3 inNormal;
layout (location = 4) in vec2 inTexcoord;

uniform mat4 mvpMatrix;

void main() {
	gl_Position = mvpMatrix * vec4(inPosition, 1.0);
}
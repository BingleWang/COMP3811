#version 330 core

uniform vec4 color;

uniform float emissionBoost = 1.0;

layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = color * emissionBoost;
}
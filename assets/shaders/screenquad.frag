#version 330 core

in vec2 texcoord;

uniform sampler2D renderTexture;

layout (location = 0) out vec4 fragColor;

void main() {
	fragColor = vec4(1.0, 0.0, 0.0, 1.0);
	fragColor = texture(renderTexture, texcoord);
}
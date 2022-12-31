#version 330 core

in vec2 texcoord;
in vec3 reflectionDirection;

layout (location = 0) out vec4 fragColor;

uniform samplerCube skybox1;
uniform samplerCube skybox2;

uniform float alpha = 0.0;

uniform float gamma = 2.2;
uniform float gammaInversed = 1.0 / 2.2;


void main() {
	vec4 color1 = texture(skybox1, reflectionDirection);
	vec4 color2 = texture(skybox2, reflectionDirection);
	fragColor = mix(color1, color2, alpha);
	// fragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
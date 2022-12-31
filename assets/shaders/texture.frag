#version 330 core

in vec3 worldNormal;
in vec3 worldPosition;
in vec2 texcoord;

layout(location = 0) out vec4 fragColor;

uniform sampler2D albedo;

void main(){
	vec4 textureColor = texture(albedo, texcoord);

	float GrayScale = 0.299 * textureColor.r + 0.578 * textureColor.g + 0.114 * textureColor.b;
	fragColor = vec4(GrayScale, GrayScale, GrayScale, 1.0);
}
#version 330 core

in vec3 worldNormal;
in vec3 worldPosition;
in vec2 texcoord;

layout(location = 0) out vec4 fragColor;

uniform sampler2D albedo;

void main(){
	vec4 textureColor = texture(albedo, texcoord);
	fragColor = textureColor;
}
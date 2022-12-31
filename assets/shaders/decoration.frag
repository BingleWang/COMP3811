#version 330 core

in vec3 worldNormal;
in vec3 worldPosition;
in vec2 texcoord;

layout(location = 0) out vec4 fragColor;

uniform bool bLight;
uniform sampler2D albedo;
uniform vec4 lightColor;

void main(){
	if (bLight) {
		fragColor = lightColor;
	}
	else {
		fragColor = texture(albedo, texcoord) * 0.5;
	}
}
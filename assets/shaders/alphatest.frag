#version 330 core

in vec3 worldNormal;
in vec3 worldPosition;
in vec2 texcoord;

layout (location = 0) out vec4 fragColor;

struct Light{
	vec4 color;
	vec4 position;
	vec3 direction;
    float exponent;
    float cutoff;
	float intensity;
	int type;
};

struct Material{
	float Ka;
	float Kd;
	float Ks;
	float shininess;
};

struct Fog {
	float minDistance;
	float maxDistance;
	float density;
	vec4 color;
};

uniform Light lights[3];

uniform Material material;

uniform Fog fog;

uniform vec3 ambient = vec3(0.2, 0.2, 0.2);

uniform vec3 eye;

uniform sampler2D brickTexture;
uniform sampler2D mossTexture;

float computeLinearFog(Fog fog, float distance) {
	return clamp((fog.maxDistance - distance) / (fog.maxDistance - fog.minDistance), 0.0, 1.0);
}

float computeExponentFog(Fog fog, float distance, float power) {
	return exp(-fog.density * distance);
}

vec3 blinnPhong(Light light, vec3 worldPosition, vec3 normal, vec3 eye, vec3 albedo, Material material) {
	vec3 lightDirection  = vec3(0.0);
	float attenuation = 1.0;

	if (light.position.w == 0.0) {
		lightDirection = normalize(-light.position.xyz);
	}
	else if (light.position.w >= 1.0) {
		lightDirection = normalize(light.position.xyz - worldPosition);
		attenuation = 1.0 / length(worldPosition - light.position.xyz);
	}

	float nDotL = dot(normal, lightDirection);
	 
	// Standard Lambert's law
	// float diffuse = max(0.0f, nDotL);

	// Half Lambert
	vec3 diffuse = (0.5 * nDotL + 0.5) * albedo * material.Kd;
	
	vec3 reflected = reflect(-lightDirection, normal);

	vec3 viewDirection = normalize(eye - worldPosition);
	
	vec3 halfVector = normalize(lightDirection + viewDirection);
	
	float specular = 0.0;

	if(nDotL > 0.0) {
		specular = pow(max(0.0, dot(halfVector, normal)), material.shininess) * material.Ks;
	}

	// Spot Light
	if (light.position.w == 2.0) {
		vec3 spotLightDirection = normalize(light.direction);
		float angle = acos(dot(-lightDirection, spotLightDirection));
		float cutoff = radians(clamp(light.cutoff, 0.0, 90.0));
		// In Spot Light cone
		if (angle < cutoff) {
			attenuation = pow(dot(-lightDirection, spotLightDirection), light.exponent);
		}
		else {
			attenuation = 0.0;
		}
	}

	// return (diffuse * albedo + specular) * light.color.rgb * light.intensity * attenuation;
	return diffuse;
}

void main() {
	vec3 normal = normalize(worldNormal);

	if (!gl_FrontFacing) {
		normal = -normal;
	}

	vec4 brick = texture(brickTexture, texcoord);
	vec4 moss = texture(mossTexture, texcoord);

	vec4 albedo = mix(brick, moss, moss.a);

	vec3 light1 = blinnPhong(lights[0], worldPosition, normal, eye, albedo.rgb, material);

	vec3 light2 = vec3(1.0);
	if (gl_FrontFacing) {
		light2 = blinnPhong(lights[1], worldPosition, normal, eye, albedo.rgb, material);	
	}
	else {
		light2 = blinnPhong(lights[1], worldPosition, -normal, eye, albedo.rgb, material);
	}

	vec3 light3 = blinnPhong(lights[2], worldPosition, normal, eye, albedo.rgb, material);

	float distance = length(eye - worldPosition);

	float fogFactor = computeLinearFog(fog, distance);

	fogFactor = computeExponentFog(fog, distance, 2.0);

	vec3 finalColor = mix(fog.color.rgb, light2, fogFactor);

	// fragColor = vec4(light1 + light2 + light3, 1.0);
	if (moss.a < 0.5) {
		discard;
	}
	else {
		fragColor = vec4(finalColor, 1.0);
	}
}
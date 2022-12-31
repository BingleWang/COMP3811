#version 330 core

in vec3 worldNormal;
in vec3 worldPosition;
in vec3 tangentToWorld1;
in vec3 tangentToWorld2;
in vec3 tangentToWorld3;
in vec2 texcoord;
in vec3 reflectionDirection;
in vec3 refractionDirection;
in vec3 worldViewDirection;
in vec4 projectorTexcoord;
in vec4 fragPosLightSpace;

layout (location = 0) out vec4 fragColor;

struct Light{
	vec4 color;
	vec4 position;
	vec3 direction;
    float exponent;
    float cutoff;
	float outerCutoff;
	float intensity;
	float Kc;
    float Kl;
    float Kq;
	int type;
};

struct Material{
	vec3 Ka;
	vec3 Kd;
	vec3 Ks;
    vec3 Ke;
    float shininess;
    float reflectionFactor;
    float refractionFactor;
    // relative index of refraction(n1/n2)
    float ior;
    float eta;
    bool hasNormalMap;
};

struct Fog {
	float minDistance;
	float maxDistance;
	float density;
	vec4 color;
};

uniform Light lights[5];

uniform Material material;

uniform Fog fog;

uniform vec3 eye;

uniform samplerCube skybox1;
uniform samplerCube skybox2;
uniform sampler2D projection;
uniform sampler2D renderTexture;
uniform sampler2D textures[2];
uniform sampler2D shadowMap;

uniform float ambientIntensity = 1.0;

uniform bool drawSkybox = false;
uniform bool showProjector = false;

uniform float gamma = 2.2;
uniform float gammaInversed = 1.0 / 2.2;

uniform float shadowmapBias;

float computeAttenuation(Light light, float distance) {
	return 1.0 / (light.Kc + light.Kl * distance + light.Kq * pow(distance, 2.0));
}

float computeLinearFog(Fog fog, float distance) {
	return clamp((fog.maxDistance - distance) / (fog.maxDistance - fog.minDistance), 0.0, 1.0);
}

float computeExponentFog(Fog fog, float distance, float power) {
	return exp(-pow(fog.density * distance, power));
}

vec3 blinnPhong(Light light, vec3 worldPosition, vec3 normal, vec3 eye, vec3 albedo, Material material, out float dotLightNormal) {
	vec3 lightDirection  = vec3(0.0);
	float attenuation = 1.0;

	if (light.position.w == 0.0) {
		lightDirection = normalize(-light.position.xyz);
		dotLightNormal = dot(normal, lightDirection);
	}
	else if (light.position.w >= 1.0) {
		lightDirection = normalize(light.position.xyz - worldPosition);
		float distance = length(worldPosition - light.position.xyz);
		attenuation = computeAttenuation(light, distance);
	}

	float nDotL = dot(normal, lightDirection);
	 
	// Standard Lambert's law
	vec3 diffuse = max(0.0, nDotL) * albedo * material.Kd;

	// Half Lambert
	// vec3 diffuse = (0.5 * nDotL + 0.5) * albedo * material.Kd;
	
	vec3 reflected = reflect(-lightDirection, normal);

	// vec3 viewDirection = normalize(eye - worldPosition);
	
	vec3 halfVector = normalize(lightDirection + worldViewDirection);
	
	vec3 specular = vec3(0.0);

	if(nDotL > 0.0) {
		specular = pow(max(0.0, dot(halfVector, normal)), material.shininess) * albedo * material.Ks;
	}

	// Spot Light
	// I = (θ - γ) / ϵ, ϵ = Φ - γ 
	// θ = angle between spotlight direction and -lightDirection
	// Φ = inner cone angle
	// γ = outer cone angle
	// see https://learnopengl.com/Lighting/Light-casters
	if (light.position.w == 2.0) {
		vec3 spotLightDirection = normalize(light.direction);
		float theta = dot(-lightDirection, spotLightDirection);
		float epsilon = light.cutoff - light.outerCutoff;
		attenuation = clamp((theta - light.outerCutoff) / epsilon, 0.0, 1.0);
	}

	return (diffuse + specular) * light.color.rgb * light.intensity * attenuation;
	// return diffuse;
	// return diffuse * light.color.rgb * light.intensity * attenuation;
	// return vec3(nDotL);
}

float shadowCalculation(vec4 fragPosLightSpace, float dotLightNormal) {
	vec3 projectedCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;

	projectedCoords = projectedCoords * 0.5 + 0.5;

	if (projectedCoords.z > 1.0) {
		projectedCoords.z = 1.0;
	}

	float bias = max(shadowmapBias * (1.0 - dotLightNormal), 0.0005);

	float currentDepth = projectedCoords.z;

	// PCF (Percentage-Closer Filter)
	float shadow = 0.0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);
	for (int x = -1; x <= 1; x++) {
		for (int y = -1; y <= 1; y++) {
			float closestDepth = texture(shadowMap, projectedCoords.xy + vec2(x, y) * texelSize).r;
			shadow += (closestDepth + shadowmapBias) < currentDepth ? 0.0 : 1.0;
		}
	}

	return shadow / 9.0;
}

void main() {
	vec4 albedo = texture(textures[0], texcoord);

	albedo = vec4(pow(albedo.rgb, vec3(gamma)), 1.0);

	vec3 ambient = material.Ka * albedo.rgb * ambientIntensity;

	vec3 normal = vec3(0);
	
	normal = normalize(worldNormal);

	if (material.hasNormalMap) {
		normal = texture(textures[1], texcoord).rgb;
		normal = normal * 2.0 - 1.0;
		normal = normalize(vec3(dot(tangentToWorld1, normal), dot(tangentToWorld2, normal), dot(tangentToWorld3, normal)));
	}

	if (!gl_FrontFacing) {
		normal = -normal;
	}

	float dotLightNormal = 0.0;

	vec3 light1 = blinnPhong(lights[0], worldPosition, normal, eye, albedo.rgb, material, dotLightNormal);

	vec3 light2 = blinnPhong(lights[1], worldPosition, normal, eye, albedo.rgb, material, dotLightNormal);

	vec3 light3 = blinnPhong(lights[2], worldPosition, normal, eye, albedo.rgb, material, dotLightNormal);

	vec3 light4 = blinnPhong(lights[3], worldPosition, normal, eye, albedo.rgb, material, dotLightNormal);

	vec3 light5 = blinnPhong(lights[4], worldPosition, normal, eye, albedo.rgb, material, dotLightNormal);

	float distance = length(eye - worldPosition);

	float fogFactor = computeLinearFog(fog, distance);

	fogFactor = computeExponentFog(fog, distance, 2.0);

	// vec3 finalColor = mix(fog.color.rgb, light1 + light2 + ambient, fogFactor);
	// vec3 finalColor = mix(fog.color.rgb, light1 + light2 + light3 + light4 + ambient, fogFactor);
	float shadow = shadowCalculation(fragPosLightSpace, dotLightNormal);

	vec3 finalColor = mix(fog.color.rgb, (light1 + light2 + light3 + light4 + light5) * shadow + ambient, fogFactor);
	// vec3 finalColor = light1 + light2 + light3 + light4 + light5 + ambient;
	// vec3 finalColor = light1;

	vec4 reflectionColor = texture(skybox1, reflectionDirection);
	vec4 refractionColor = texture(skybox1, refractionDirection);

	finalColor = mix(mix(vec4(finalColor, 1.0), reflectionColor, material.reflectionFactor), refractionColor, material.refractionFactor).rgb;
	fragColor = vec4(pow(finalColor, vec3(gammaInversed)), 1.0);
	// fragColor = vec4(finalColor, 1.0);
	// vec3 color = projectionTextureColor.z > 0.0 ? projectionTextureColor.rgb :vec3(0.0);
	// fragColor = vec4(light2, 1.0);
	// fragColor = vec4(vec3(1.0 - shadow), 1.0);
	
	vec4 projectionTextureColor = vec4(0.0);
	// Position in projector's 'CVV'
	// if(showProjector && (clamp(projectorTexcoord.x / projectorTexcoord.w, 0.0, 1.0) == projectorTexcoord.x / projectorTexcoord.w) 
	// && (clamp(projectorTexcoord.y / projectorTexcoord.w, 0.0, 1.0) == projectorTexcoord.y / projectorTexcoord.w)) {
	if (showProjector && projectorTexcoord.z > 0.0) {
		projectionTextureColor = textureProj(projection, projectorTexcoord) * 0.5;
		projectionTextureColor = texture(projection, projectorTexcoord.xy / projectorTexcoord.w) * 0.5;
		float depth = projectorTexcoord.z / projectorTexcoord.w;
		fragColor = vec4(vec3(depth), 1.0);
		fragColor = projectionTextureColor;
	}

	// fragColor = vec4(material.Ka, 1.0);

	// fragColor = vec4(pow(albedo.rgb + ambient, vec3(gammaInversed)), 1.0);
	// if (showProjector && projectorTexcoord.z < 0.0) {
	// 	projectionTextureColor = vec4(1.0, 0.0, 0.0, 1.0);
	// }
}
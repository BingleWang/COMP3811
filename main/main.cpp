#include <typeinfo>
#include <stdexcept>

#include <cstdio>
#include <cstdlib>
#include <random>
#include <filesystem>

#include "../support/error.hpp"
#include "../support/program.hpp"
#include "../support/checkpoint.hpp"
#include "../support/debug_output.hpp"

#include "../vmlib/vec4.hpp"
#include "../vmlib/mat44.hpp"

#include "defaults.hpp"

#define IMGUI_IMPL_OPENGL_LOADER_GLAD
#include <glad.h>
#include <GLFW/glfw3.h>

#include "imgui/imgui_impl_glfw.h"
#include "imgui/imgui_impl_opengl3.h"

#include "lodepng.h"

#define TINYOBJLOADER_IMPLEMENTATION
#include "tinyobjloader/tiny_obj_loader.h"

#define GLM_FORCE_SILENT_WARNINGS
#include "glm/glm.hpp"

//#include "SoundPlayer.hpp"
#include "GeometryGenerator.hpp"

#include "Shader.hpp"
#include "Model.hpp"
#include "Camera.hpp"
#include "glDebug.hpp"
#include "Particle.hpp"

#define UNUSED(x) (void)(x)

namespace
{
	constexpr char const* kWindowTitle = "COMP3811 - Coursework 2";
	
	void glfw_callback_error_( int, char const* );

	void glfw_callback_key_( GLFWwindow*, int, int, int, int );

	struct GLFWCleanupHelper
	{
		~GLFWCleanupHelper();
	};
	struct GLFWWindowDeleter
	{
		~GLFWWindowDeleter();
		GLFWwindow* window;
	};
}

struct Light {
	glm::vec4 color;
	glm::vec4 position;
	glm::vec3 direction;
	float exponent;
	float cutoff;
	float outerCutoff;
	float intensity;
	float Kc;
	float Kl;
	float Kq;
	int type;
};

struct Fog {
	float minDistance;
	float maxDistance;
	float density = 0.05f;
	glm::vec4 color;
};

std::vector<Particle> particles;

uint32_t lightCubeVAO;
uint32_t screenQuadVAO;
uint32_t particleQuadVAO;
uint32_t textureId;

std::shared_ptr<Shader> sceneShader;
std::shared_ptr<Shader> skyboxShader;
std::shared_ptr<Shader> textureShader;
std::shared_ptr<Shader> lightCubeShader;
std::shared_ptr<Shader> colorShader;
std::shared_ptr<Shader> decorationShader;
std::shared_ptr<Shader> particleShader;
std::shared_ptr<Shader> depthShader;
std::shared_ptr<Shader> screenQuadShader;

std::map<std::string, std::shared_ptr<Texture>> textures;
std::map<std::string, std::shared_ptr<Material>> materials;

std::vector<Light> lights(5);

bool lightSwitchs[5]{ true, true, true, true, true };

float lightIntensitys[5]{ 0.25f, 0.25f, 0.25f, 0.5f, 0.25f };

Fog fog = { 1.0f, 10.0f, 0.0f, {0.8f, 0.8f, 0.8f, 1.0f} };

bool bShowDemoWindow = false;
bool bShowImGUIWindow = true;
bool bShowAnotherWindow = false;
bool bShowOpenMenuItem = true;
bool bShowProjector = false;
bool bDrawNormals = false;
bool bFog = true;
bool bOrbitCamera = false;
bool bToggleHouseLight = false;
bool bShowDepthMap = false;
bool bDrawParticles = true;

bool toggleSnowmanAnimation = true;

bool bToggleChirstmasTreeLights[6][3];

bool bActivatedChristmasTreeLight[6] = { true, true, true, true, true, true };

bool bPlayMerryChristmasAnimation = false;

bool bSnowmanControlCameras[] = { false, false, false };
int32_t selectedSnowmanIndex = 0;

int32_t toggleLightIndex = 0;

Vec4f clearColor = { 0.392f, 0.584f, 0.929f, 1.0f };
Vec4f pointLightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
Vec4f directionaLightColor = { 1.0f, 1.0f, 1.0f, 1.0f };
Vec4f lightDirection = { 0.0f, -1.0f, -1.0f, 0.0f };

GLFWwindow* window = nullptr;

int32_t WindowWidth = 1280;
int32_t WindowHeight = 720;

float aspect = static_cast<float>(WindowWidth) / WindowHeight;

float frameTime = 0.016667f;
float elapsedTime = 0.0f;

bool bRightMouseButtonDown = false;
bool bMiddleMouseButtonDown = false;

glm::vec2 lastMousePosition = { 0.0f, 0.0f };

float rotateSpeed = 3.0f;

glm::vec3 projectorPosition = glm::vec3(-15.0f, 15.0f, -15.0f);
glm::vec3 projectAt = glm::vec3(0.0f, 0.0f, 0.0f);
glm::vec3 projectorUp = glm::vec3(0.0f, 1.0f, 0.0f);
glm::vec3 rotateCenter = glm::vec3(0.0f);
glm::vec3 cameraPosition = glm::vec3(0.0, 0.0f, 10.0f);
glm::vec3 activeSnowmanVelocity = glm::vec3(0.0f);

float fov = 60.0f;
float nearPlane = 0.2f;
float farPlane = 200.0f;

float exposure = 1.0f;

float maxCameraSpeed = 10.0f;
float cameraSpeed = 6.0f;
float minCameraSpeed = 3.0f;

//Camera mainCamera(projectorPosition, projectAt);
Camera mainCamera({ 0.0, 0.0f, 10.0f }, { 0.0f, 0.0f, -10.0f });
Camera secondaryCamera({ 0.0, 0.0f, 10.0f }, { 0.0f, 0.0f, -10.0f });
//Camera camera({ cameraPosition.x, -cameraPosition.y + (-1.5f * 2.0f), cameraPosition.z }, { 0.0f, cameraPosition.y, -1.0f });

glm::mat4 projectorView = glm::lookAt(projectorPosition, projectAt, projectorUp);
glm::mat4 projectorProjection = glm::perspective(glm::radians(60.0f), aspect, nearPlane, farPlane);
glm::mat4 projectorScaleTranslate = glm::mat4(1.0f);
glm::mat4 projectorTransform = glm::mat4(1.0f);
glm::mat4 lightSpaceMatrix;

uint32_t renderSceneFBO;

std::shared_ptr<Texture> sceneTexture;
std::shared_ptr<Texture> skyboxDay;
std::shared_ptr<Texture> skyboxDusk;
std::shared_ptr<Texture> skyboxNight;
std::shared_ptr<Texture> currentSkybox;
std::shared_ptr<Texture> defaultAlbedo;
std::shared_ptr<Texture> currentHouseTexture;
std::shared_ptr<Texture> houseTexture;
std::shared_ptr<Texture> houseLightTexture;
std::shared_ptr<Texture> snowflakesTexture;
std::shared_ptr<Texture> depthMapTexture;
std::shared_ptr<Texture> markusTexture;

uint32_t depthBuffer = 0;

std::shared_ptr<Model> cubemapSphere;
std::shared_ptr<Model> movingSphere;
std::shared_ptr<Model> renderWindow;
std::shared_ptr<Model> reflectionFloor;
std::shared_ptr<Model> teapot;
std::shared_ptr<Model> lightSphere;
std::shared_ptr<Model> terrain;
std::shared_ptr<Model> leftHouse;
std::shared_ptr<Model> rightHouse;

std::shared_ptr<Model> leftSnowman;
std::shared_ptr<Model> leftSnowmanArm;
std::shared_ptr<Model> middleSnowman;
std::shared_ptr<Model> middleSnowmanArm;
std::shared_ptr<Model> rightSnowman;
std::shared_ptr<Model> rightSnowmanArm;

std::shared_ptr<Model> merryChristmasSnowman;
std::shared_ptr<Model> merryChristmasSnowmanArm;

std::shared_ptr<Model> flagpole;
std::shared_ptr<Model> flag;

std::shared_ptr<Model> activeSnowman;
std::shared_ptr<Model> activeSnowmanArm;

int32_t activeSnowmanIndex = 0;

std::vector<std::shared_ptr<Model>> snowmen;

std::vector<std::shared_ptr<Model>> models;
std::vector<std::shared_ptr<Model>> smokes1;
std::vector<std::shared_ptr<Model>> smokes2;

std::shared_ptr<Model> redBalls[6];
std::shared_ptr<Model> goldenBalls[6];
std::shared_ptr<Model> purpleBalls[6];

std::shared_ptr<Model> giftBoxBody;
std::shared_ptr<Model> giftBoxCover;

std::vector<std::vector<std::shared_ptr<Model>>> smokesGroup = { smokes1, smokes2 };

//glm::vec3 smokeSpwanPositions[3] = { { leftHouse->getPosition().x, 5.25f, -0.75f},
//                                     {0.75f, 4.75f, -0.75f},
//                                     {0.75f, 4.0f, -0.75f} };

glm::vec3 smokeSpawnPositions1[3];
glm::vec3 smokeSpawnPositions2[3];

float smokeInterval = 3.0f;
float smokeTimers[3];

GeometryGenerator geometryGenerator;
float edgeThreshold = 0.05f;
glm::vec3 edgeColor = { 1.0f, 1.0f, 1.0f };

char uniformName[20];
std::vector<float> weights(10);
float sum;
float sigmaSquared = 4.0f;
float cameraRotateRate = 0.05f;
float luminanceThreshold = 0.95f;
float globalScale = 1.0f;
float skyboxRotateRate = 1.0f;
float ambientIntensity = 0.6f;

float jumpHeight = 5.0f;
glm::vec3 velocityY = glm::vec3(0.0f, 12.0f, 0.0f);
glm::vec3 gravity = glm::vec3(0.0f, -0.98f, 0.0f);

float snowmenRollRate[3] = { 2.0f, 3.0f, -4.0f };

float snowmanMoveSpeed = 3.0f;
float snowmanYawRate = 90.0f;

float timeOfDay = 0.0f;

float snowmanYawRates[3];

float lightTimer = 0.0f;

constexpr int FLOAT_MIN = 0;
constexpr int FLOAT_MAX = 1;

constexpr GLuint ShadowMapWidth = 2048;
constexpr GLuint SHadowMapHeight = 2048;

float shadowmapBias = 0.001f;

unsigned int depthMapFBO;
unsigned int depthMap;

std::shared_ptr<Model> createSmoke(float radius, const glm::vec3& position);
void drawParticle(const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix);

void updateFPSCounter(GLFWwindow* window);

float getNoise(float x, float z) {
	std::srand(static_cast<uint32_t>(x * 49632) + static_cast<uint32_t>(z * 325186 + 1000000000));
	return FLOAT_MIN + (float)(rand()) / ((float)(RAND_MAX / (FLOAT_MAX - FLOAT_MIN)));
}

float getSmoothNoise(float x, float z) {

	float corners = (getNoise(x - 1.0f, z - 1.0f) +
		getNoise(x + 1.0f, z - 1.0f) +
		getNoise(x - 1.0f, z + 1.0f) +
		getNoise(x + 1.0f, z + 1.0f)) / 16.0f;

	float sides = (getNoise(x - 1.0f, z) +
		getNoise(x + 1.0f, z) +
		getNoise(x, z - 1.0f) +
		getNoise(x, z + 1.0f)) / 8.0f;

	float center = getNoise(x, z) / 4.0f;

	return corners + sides + center;
}

const auto& getTexture(const std::string& name);

void writeToPNG(const std::string& path, int32_t width, int32_t height, uint8_t* pixelBuffer);

std::shared_ptr<Model> loadModel(const std::string& fileName, const std::string& inName = "", const std::string& materialPath = "./assets/models", const std::string& texturePath = "./assets/textures/");

void onFrameBufferResize(GLFWwindow* inWindow, int width, int height) {

	UNUSED(inWindow);

	if (width > 0 && height > 0)
	{
		WindowWidth = width;
		WindowHeight = height;
		aspect = static_cast<float>(width) / static_cast<float>(height);
		mainCamera.perspective(fov, aspect, nearPlane, farPlane);
		glViewport(0, 0, width, height);
	}
}

void changeCameraControl() {

	auto newCameraPosition = glm::vec3(activeSnowman->getPosition().x, 0.25f, activeSnowman->getPosition().z);
	auto offset = glm::vec3(activeSnowman->getForward() * 5.0f);
	mainCamera.setCenter(newCameraPosition);

	if (bSnowmanControlCameras[selectedSnowmanIndex]) {
		mainCamera.setEye(newCameraPosition - offset);

	}
	else {
		mainCamera.setEye(newCameraPosition + offset * 2.0f);
	}
}

void onKeyCallback(GLFWwindow* inWindow, int key, int scancode, int action, int mods) {

	//ImGui_ImplGlfw_KeyCallback(window, key, scancode, action, mods);
	UNUSED(mods);
	UNUSED(action);
	UNUSED(scancode);
	UNUSED(key);

	if (glfwGetKey(inWindow, GLFW_KEY_C) == GLFW_PRESS) {
		bOrbitCamera = !bOrbitCamera;
	}

	if (glfwGetKey(inWindow, GLFW_KEY_H) == GLFW_PRESS) {
		if (bToggleHouseLight) {
			currentHouseTexture = houseTexture;
		}
		else {
			currentHouseTexture = houseLightTexture;
		}

		bToggleHouseLight = !bToggleHouseLight;
	}

	if (key == GLFW_KEY_EQUAL && action == GLFW_PRESS)
	{
		skyboxRotateRate += 2.0f;
	}

	if (key == GLFW_KEY_MINUS && action == GLFW_PRESS)
	{
		skyboxRotateRate -= 2.0f;
	}

	if (key == GLFW_KEY_KP_ADD)
	{
		snowmenRollRate[0] += 5.0f;
		snowmenRollRate[1] += 5.0f;
		snowmenRollRate[2] -= 5.0f;
	}

	if (key == GLFW_KEY_KP_SUBTRACT)
	{
		snowmenRollRate[0] -= 5.0f;
		snowmenRollRate[1] -= 5.0f;
		snowmenRollRate[2] += 5.0f;
	}

	if (key == GLFW_KEY_KP_1 && action == GLFW_PRESS)
	{
		lightSwitchs[0] = !lightSwitchs[0];
	}

	if (key == GLFW_KEY_KP_2 && action == GLFW_PRESS)
	{
		lightSwitchs[1] = !lightSwitchs[1];
	}

	if (key == GLFW_KEY_KP_3 && action == GLFW_PRESS)
	{
		lightSwitchs[2] = !lightSwitchs[2];
	}

	if (key == GLFW_KEY_KP_4 && action == GLFW_PRESS)
	{
		lightSwitchs[3] = !lightSwitchs[3];
	}

	if (key == GLFW_KEY_KP_5 && action == GLFW_PRESS)
	{
		lightSwitchs[4] = !lightSwitchs[4];
	}

	if (key == GLFW_KEY_KP_ENTER && action == GLFW_PRESS)
	{
		toggleSnowmanAnimation = !toggleSnowmanAnimation;
	}

	if (glfwGetKey(inWindow, GLFW_KEY_SPACE) == GLFW_PRESS) {
		if (activeSnowman && !activeSnowman->bJumping) {
			auto activeSnowmanPosition = activeSnowman->getPosition();
			activeSnowmanPosition += velocityY * frameTime * jumpHeight;
			activeSnowman->setPosition(activeSnowmanPosition);
			activeSnowmanArm->setPosition(activeSnowmanPosition);

			snowmanYawRates[activeSnowmanIndex] = 360.0f / ((activeSnowmanPosition.y - (-1.8f)) / (gravity.y * frameTime));

			activeSnowman->bJumping = true;
		}
	}

	if (glfwGetKey(inWindow, GLFW_KEY_1) == GLFW_PRESS) {
		activeSnowman = leftSnowman;
		activeSnowmanArm = leftSnowmanArm;
		activeSnowmanIndex = 0;
		bSnowmanControlCameras[selectedSnowmanIndex] = !bSnowmanControlCameras[selectedSnowmanIndex];
		//changeCameraControl();

		if (!bSnowmanControlCameras[selectedSnowmanIndex]) {
			auto newCameraPosition = glm::vec3(activeSnowman->getPosition().x, 0.25f, activeSnowman->getPosition().z);
			auto offset = glm::vec3(activeSnowman->getForward() * 5.0f);
			mainCamera.setCenter(newCameraPosition);
			mainCamera.setEye(newCameraPosition + offset * 2.0f);
		}
	}

	if (glfwGetKey(inWindow, GLFW_KEY_2) == GLFW_PRESS) {
		activeSnowman = middleSnowman;
		activeSnowmanArm = middleSnowmanArm;
		activeSnowmanIndex = 1;
		bSnowmanControlCameras[selectedSnowmanIndex] = !bSnowmanControlCameras[selectedSnowmanIndex];
		//changeCameraControl();
		// 
		if (!bSnowmanControlCameras[selectedSnowmanIndex]) {
			auto newCameraPosition = glm::vec3(activeSnowman->getPosition().x, 0.25f, activeSnowman->getPosition().z);
			auto offset = glm::vec3(activeSnowman->getForward() * 5.0f);
			mainCamera.setCenter(newCameraPosition);
			mainCamera.setEye(newCameraPosition + offset * 2.0f);
		}
		//if (!middleSnowman->bJumping) {
		//    auto middleSnowmanPosition = middleSnowman->getPosition();
		//    middleSnowmanPosition += velocityY * frameTime * jumpHeight;
		//    middleSnowman->setPosition(middleSnowmanPosition);
		//    middleSnowmanArm->setPosition(middleSnowmanPosition);

		//    snowmanYawRates[1] = 360.0f / ((middleSnowmanPosition.y - (-1.6f)) / (gravity.y * frameTime));

		//    middleSnowman->bJumping = true;
		//}
	}

	if (glfwGetKey(inWindow, GLFW_KEY_3) == GLFW_PRESS) {
		activeSnowman = rightSnowman;
		activeSnowmanArm = rightSnowmanArm;
		activeSnowmanIndex = 2;
		bSnowmanControlCameras[selectedSnowmanIndex] = !bSnowmanControlCameras[selectedSnowmanIndex];
		//changeCameraControl();

		if (!bSnowmanControlCameras[selectedSnowmanIndex]) {
			auto newCameraPosition = glm::vec3(activeSnowman->getPosition().x, 0.25f, activeSnowman->getPosition().z);
			auto offset = glm::vec3(activeSnowman->getForward() * 5.0f);
			mainCamera.setCenter(newCameraPosition);
			mainCamera.setEye(newCameraPosition + offset * 2.0f);
		}
		//if (!rightSnowman->bJumping) {
		//    auto rightSnowmanPosition = rightSnowman->getPosition();
		//    rightSnowmanPosition += velocityY * frameTime * jumpHeight;
		//    rightSnowman->setPosition(rightSnowmanPosition);
		//    rightSnowmanArm->setPosition(rightSnowmanPosition);

		//    snowmanYawRates[2] = 360.0f / ((rightSnowmanPosition.y - (-1.6f)) / (gravity.y * frameTime));

		//    rightSnowman->bJumping = true;
		//}
	}

	if (glfwGetKey(inWindow, GLFW_KEY_4) == GLFW_PRESS) {
		bActivatedChristmasTreeLight[0] = !bActivatedChristmasTreeLight[0];
	}

	if (glfwGetKey(inWindow, GLFW_KEY_5) == GLFW_PRESS) {
		bActivatedChristmasTreeLight[1] = !bActivatedChristmasTreeLight[1];
	}

	if (glfwGetKey(inWindow, GLFW_KEY_6) == GLFW_PRESS) {
		bActivatedChristmasTreeLight[2] = !bActivatedChristmasTreeLight[2];
	}

	if (glfwGetKey(inWindow, GLFW_KEY_7) == GLFW_PRESS) {
		bActivatedChristmasTreeLight[3] = !bActivatedChristmasTreeLight[3];
	}

	if (glfwGetKey(inWindow, GLFW_KEY_8) == GLFW_PRESS) {
		bActivatedChristmasTreeLight[4] = !bActivatedChristmasTreeLight[4];
	}

	if (glfwGetKey(inWindow, GLFW_KEY_9) == GLFW_PRESS) {
		bActivatedChristmasTreeLight[5] = !bActivatedChristmasTreeLight[5];
	}

	if (glfwGetKey(inWindow, GLFW_KEY_UP) == GLFW_RELEASE) {
		activeSnowmanVelocity = glm::vec3(0.0f);
	}

	if (glfwGetKey(inWindow, GLFW_KEY_DOWN) == GLFW_RELEASE) {
		activeSnowmanVelocity = glm::vec3(0.0f);
	}

	if (glfwGetKey(inWindow, GLFW_KEY_G) == GLFW_PRESS) {
		bPlayMerryChristmasAnimation = !bPlayMerryChristmasAnimation;

		if (bPlayMerryChristmasAnimation) {
			giftBoxCover->translate(glm::vec3(0.0f, 10.0f, 0.0f));
			//SoundPlayer::getInstance()->stopAllSounds();
			//SoundPlayer::getInstance()->setVolume(10.0f);
			//SoundPlayer::getInstance()->play("./assets/music/MerryChristmas.wav", true);
		}
		else {
			//SoundPlayer::getInstance()->stopAllSounds();
			//SoundPlayer::getInstance()->setVolume(10.0f);
			//SoundPlayer::getInstance()->play("./assets/music/ChristmasEve.ogg", true);
		}

	}

	if (glfwGetKey(inWindow, GLFW_KEY_P) == GLFW_PRESS) {
		bDrawParticles = !bDrawParticles;
	}

	if (glfwGetKey(inWindow, GLFW_KEY_F) == GLFW_PRESS) {
		bShowDepthMap = !bShowDepthMap;
	}

	if (glfwGetKey(inWindow, GLFW_KEY_I) == GLFW_PRESS) {
		bShowImGUIWindow = !bShowImGUIWindow;
	}

	if (glfwGetKey(inWindow, GLFW_KEY_P) == GLFW_PRESS) {
		glReadBuffer(GL_FRONT);

		uint8_t* pixels = new uint8_t[WindowWidth * WindowHeight * 3];
		glReadPixels(0, 0, WindowWidth, WindowHeight, GL_RGB, GL_UNSIGNED_BYTE, pixels);

		writeToPNG("screenshot.png", WindowWidth, WindowHeight, pixels);
	}
}

void onMouseButtonCallback(GLFWwindow* inWindow, int button, int action, int mods) {
	UNUSED(mods);

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS) {
		bRightMouseButtonDown = true;
		double x;
		double y;
		glfwGetCursorPos(inWindow, &x, &y);
		lastMousePosition.x = static_cast<float>(x);
		lastMousePosition.y = static_cast<float>(y);
	}

	if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_RELEASE) {
		bRightMouseButtonDown = false;
	}

	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_PRESS) {
		bMiddleMouseButtonDown = true;
	}

	if (button == GLFW_MOUSE_BUTTON_MIDDLE && action == GLFW_RELEASE) {
		bMiddleMouseButtonDown = false;
	}

	//ImGui_ImplGlfw_MouseButtonCallback(window, button, action, mods);
}

void onScrollCallback(GLFWwindow* inWindow, double xOffset, double yOffset) {
	UNUSED(xOffset);
	UNUSED(inWindow);
	//ImGui_ImplGlfw_ScrollCallback(inWindow, xOffset, yOffset);

	//if (ImGui::GetIO().WantCaptureMouse) {
	//    return;
	//}

	mainCamera.walk(static_cast<float>(yOffset) / 8.0f);
}

void onMouseMoveCallback(GLFWwindow* inWindow, double x, double y) {

	UNUSED(inWindow);

	float dx = (lastMousePosition.x - static_cast<float>(x)) * frameTime;
	float dy = (lastMousePosition.y - static_cast<float>(y)) * frameTime;

	if (bRightMouseButtonDown) {
		mainCamera.yaw(dx * rotateSpeed);
		mainCamera.pitch(dy * rotateSpeed);
	}

	if (bMiddleMouseButtonDown) {
		mainCamera.strafe(-dx / 2.0f);
		mainCamera.raise(dy / 2.0f);
	}

	lastMousePosition.x = static_cast<float>(x);
	lastMousePosition.y = static_cast<float>(y);
}

void processInput(GLFWwindow* inWindow) {

	if (glfwGetKey(inWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(inWindow, true);
	}

	//if (ImGui::GetIO().WantCaptureMouse || ImGui::GetIO().WantCaptureKeyboard) {
	//    return;
	//}

	if (bOrbitCamera) {
		if (glfwGetKey(inWindow, GLFW_KEY_LEFT)) {
			// Orbit camera
			float x0 = mainCamera.getEye().x * std::cos(cameraRotateRate) - mainCamera.getEye().z * std::sin(cameraRotateRate);
			float z0 = mainCamera.getEye().x * std::sin(cameraRotateRate) + mainCamera.getEye().z * std::cos(cameraRotateRate);

			mainCamera.setEye(glm::vec3(x0, mainCamera.getEye().y, z0));
		}

		if (glfwGetKey(inWindow, GLFW_KEY_RIGHT)) {
			// Orbit camera
			float x0 = mainCamera.getEye().x * std::cos(-cameraRotateRate) - mainCamera.getEye().z * std::sin(-cameraRotateRate);
			float z0 = mainCamera.getEye().x * std::sin(-cameraRotateRate) + mainCamera.getEye().z * std::cos(-cameraRotateRate);

			mainCamera.setEye(glm::vec3(x0, mainCamera.getEye().y, z0));
		}
	}
	else {

		if (glfwGetKey(inWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS) {
			mainCamera.setCameraSpeed(maxCameraSpeed);
		}
		else if (glfwGetKey(inWindow, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
			mainCamera.setCameraSpeed(minCameraSpeed);
		}
		else
		{
			mainCamera.setCameraSpeed(cameraSpeed);
		}

		if (glfwGetKey(inWindow, GLFW_KEY_A) == GLFW_PRESS) {
			mainCamera.strafe(-frameTime);
		}

		if (glfwGetKey(inWindow, GLFW_KEY_D) == GLFW_PRESS) {
			mainCamera.strafe(frameTime);
		}

		if (glfwGetKey(inWindow, GLFW_KEY_W) == GLFW_PRESS) {
			mainCamera.walk(frameTime);
		}

		if (glfwGetKey(inWindow, GLFW_KEY_S) == GLFW_PRESS) {
			mainCamera.walk(-frameTime);
		}

		if (glfwGetKey(inWindow, GLFW_KEY_Q) == GLFW_PRESS) {
			mainCamera.raise(frameTime);
		}

		if (glfwGetKey(inWindow, GLFW_KEY_E) == GLFW_PRESS) {
			mainCamera.raise(-frameTime);
		}

		if (glfwGetKey(inWindow, GLFW_KEY_UP) == GLFW_PRESS) {
			if (activeSnowman) {
				activeSnowmanVelocity = activeSnowman->getForward() * frameTime * snowmanMoveSpeed;
				mainCamera.setCameraSpeed(1.0);
				mainCamera.walk(frameTime * snowmanMoveSpeed);
				changeCameraControl();
			}
		}

		if (glfwGetKey(inWindow, GLFW_KEY_DOWN) == GLFW_PRESS) {
			if (activeSnowman) {
				activeSnowmanVelocity = -activeSnowman->getForward() * frameTime * snowmanMoveSpeed;
				mainCamera.setCameraSpeed(1.0);
				mainCamera.walk(-frameTime * snowmanMoveSpeed);
				changeCameraControl();
			}
		}

		if (glfwGetKey(inWindow, GLFW_KEY_LEFT) == GLFW_PRESS) {
			if (activeSnowman) {
				activeSnowman->rotate(frameTime * snowmanYawRate, glm::vec3(0.0f, 1.0f, 0.0f));
				activeSnowmanArm->rotate(frameTime * snowmanYawRate, glm::vec3(0.0f, 1.0f, 0.0f));

				auto newCameraPosition = glm::vec3(activeSnowman->getPosition().x, 0.25f, activeSnowman->getPosition().z);
				mainCamera.setEye(newCameraPosition);
				mainCamera.yaw(frameTime * snowmanYawRate);
				mainCamera.setEye(newCameraPosition - glm::vec3(activeSnowman->getForward() * 5.0f));
			}
		}


		if (glfwGetKey(inWindow, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			if (activeSnowman) {
				activeSnowman->rotate(-frameTime * snowmanYawRate, glm::vec3(0.0f, 1.0f, 0.0f));
				activeSnowmanArm->rotate(-frameTime * snowmanYawRate, glm::vec3(0.0f, 1.0f, 0.0f));

				auto newCameraPosition = glm::vec3(activeSnowman->getPosition().x, 0.25f, activeSnowman->getPosition().z);
				mainCamera.setEye(newCameraPosition);
				mainCamera.yaw(-frameTime * snowmanYawRate);
				mainCamera.setEye(newCameraPosition - glm::vec3(activeSnowman->getForward() * 5.0f));
			}
		}
	}

	mainCamera.updateViewMatrix();
}

void generateFrameBufferObject(uint32_t& fbo, const std::shared_ptr<Texture>& renderTexture, bool hasDepthBuffer = true, GLenum target = GL_TEXTURE_2D) {

	// Generate and bind the framebuffer
	glGenFramebuffers(1, &fbo);
	glBindFramebuffer(GL_FRAMEBUFFER, fbo);

	// Bind the texture to the FBO
	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, target, renderTexture->getTextureId(), 0);

	// Every framebuffer need a depth buffer
	if (hasDepthBuffer) {
		// Create the depth buffer
		glGenRenderbuffers(1, &depthBuffer);
		glBindRenderbuffer(GL_RENDERBUFFER, depthBuffer);
		glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, renderTexture->getWidth(), renderTexture->getHeight());

		glBindRenderbuffer(GL_RENDERBUFFER, 0);

		// Bind the depth buffer to the FBO
		glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthBuffer);
	}

	// Set the target for the fragment shader outputs
	uint32_t drawBuffers[] = { GL_NONE, GL_COLOR_ATTACHMENT0 };
	glDrawBuffers(2, drawBuffers);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Frame Buffer is not Complete." << std::endl;
	}

	// Unbind the framebuffer, and revert to default framebuffer
	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void generateDepthFrameBufferObject() {

	glGenFramebuffers(1, &depthMapFBO);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMapTexture->getTextureId(), 0);

	glDrawBuffer(GL_NONE);
	glReadBuffer(GL_NONE);

	if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
		std::cout << "Frame Buffer is not Complete." << std::endl;
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	//glGenFramebuffers(1, &depthMapFBO);
	// create depth texture
	//glGenTextures(1, &depthMap);
	//glActiveTexture(GL_TEXTURE0 + Texture::getActiveIndex());
	//Texture::increaseActiveIndex();
	//glBindTexture(GL_TEXTURE_2D, depthMap);
	//glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, ShadowMapWidth, SHadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// attach depth texture as FBO's depth buffer
	//glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);
	//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, depthMap, 0);
	//glDrawBuffer(GL_NONE);
	//glReadBuffer(GL_NONE);
	//glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void prepareFrameBufferObject() {
	//generateFrameBufferObject(renderSceneFBO, sceneTexture);
	generateDepthFrameBufferObject();
}

auto createShader(const std::string& name, const std::string& basePath) {

	auto shader = std::make_shared<Shader>(name);

	shader->create();
	shader->compileShaderFromFile(basePath + ".vert", ShaderType::VERTEX);
	shader->compileShaderFromFile(basePath + ".frag", ShaderType::FRAGMENT);

	shader->link();

	return shader;
}

void prepareShaderResources() {

	sceneShader = createShader("scene", "./assets/shaders/scene");
	skyboxShader = createShader("skybox", "./assets/shaders/skybox");
	textureShader = createShader("texture", "./assets/shaders/texture");
	lightCubeShader = createShader("color", "./assets/shaders/color");
	colorShader = createShader("color", "./assets/shaders/color");
	decorationShader = createShader("decoration", "./assets/shaders/decoration");
	particleShader = createShader("particle", "./assets/shaders/particle");
	depthShader = createShader("depth", "./assets/shaders/depth");
	//screenQuadShader = createShader("screenquad", "./resources/shaders/screenquad");
	screenQuadShader = createShader("screenquad", "./assets/shaders/debugquaddepth");

	lights[0].color = { 1.0f, 0.0f, 0.2f, 1.0f };
	lights[0].position = { -1.0f, -1.0f, -1.0f, 0.0f };
	lights[0].intensity = 0.25f;
	lights[0].Kc = 0.5f;
	lights[0].Kl = 0.09f;
	lights[0].Kq = 0.032f;
	lights[0].type = 1;

	lights[1].color = { 0.0f, 1.0f, 0.0f, 1.0f };
	lights[1].position = { -1.0f, 5.0f, 0.0f, 1.0f };
	lights[1].intensity = 0.25f;
	lights[1].Kc = 0.5f;
	lights[1].Kl = 0.09f;
	lights[1].Kq = 0.032f;
	lights[1].type = 0;

	lights[2].color = { 0.0f, 0.0f, 1.0f, 1.0f };
	lights[2].position = { -1.0f, 2.0f, -0.5f, 1.0f };
	lights[2].intensity = 0.25f;
	lights[2].Kc = 0.5f;
	lights[2].Kl = 0.09f;
	lights[2].Kq = 0.032f;
	lights[2].type = 0;

	lights[3].color = { 1.0f, 1.0f, .0f, 1.0f };
	lights[3].position = { 1.0f, -1.0f, 1.0f, 0.0 };
	lights[3].intensity = 0.5f;
	lights[3].type = 0;

	lights[4].color = { 1.0f, 1.0f, 1.0f, 1.0f };
	lights[4].position = { -5.5f, 3.0f, 2.0f, 2.0f };
	lights[4].direction = { 0.0f, -1.0f, 0.0f };
	lights[4].exponent = 8.0f;
	lights[4].cutoff = 15.0f;
	lights[4].outerCutoff = 30.0f;
	lights[4].intensity = 0.25f;
	lights[4].type = 2;

	projectorTransform = projectorProjection * projectorView;

	prepareFrameBufferObject();
}

void createScreenQuad() {

	SimpleVertex quadVertices[] = {
		// positions        // texture Coords
		{{-1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
		{{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}},
		{{ 1.0f,  1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},
		{{ 1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f} }
	};
	// setup plane VAO
	glGenVertexArrays(1, &screenQuadVAO);

	uint32_t screenQuadVBO = 0;
	glGenBuffers(1, &screenQuadVBO);
	glBindVertexArray(screenQuadVAO);
	glBindBuffer(GL_ARRAY_BUFFER, screenQuadVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), quadVertices, GL_STATIC_DRAW);

	auto stride = sizeof(SimpleVertex);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)0);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)(sizeof(float) * 3));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)(sizeof(float) * 6));
}

void createParticleQuad() {

	SimpleVertex particleQuadVertices[] = {
	{{ -0.5f, -0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 0.0f }},
	{{  0.5f, -0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 0.0f }},
	{{  0.5f,  0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 1.0f, 1.0f }},
	{{ -0.5f,  0.5f, 0.0f}, { 0.0f, 0.0f, 1.0f }, { 0.0f, 1.0f }}
	};

	uint32_t particleQuadIndices[] = {
		0, 1, 2,
		2, 3, 0
	};

	glGenVertexArrays(1, &particleQuadVAO);
	glBindVertexArray(particleQuadVAO);

	uint32_t particleQuadVbo;
	glGenBuffers(1, &particleQuadVbo);
	glBindBuffer(GL_ARRAY_BUFFER, particleQuadVbo);

	glBufferData(GL_ARRAY_BUFFER, sizeof(particleQuadVertices), particleQuadVertices, GL_STATIC_DRAW);

	auto stride = sizeof(SimpleVertex);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)nullptr);
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)(sizeof(float) * 3));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, static_cast<GLsizei>(stride), (void*)(sizeof(float) * 6));
	glEnableVertexAttribArray(2);

	uint32_t particleQuadIbo;

	glGenBuffers(1, &particleQuadIbo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, particleQuadIbo);

	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(particleQuadIndices), particleQuadIndices, GL_STATIC_DRAW);
}

void spawnParticles(int32_t amount) {

	std::random_device randomDevice;
	std::mt19937 mt(randomDevice());
	std::uniform_real_distribution<float> distribution(-1.0f, 1.0f);

	for (auto i = 0; i < amount; i++) {
		auto position = glm::vec3(distribution(mt) * 20.0f, 10.0f, distribution(mt) * 20.0f);
		auto gravityEffect = (distribution(mt) + 1.0f) * 0.4f + 0.1f;
		auto particle = Particle(position, glm::vec3(0.0f), -0.98f, gravityEffect);
		particles.push_back(particle);
	}
}

void prepareGeometryData() {

	Vertex vertices[] = {
		// Back
		{{-0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}}, // 0
		{{ 0.5f, -0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}}, // 1
		{{ 0.5f,  0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}}, // 2
		{{-0.5f,  0.5f, -0.5f}, { 0.0f, 0.0f, -1.0f}}, // 3

		// Front
		{{-0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}}, // 4
		{{ 0.5f, -0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}}, // 5
		{{ 0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}}, // 6
		{{-0.5f,  0.5f,  0.5f}, { 0.0f,  0.0f,  1.0f}}, // 7

		// Left
		{{-0.5f,  0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f }}, // 8
		{{-0.5f,  0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f }}, // 9
		{{-0.5f, -0.5f, -0.5f}, {-1.0f,  0.0f,  0.0f }}, // 10
		{{-0.5f, -0.5f,  0.5f}, {-1.0f,  0.0f,  0.0f }}, // 11

		// Right
		{{ 0.5f,  0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f }}, // 12
		{{ 0.5f,  0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f }}, // 13
		{{ 0.5f, -0.5f, -0.5f}, { 1.0f,  0.0f,  0.0f }}, // 14
		{{ 0.5f, -0.5f,  0.5f}, { 1.0f,  0.0f,  0.0f }}, // 15

		{{-0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f }}, // 16
		{{ 0.5f, -0.5f, -0.5f}, { 0.0f, -1.0f,  0.0f }}, // 17
		{{ 0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f }}, // 18
		{{-0.5f, -0.5f,  0.5f}, { 0.0f, -1.0f,  0.0f }}, // 19

		{{-0.5f,  0.5f, -0.5f},  { 0.0f,  1.0f,  0.0f}}, // 20
		{{ 0.5f,  0.5f, -0.5f},  { 0.0f,  1.0f,  0.0f}}, // 21
		{{ 0.5f,  0.5f,  0.5f},  { 0.0f,  1.0f,  0.0f}}, // 22
		{{-0.5f,  0.5f,  0.5f},  { 0.0f,  1.0f,  0.0f}}, // 23
	};

	uint32_t indices[] = {
		0, 1, 2,
		2, 3, 0,
		4, 5, 6,
		6, 7, 4,
		8, 9, 10,
		10, 11, 8,
		12, 13, 14,
		14, 15, 12,
		16, 17, 18,
		18, 19, 16,
		20, 21, 22,
		22, 23, 20
	};

	// Create the buffer objects
	uint32_t lightCubeVBO;
	glGenBuffers(1, &lightCubeVBO);

	uint32_t lightCubeIBO;
	glGenBuffers(1, &lightCubeIBO);

	// Create and setup the vertex array object
	glGenVertexArrays(1, &lightCubeVAO);
	glBindVertexArray(lightCubeVAO);

	glBindBuffer(GL_ARRAY_BUFFER, lightCubeVBO);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	int32_t stride = sizeof(Vertex);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)nullptr);
	// Map index 0 to the position buffer
	glEnableVertexAttribArray(0);	// Vertex Position

	// Enable the vertex attribute arrays
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
	// Map index 1 to the tangent buffer
	glEnableVertexAttribArray(1);	// tangent

	// Enable the vertex attribute arrays
	glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
	// Map index 1 to the binormal buffer
	glEnableVertexAttribArray(2);	// binormal

	// Enable the vertex attribute arrays
	glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 9));
	// Map index 1 to the normal buffer
	glEnableVertexAttribArray(3);	// normal

	glVertexAttribPointer(4, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 12));
	// Map index 1 to the texture coordinate buffer
	glEnableVertexAttribArray(4);	//

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, lightCubeIBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

	createScreenQuad();
	createParticleQuad();
}

void initImGui() {
	// Setup Dear ImGui context.
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO(); (void)io;
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
	//io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

	// Setup Dear ImGui style.
	ImGui::StyleColorsDark();
	ImGui::GetStyle().ScaleAllSizes(1.0f);
	// ImGui::StyleColorsClassic();

	// Setup platform/Renderer bindings.
	const char* glsl_version = "#version 400";
	ImGui_ImplGlfw_InitForOpenGL(window, false);
	ImGui_ImplOpenGL3_Init(glsl_version);

	// Load Fonts
	// - If no fonts are loaded, dear imgui will use the default font. You can also load multiple fonts and use ImGui::PushFont()/PopFont() to select them.
	// - AddFontFromFileTTF() will return the ImFont* so you can store it if you need to select the font among multiple.
	// - If the file cannot be loaded, the function will return NULL. Please handle those errors in your application (e.g. use an assertion, or display an error and quit).
	// - The fonts will be rasterized at a given size (w/ oversampling) and stored into a texture when calling ImFontAtlas::Build()/GetTexDataAsXXXX(), which ImGui_ImplXXXX_NewFrame below will call.
	// - Read 'misc/fonts/README.txt' for more instructions and details.
	// - Remember that in C/C++ if you want to include a backslash \ in a string literal you need to write a double backslash \\ !
	//io.Fonts->AddFontDefault();
	//fonts.push_back(io.Fonts->AddFontFromFileTTF("./resources/imgui/misc/fonts/Roboto-Medium.ttf", 10.0f));
	//fonts.push_back(io.Fonts->AddFontFromFileTTF("./resources/imgui/misc/fonts/Cousine-Regular.ttf", 10.0f));
	//fonts.push_back(io.Fonts->AddFontFromFileTTF("./resources/imgui/misc/fonts/DroidSans.ttf", 10.0f));
	//fonts.push_back(io.Fonts->AddFontFromFileTTF("./resources/imgui/misc/fonts/ProggyTiny.ttf", 10.0f));
	//ImFont* font = io.Fonts->AddFontFromFileTTF("c:\\Windows\\Fonts\\ArialUni.ttf", 18.0f, NULL, io.Fonts->GetGlyphRangesJapanese());
	//IM_ASSERT(font != NULL);
}

void showMenuBar() {
	// Menu Bar
	if (ImGui::BeginMenuBar())
	{
		if (ImGui::BeginMenu("File"))
		{
			if (ImGui::MenuItem("Open", "Ctrl+O")) {
			}

			ImGui::EndMenu();
		}

		ImGui::EndMenuBar();
	}
}

const auto& getTexture(const std::string& name) {
	return textures[name];
}

auto createTexture(const std::string& name, int32_t width, int32_t height, int32_t filter = GL_LINEAR, int32_t internalFormat = GL_RGBA, int32_t format = GL_RGBA) {
	return std::make_shared<Texture>(name, width, height, filter, internalFormat, format);
}

auto addTexture(const std::string& name, const std::string& path, int32_t wrapMode = GL_REPEAT) {

	auto texture = getTexture(name);

	if (texture) {
		return texture;
	}

	//if (std::filesystem::exists(path)) {
		texture = std::make_shared<Texture>(path, wrapMode);
		textures[name] = texture;
	//}

	return texture;
}

auto addCubemapTexture(const std::string& name, const std::string& path, int32_t wrapMode = GL_REPEAT, bool cubeMap = false, bool hdr = false) {

	auto texture = getTexture(name);

	if (texture) {
		return texture;
	}

	texture = std::make_shared<Texture>(path, wrapMode, cubeMap, hdr);
	if (texture) {
		textures[name] = texture;
		return texture;
	}

	return std::make_shared<Texture>();
}

void addMaterial(const std::string& name, const std::shared_ptr<Material>& material) {
	materials[name] = material;
}

const auto& getMaterial(const std::string& name) {
	return materials[name];
}

std::shared_ptr<Model> loadModel(const std::string& fileName, const std::string& inName, const std::string& materialPath, const std::string& texturePath) {

	tinyobj::ObjReaderConfig readConfig;
	readConfig.mtl_search_path = materialPath;

	tinyobj::ObjReader reader;

	if (!reader.ParseFromFile(fileName, readConfig)) {
		if (!reader.Error().empty()) {
			std::cerr << "TinyObjRead: " << reader.Error();
		}
		return nullptr;
	}

	if (!reader.Warning().empty()) {
		std::cout << "TinyObjReader: " << reader.Warning();
	}

	auto slash = fileName.find_last_of('/');
	auto dot = fileName.find_last_of('.');

	auto model = std::make_shared<Model>();

	if (!inName.empty()) {
		model->setName(inName);
	}
	else {
		model->setName(fileName.substr(slash + 1, dot - (slash + 1)));
	}

	auto& attrib = reader.GetAttrib();
	auto& shapes = reader.GetShapes();
	auto& objMaterials = reader.GetMaterials();

	size_t materialIndex = 0;

	for (const auto& shape : shapes) {
		auto mesh = std::make_shared<Mesh>();
		std::unordered_map<Vertex, uint32_t> uniqueVertices;
		for (const auto& index : shape.mesh.indices) {
			Vertex vertex = {};

			vertex.position = {
				attrib.vertices[3 * index.vertex_index + 0],
				attrib.vertices[3 * index.vertex_index + 1],
				attrib.vertices[3 * index.vertex_index + 2]
			};

			// Check if 'normal_index' is zero of positive. negative = no normal data
			if (index.normal_index >= 0) {
				tinyobj::real_t nx = attrib.normals[3 * size_t(index.normal_index) + 0];
				tinyobj::real_t ny = attrib.normals[3 * size_t(index.normal_index) + 1];
				tinyobj::real_t nz = attrib.normals[3 * size_t(index.normal_index) + 2];
				vertex.normal = { nx, ny, nz };
			}

			if (index.texcoord_index >= 0) {
				vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]
				};
			}

			if (uniqueVertices.count(vertex) == 0) {
				uniqueVertices[vertex] = static_cast<uint32_t>(mesh->getVertices().size());
				mesh->addVertex(vertex);
			}

			mesh->addIndex(uniqueVertices[vertex]);
		}

		mesh->setName(shape.name);

		const auto& material = objMaterials[materialIndex];

		auto meshMaterial = std::make_shared<Material>();

		meshMaterial->Ka = { material.ambient[0], material.ambient[1], material.ambient[2] };
		meshMaterial->Kd = { material.diffuse[0], material.diffuse[1], material.diffuse[2] };
		meshMaterial->Ke = { material.emission[0], material.emission[2], material.emission[2] };
		meshMaterial->Ks = { material.specular[0], material.specular[1], material.specular[2] };

		meshMaterial->shininess = material.shininess;
		meshMaterial->ior = material.ior;
		meshMaterial->eta = 1.0f / meshMaterial->ior;

		if (!material.diffuse_texname.empty()) {
			auto texture = addTexture(material.name + "Diffuse", texturePath + material.diffuse_texname, GL_CLAMP_TO_EDGE);

			if (texture) {
				mesh->addTexture(texture);
			}

		}
		else {
			mesh->addTexture(defaultAlbedo);
		}

		if (!material.bump_texname.empty()) {
			auto texture = addTexture(material.name + "Normal", texturePath + material.bump_texname);

			if (texture) {
				mesh->addTexture(texture);
				meshMaterial->hasNormalMap = true;
			}
		}

		mesh->setMaterial(std::move(meshMaterial));

		materialIndex++;

		model->addMesh(std::move(mesh));
	}

	return model;
}

void buildImGuiWidgets() {

	// 1. Show the big demo window (Most of the sample code is in ImGui::ShowDemoWindow()! You can browse its code to learn more about Dear ImGui!).
	if (bShowDemoWindow)
		ImGui::ShowDemoWindow(&bShowDemoWindow);

	frameTime = 1.0f / ImGui::GetIO().Framerate;
	elapsedTime += frameTime;

	// 2. Show a simple window that we create ourselves. We use a Begin/End pair to created a named window.
	if (bShowImGUIWindow)
	{
		//static float f = 0.0f;
		static int counter = 0;

		ImGui::Begin("Hello, world!", nullptr, ImGuiWindowFlags_MenuBar);                          // Create a window called "Hello, world!" and append into it.

		showMenuBar();

		ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)
		//ImGui::PushFont(fonts[2]);
		ImGui::Text("This is some useful text use another font.");
		//ImGui::PopFont();
		//ImGui::Checkbox("Demo Window", &bShowDemoWindow);      // Edit bools storing our window open/close state
		//ImGui::Checkbox("Another Window", &bShowAnotherWindow);

		auto commonMaterial = getMaterial("Common");

		ImGui::ColorEdit3("Ambient", (float*)&commonMaterial->Ka); // Edit 1 float using a slider from 0.1f to 1.0f
		ImGui::SliderFloat("Shininess", &commonMaterial->shininess, 32.0f, 128.0f);
		ImGui::Checkbox("Fog", &bFog);

		if (bFog)
		{
			fog.density = 0.05f;
		}
		else
		{
			fog.density = 0.05f;
		}

		ImGui::SliderFloat("Fog Density", &fog.density, 0.0f, 1.0f);
		ImGui::SliderFloat("Scale", &globalScale, -1.0f, 1.0f);
		ImGui::SliderFloat("Ambient Intensity", &ambientIntensity, 0.0f, 1.0f);
		//ImGui::ColorEdit3("Clear color", (float*)&clearColor); // Edit 3 floats representing a color
		//ImGui::ColorEdit3("Point Light Color", (float*)&lights[0].color);
		ImGui::SliderFloat("Light0 Intensity", &lights[0].intensity, 0.0f, 1.0f);
		ImGui::ColorEdit3("Light0 Color", (float*)&lights[0].color);
		//ImGui::ColorEdit3("Light4 Color", (float*)&lights[4].color);
		//ImGui::SliderFloat("Light4 Intensity", &lights[4].intensity, 0.0f, 1.0f);
		//ImGui::SliderFloat("Shadowmap Bias", &shadowmapBias, 0.0f, 0.05f);
		ImGui::DragFloat3("Light Direction", (float*)&lights[0].position, 0.1f, -1.0f, 1.f);
		//ImGui::DragFloat3("Light0 Position", (float*)&lights[1].position, 0.1f, -10.0f, 10.f);
		//ImGui::Checkbox("Projective Texture Mapping", &bShowProjector);
		ImGui::Checkbox("Draw Normals", &bDrawNormals);

		if (ImGui::Button("Button"))                            // Buttons return true when clicked (most widgets return true when edited/activated)
			counter++;
		ImGui::SameLine();
		ImGui::Text("counter = %d", counter);

		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);

		ImGui::Text("Camera Position %f, %f, %f", mainCamera.getEye().x, mainCamera.getEye().y, mainCamera.getEye().z);
		ImGui::End();
	}

	// 3. Show another simple window.
	if (bShowAnotherWindow)
	{
		ImGui::Begin("Another Window", &bShowAnotherWindow);   // Pass a pointer to our bool variable (the window will have a closing button that will clear the bool when clicked)

		ImGui::Text("Hello from another window!");
		if (ImGui::Button("Close Me"))
			bShowAnotherWindow = false;
		ImGui::End();
	}
}


void bindCallbacks() {
	glfwSetFramebufferSizeCallback(window, onFrameBufferResize);
	glfwSetKeyCallback(window, onKeyCallback);
	glfwSetMouseButtonCallback(window, onMouseButtonCallback);
	glfwSetScrollCallback(window, onScrollCallback);
	glfwSetCursorPosCallback(window, onMouseMoveCallback);
}

float timeStep = 0.0f;
float spherTimeStep = 0.0f;
int sign = 1;

glm::vec3 g_vStartPoint = { 0.0f, -1.5f, -10.0f };   // This is the starting point of the curve
glm::vec3 g_vControlPoint1 = { 0.0f, 4.0f, -10.0f }; // This is the first control point of the curve
glm::vec3 g_vControlPoint2 = { 3.0f, 4.0f, -10.0f }; // This is the second control point of the curve
glm::vec3 g_vEndPoint = { 3.0f, -1.5f, -10.0f };      // This is the end point of the curve

glm::vec3 g_vSphereStartPoint = { -4.0f, 3.0f, -2.0f };   // This is the starting point of the curve
glm::vec3 g_vSphereControlPoint1 = { -2.0f, 6.0f, 0.0f }; // This is the first control point of the curve
glm::vec3 g_vSphereControlPoint2 = { 2.0f, 0.0f, -4.0f }; // This is the second control point of the curve
glm::vec3 g_vSphereEndPoint = { 4.0f, 3.0f, -2.0f };      // This is the end point of the curve

glm::vec3 PointOnCurve(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3, glm::vec3 p4, float t) {

	float var1, var2, var3;
	glm::vec3 vPoint = { 0.0f, 0.0f, 0.0f };

	// Here is the juice of our tutorial.  Below is the equation for a 4 control point
	// bezier curve:
	// B(t) = P1 * ( 1 - t )^3 + P2 * 3 * t * ( 1 - t )^2 + P3 * 3 * t^2 * ( 1 - t ) + P4 * t^3
	// Yes I agree, this isn't the most intuitive equation, but it is pretty straight forward.
	// If you got up to Trig, you will notice that this is a polynomial.  That is what a curve is.
	// "t" is the time from 0 to 1.  You could also think of it as the distance along the
	// curve, because that is really what it is.  P1 - P4 are the 4 control points.
	// They each have an (x, y, z) associated with them.  You notice that there is a lot of
	// (1 - t) 's?  Well, to clean up our code we will try to contain some of these
	// repetitions into variables.  This helps our repeated computations as well.

	// Store the (1 - t) in a variable because it is used frequently
	var1 = 1 - t;

	// Store the (1 - t)^3 into a variable to cut down computation and create clean code
	var2 = var1 * var1 * var1;

	// Store the t^3 in a variable to cut down computation and create clean code
	var3 = t * t * t;

	// Now that we have some computation cut down, we just follow the equation above.
	// If you multiply and simplify the equation, you come up with what we have below.
	// If you don't see how we came to here from the equation, multiply the equation
	// out and it will become more clear.  I don't intend to go into any more detail
	// on the math of a bezier curve, because there are far better places out there
	// with graphical displays and tons of examples.  Look in our * Quick Notes *
	// for an EXCELLENT web site that does just this.  It derives everything and has
	// excellent visuals.  It's the best I have seen so far.
	vPoint.x = var2 * p1.x + 3 * t * var1 * var1 * p2.x + 3 * t * t * var1 * p3.x + var3 * p4.x;
	vPoint.y = var2 * p1.y + 3 * t * var1 * var1 * p2.y + 3 * t * t * var1 * p3.y + var3 * p4.y;
	vPoint.z = var2 * p1.z + 3 * t * var1 * var1 * p2.z + 3 * t * t * var1 * p3.z + var3 * p4.z;

	// Now we should have the point on the curve, so let's return it.
	return (vPoint);
}

void updateSnowman() {

	for (int i = 0; i < 6; i += 2) {
		auto snowmanPosition = snowmen[i]->getPosition();

		if (snowmanPosition.y > -1.8f) {
			snowmanPosition += gravity * frameTime;
			snowmen[i]->setPosition(snowmanPosition);
			snowmen[i + 1]->setPosition(snowmanPosition);

			snowmen[i]->setTransform(glm::rotate(snowmen[i]->getTransform(), glm::radians(snowmanYawRates[i / 2]), glm::vec3(0.0f, 1.0f, 0.0f)));
			snowmen[i + 1]->setTransform(glm::rotate(snowmen[i]->getTransform(), glm::radians(snowmanYawRates[i / 2]), glm::vec3(0.0f, 1.0f, 0.0f)));
		}
		else {
			snowmen[i]->bJumping = false;
		}
	}

	for (int i = 0; i < 6; i += 2) {
		auto snowman = snowmen[i];
		auto snowmanArm = snowmen[i + 1];

		auto transform = glm::rotate(snowman->getTransform(), glm::radians(snowmenRollRate[i / 2] * frameTime), glm::vec3(0.0f, 0.0f, 1.0f));

		if (toggleSnowmanAnimation)
		{
			snowman->setTransform(transform);
			snowmanArm->setTransform(transform);
			snowman->roll += snowmenRollRate[i / 2] * frameTime;
			snowmanArm->roll += snowmenRollRate[i / 2] * frameTime;

			if (snowman->roll > 5.0f) {
				snowman->roll = 5.0f;
				snowmanArm->roll = 5.0f;
				snowmenRollRate[i / 2] = -snowmenRollRate[i / 2];
			}

			if (snowman->roll < -5.0f) {
				snowman->roll = -5.0f;
				snowmanArm->roll = -5.0f;
				snowmenRollRate[i / 2] = -snowmenRollRate[i / 2];
			}
		}
	}

	if (activeSnowman) {
		auto position = activeSnowman->getPosition();

		position += activeSnowmanVelocity;

		activeSnowman->setPosition(position);
		activeSnowmanArm->setPosition(position);
	}
}

void updateSkybox() {

	timeOfDay += frameTime;

	skyboxShader->use();

	if (timeOfDay < 10.0f) {
		skyboxShader->setUniform("skybox1", skyboxDay->getTextureIndex());
	}

	if (timeOfDay >= 10.0f) {
		skyboxShader->setUniform("skybox1", skyboxDay->getTextureIndex());
		skyboxShader->setUniform("skybox2", skyboxDusk->getTextureIndex());
	}

	if ((10.0f < timeOfDay) && (timeOfDay < 15.0f)) {
		skyboxShader->setUniform("alpha", (timeOfDay - 10.0f) / 5.0f);
		ambientIntensity -= frameTime * 0.04f;
	}

	if (timeOfDay >= 25.0f) {
		skyboxShader->setUniform("skybox1", skyboxDusk->getTextureIndex());
		skyboxShader->setUniform("skybox2", skyboxNight->getTextureIndex());

		//currentHouseTexture = houseLightTexture;
	}

	if ((25.0f < timeOfDay) && (timeOfDay < 30.0f)) {
		skyboxShader->setUniform("alpha", (timeOfDay - 25.0f) / 5.0f);
		ambientIntensity -= frameTime * 0.04f;
	}

	if (timeOfDay >= 40.0f) {
		skyboxShader->setUniform("skybox1", skyboxNight->getTextureIndex());
		skyboxShader->setUniform("skybox2", skyboxDay->getTextureIndex());
	}

	if ((40.0f < timeOfDay) && (timeOfDay < 45.0f)) {
		skyboxShader->setUniform("alpha", (timeOfDay - 40.0f) / 5.0f);
		ambientIntensity += frameTime * 0.08f;
	}

	if (timeOfDay > 45.0f) {
		//currentHouseTexture = houseTexture;
		timeOfDay = 0.0f;
	}
}

void updateSmoke() {

	for (auto j = 0; j < 2; j++) {
		auto& smokes = smokesGroup[j];

		for (int32_t i = 0; i < static_cast<int32_t>(smokes.size()); i++) {
			auto positon = smokes[i]->getPosition();
			positon.y += frameTime;
			smokes[i]->setPosition(positon);

			auto scale = smokes[i]->getScale();

			scale += frameTime / (i + 1);
			smokes[i]->setScale(scale);

			smokeTimers[i] += frameTime;

			if (smokes[i]->getPosition().y >= 7.0f)
			{
				//smokes.erase(smokes.begin());
				//auto smoke = createSmoke(0.125f, smokeSpwanPositions[i]);
				auto smokeSpawnPosition = glm::vec3(0.0);

				if (j == 0) {
					smokeSpawnPosition = smokeSpawnPositions1[2];
				}
				else {
					smokeSpawnPosition = smokeSpawnPositions2[2];
				}
				smokes[i]->setScale(glm::vec3(1.0f));
				smokes[i]->setPosition(smokeSpawnPosition);
				smokeTimers[i] = 0.0f;
			}
		}
	}
}

void updateChristmasTreeLight() {

	lightTimer += frameTime;

	if (lightTimer >= 1.0) {
		toggleLightIndex = (toggleLightIndex + 1) % 3;

		for (auto i = 0; i < 6; i++) {
			if (bActivatedChristmasTreeLight[i]) {
				for (auto j = 0; j < 3; j++) {
					bToggleChirstmasTreeLights[i][j] = false;
					if (toggleLightIndex == j) {
						bToggleChirstmasTreeLights[i][j] = true;
					}
				}
			}
		}

		lightTimer = 0.0f;
	}
}

void updateParticles() {

	for (auto iterator = particles.begin(); iterator != particles.end();) {
		if (!(*iterator).update(frameTime)) {
			iterator = particles.erase(iterator);
		}
		else {
			iterator++;
		}
	}

	if (particles.size() < 1500) {
		spawnParticles(500);
	}
}

void update() {

	rotateCenter = glm::vec3(0.0f);

	// Orbit light
	float x0 = (lights[1].position.x - rotateCenter.x) * std::cos(cameraRotateRate) - (lights[1].position.z - rotateCenter.z) * std::sin(cameraRotateRate) + rotateCenter.x;
	float z0 = (lights[1].position.x - rotateCenter.x) * std::sin(cameraRotateRate) + (lights[1].position.z - rotateCenter.z) * std::cos(cameraRotateRate) + rotateCenter.z;

	lights[1].position.x = x0;
	lights[1].position.z = z0;

	//lightSphere->setPosition(glm::vec3(x0, lightSphere->getPosition().y, z0));

 //   rotateCenter = glm::vec3(teapot->getPosition());

 //   auto position = cubemapSphere->getPosition();

	//x0 = (position.x - rotateCenter.x) * std::cos(cameraRotateRate) - (position.z - rotateCenter.z) * std::sin(cameraRotateRate) + rotateCenter.x;
	//z0 = (position.x - rotateCenter.x) * std::sin(cameraRotateRate) + (position.z - rotateCenter.z) * std::cos(cameraRotateRate) + rotateCenter.z;

 //   position.x = x0;
 //   position.z = z0;
 //   
 //   cubemapSphere->setPosition(position);

	// Bezier Curve animation
	if (bPlayMerryChristmasAnimation) {
		timeStep += frameTime * 0.5f;
		glm::vec3 vPoint = PointOnCurve(g_vStartPoint, g_vControlPoint1, g_vControlPoint2, g_vEndPoint, timeStep);
		//movingSphere->setPosition(vPoint);
		merryChristmasSnowman->setPosition(vPoint);
		merryChristmasSnowmanArm->setPosition(vPoint);
		flagpole->setPosition(vPoint);
		flag->setPosition(vPoint);
	}

	if (!bPlayMerryChristmasAnimation && timeStep > 0.0f) {
		timeStep -= frameTime * 0.5f;
		glm::vec3 vPoint = PointOnCurve(g_vStartPoint, g_vControlPoint1, g_vControlPoint2, g_vEndPoint, timeStep);
		//movingSphere->setPosition(vPoint);
		merryChristmasSnowman->setPosition(vPoint);
		merryChristmasSnowmanArm->setPosition(vPoint);
		flagpole->setPosition(vPoint);
		flag->setPosition(vPoint);
	}

	spherTimeStep += frameTime * 0.5f * sign;

	auto vSpherePoint = PointOnCurve(g_vSphereStartPoint, g_vSphereControlPoint1, g_vSphereControlPoint2, g_vSphereEndPoint, spherTimeStep);

	movingSphere->setPosition(vSpherePoint);

	if (spherTimeStep > 1.0f) {
		sign = -sign;
		//spherTimeStep = 1.0f;
	}

	if (-1.0f > spherTimeStep) {
		sign = -sign;
		//spherTimeStep = 0.0f;
	}

	if (timeStep > 1.0f) {
		//sign = !sign;
		timeStep = 1.0f;
	}

	if (0.0f > timeStep) {
		//sign = !sign;
		timeStep = 0.0f;
		giftBoxCover->setPosition(glm::vec3(0.0f, -1.5f, -10.0f));
	}

	updateSnowman();

	updateSkybox();

	updateSmoke();

	updateChristmasTreeLight();

	updateParticles();

	// Start the Dear ImGui frame
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	buildImGuiWidgets();
}

void renderImGui() {
	// Rendering
	ImGui::Render();

	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}

void updateGlobalUniform() {

	sceneShader->use();

	for (auto i = 0; i < 5; i++)
	{
		if (lightSwitchs[i])
		{
			lights[i].intensity = lightIntensitys[i];
		}
		else
		{
			lights[i].intensity = 0.0f;
		}
	}

	sceneShader->setUniform("lights[0].color", lights[0].color);
	sceneShader->setUniform("lights[0].position", lights[0].position);
	sceneShader->setUniform("lights[0].intensity", lights[0].intensity);
	sceneShader->setUniform("lights[0].Kc", lights[0].Kc);
	sceneShader->setUniform("lights[0].Kl", lights[0].Kl);
	sceneShader->setUniform("lights[0].Kq", lights[0].Kq);
	//sceneShader->setUniform("lights[0].type", lights[0].type);

	sceneShader->setUniform("lights[1].color", lights[1].color);
	sceneShader->setUniform("lights[1].position", lights[1].position);
	sceneShader->setUniform("lights[1].intensity", lights[1].intensity);
	sceneShader->setUniform("lights[1].Kc", lights[1].Kc);
	sceneShader->setUniform("lights[1].Kl", lights[1].Kl);
	sceneShader->setUniform("lights[1].Kq", lights[1].Kq);
	//sceneShader->setUniform("lights[1].type", lights[1].type);

	sceneShader->setUniform("lights[2].color", lights[2].color);
	sceneShader->setUniform("lights[2].position", lights[2].position);
	sceneShader->setUniform("lights[2].intensity", lights[2].intensity);
	sceneShader->setUniform("lights[2].Kc", lights[2].Kc);
	sceneShader->setUniform("lights[2].Kl", lights[2].Kl);
	sceneShader->setUniform("lights[2].Kq", lights[2].Kq);
	//sceneShader->setUniform("lights[2].type", lights[2].type);

	sceneShader->setUniform("lights[3].color", lights[3].color);
	sceneShader->setUniform("lights[3].position", lights[3].position);
	sceneShader->setUniform("lights[3].intensity", lights[3].intensity);
	//sceneShader->setUniform("lights[3].type", lights[3].type);

	sceneShader->setUniform("lights[4].color", lights[4].color);
	sceneShader->setUniform("lights[4].position", lights[4].position);
	sceneShader->setUniform("lights[4].direction", lights[4].direction);
	//sceneShader->setUniform("lights[4].exponent", lights[4].exponent);
	sceneShader->setUniform("lights[4].cutoff", glm::cos(glm::radians(lights[4].cutoff)));
	sceneShader->setUniform("lights[4].outerCutoff", glm::cos(glm::radians(lights[4].outerCutoff)));
	sceneShader->setUniform("lights[4].intensity", lights[4].intensity);
	//sceneShader->setUniform("lights[4].type", lights[4].type);

	//sceneShader->setUniform("fog.minDistance", fog.minDistance);
	//sceneShader->setUniform("fog.maxDistance", fog.maxDistance);
	sceneShader->setUniform("fog.density", fog.density);
	sceneShader->setUniform("fog.color", fog.color);

	sceneShader->setUniform("skybox1", currentSkybox->getTextureIndex());

	sceneShader->setUniform("eye", mainCamera.getEye());

	sceneShader->setUniform("projectorTransform", projectorTransform);

	//sceneShader->setUniform("projection", getTexture("Projection")->getTextureIndex());

	sceneShader->setUniform("showProjector", bShowProjector);

	sceneShader->setUniform("ambientIntensity", ambientIntensity);
}

void drawSkybox(const glm::mat4& inViewMatrix, const glm::mat4& inProjectionMatrix) {

	skyboxShader->use();

	auto& model = models[0];

	auto& mesh = model->getMeshes()[0];

	mesh->use();

	glm::mat4 viewMatrix = inViewMatrix;

	// 消除平移部分，使天空盒和玩家一起移动(无限远的效果)
	viewMatrix[3][0] = 0.0f;
	viewMatrix[3][1] = 0.0f;
	viewMatrix[3][2] = 0.0f;

	glm::mat4 worldMatrix = model->getTransform();

	worldMatrix = glm::rotate(worldMatrix, glm::radians(frameTime * skyboxRotateRate), glm::vec3(0.0f, 1.0f, 0.0f));

	model->setTransform(worldMatrix);

	glm::mat4 mvpMatrix = inProjectionMatrix * viewMatrix * worldMatrix;

	skyboxShader->setUniform("mvpMatrix", mvpMatrix);

	glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);
}

void drawLights(const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {

	lightCubeShader->use();

	for (size_t i = 0; i < lights.size(); i++) {
		if (lights[i].type == 0) {
			glm::mat4 worldMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(lights[i].position.x, lights[i].position.y, lights[i].position.z));
			worldMatrix = glm::scale(worldMatrix, glm::vec3(0.2f, 0.2f, 0.2f));

			glm::mat4 mvpMatrix = inProjectionMatrix * inViewMaterix * worldMatrix;

			lightCubeShader->setUniform("mvpMatrix", mvpMatrix);
			lightCubeShader->setUniform("color", glm::vec4(1.0f, 1.0f, 1.0f, 1.0f));

			glBindVertexArray(lightCubeVAO);
			//glDrawArrays(GL_TRIANGLES, 0, 36);
			glDrawElements(GL_TRIANGLES, 64, GL_UNSIGNED_INT, 0);
		}
	}

	sceneShader->use();
}

void updateMaterialUniform(const std::shared_ptr<Material>& material) {

	if (material) {
		sceneShader->setUniform("material.Ka", material->Ka);
		sceneShader->setUniform("material.Kd", material->Kd);
		sceneShader->setUniform("material.Ks", material->Ks);
		sceneShader->setUniform("material.Ke", material->Ke);
		sceneShader->setUniform("material.shininess", material->shininess);
		sceneShader->setUniform("material.reflectionFactor", material->reflectionFactor);
		sceneShader->setUniform("material.refractionFactor", material->refractionFactor);
		//sceneShader->setUniform("material.ior", material->ior);
		//sceneShader->setUniform("material.eta", material->eta);
		sceneShader->setUniform("material.hasNormalMap", material->hasNormalMap);
	}
}

void drawModel(const std::shared_ptr<Model>& model, const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {

	for (auto& mesh : model->getMeshes()) {
		mesh->use();

		auto material = mesh->getMaterial();

		updateMaterialUniform(material);

		sceneShader->setUniform("textures[0]", mesh->getTextureIndex(0));
		sceneShader->setUniform("textures[1]", mesh->getTextureIndex(1));
		sceneShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);
		sceneShader->setUniform("shadowMap", depthMapTexture->getTextureIndex());
		sceneShader->setUniform("shadowmapBias", shadowmapBias);
		// TODO: 延迟渲染的时候记得取消注释
		//sceneShader->setUniform("material.Kd", glm::vec3(0.270588f, 0.552941f, 0.874510f));

		glm::mat4 worldMatrix = model->getTransform();
		worldMatrix = glm::scale(worldMatrix, glm::vec3(1.0f, 1.0f, globalScale));
		glm::mat4 mvpMatrix = inProjectionMatrix * inViewMaterix * worldMatrix;

		sceneShader->setUniform("worldMatrix", worldMatrix);
		//sceneShader->setUniform("viewMatrix", inViewMaterix);
		sceneShader->setUniform("mvpMatrix", mvpMatrix);
		sceneShader->setUniform("projectionMatrix", inProjectionMatrix);
		//sceneShader->setUniform("normalMatrix", glm::transpose(glm::inverse(worldMatrix)));
		sceneShader->setUniform("normalMatrix", worldMatrix);

		glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);
	}
}

void drawDepthModel(const std::shared_ptr<Model>& model, const glm::mat4& inLightSpaceMatrix) {

	for (auto& mesh : model->getMeshes()) {
		mesh->use();

		glm::mat4 worldMatrix = model->getTransform();

		depthShader->setUniform("model", worldMatrix);
		depthShader->setUniform("lightSpaceMatrix", inLightSpaceMatrix);

		glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);
	}
}

void drawRenderWindow(const std::shared_ptr<Model>& model, const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {

	for (auto& mesh : model->getMeshes()) {
		mesh->use();

		textureShader->setUniform("albedo", sceneTexture->getTextureIndex());

		glm::mat4 worldMatrix = model->getTransform();
		glm::mat4 mvpMatrix = inProjectionMatrix * inViewMaterix * worldMatrix;

		textureShader->setUniform("worldMatrix", worldMatrix);
		textureShader->setUniform("mvpMatrix", mvpMatrix);
		textureShader->setUniform("projectionMatrix", inProjectionMatrix);

		glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);
	}
}

void drawModels(const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {

	sceneShader->use();
	sceneShader->setUniform("drawSkybox", false);

	for (size_t i = 1; i < models.size(); i++) {
		drawModel(models[i], inViewMaterix, inProjectionMatrix);
	}
}

void drawDepthModels(const glm::mat4& inLightSpaceMatrix) {

	for (size_t i = 1; i < models.size(); i++) {
		drawDepthModel(models[i], inLightSpaceMatrix);
	}
}

void drawSmokes(const std::vector<std::shared_ptr<Model>>& inMmodels, glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {

	for (size_t i = 0; i < inMmodels.size(); i++) {
		drawModel(inMmodels[i], inViewMaterix, inProjectionMatrix);
	}
}

void drawDepthSmokes(const std::vector<std::shared_ptr<Model>>& inMmodels, const glm::mat4& inLightSpaceMatrix) {

	for (size_t i = 0; i < inMmodels.size(); i++) {
		drawDepthModel(inMmodels[i], inLightSpaceMatrix);
	}
}

void drawChristmasTreeBall(const std::shared_ptr<Model>& model, const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix, bool bLight = false, const glm::vec4& lightColor = glm::vec4(1.0f)) {

	for (auto& mesh : model->getMeshes()) {
		mesh->use();

		decorationShader->setUniform("albedo", mesh->getTextureIndex(0));
		decorationShader->setUniform("bLight", bLight);
		decorationShader->setUniform("lightColor", lightColor);

		glm::mat4 worldMatrix = model->getTransform();
		glm::mat4 mvpMatrix = inProjectionMatrix * inViewMaterix * worldMatrix;

		decorationShader->setUniform("worldMatrix", worldMatrix);
		decorationShader->setUniform("mvpMatrix", mvpMatrix);

		glDrawElements(GL_TRIANGLES, mesh->getIndexCount(), GL_UNSIGNED_INT, 0);
	}
}

void drawNormals(const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {

	lightCubeShader->use();

	lightCubeShader->setUniform("color", glm::vec4(0.270588f, 0.552941f, 0.874510f, 1.0f));

	for (size_t i = 1; i < models.size(); i++) {
		glm::mat4 worldMatrix = models[i]->getTransform();
		for (auto& mesh : models[i]->getMeshes()) {
			mesh->useNormal();

			glm::mat4 mvpMatrix = inProjectionMatrix * inViewMaterix * worldMatrix;

			lightCubeShader->setUniform("mvpMatrix", mvpMatrix);
			glDrawArrays(GL_LINES, 0, mesh->getNormalIndexCount());
		}
	}
}

void clear(Vec4f color, int32_t clearFlag) {
	glClearColor(color.x, color.y, color.z, 1.0f);
	glClear(clearFlag);
}

//void clear(float r, float g, float b, int32_t clearFlag) {
//    glClearColor(r, g, b, 1.0f);
//    glClear(clearFlag);
//}

void drawScene(glm::mat4 viewMatrix, glm::mat4 projectionMatrix) {

	drawModels(viewMatrix, projectionMatrix);
	drawSmokes(smokesGroup[0], viewMatrix, projectionMatrix);
	drawSmokes(smokesGroup[1], viewMatrix, projectionMatrix);

	drawModel(merryChristmasSnowman, viewMatrix, projectionMatrix);
	drawModel(merryChristmasSnowmanArm, viewMatrix, projectionMatrix);

	drawModel(flagpole, viewMatrix, projectionMatrix);
	drawModel(flag, viewMatrix, projectionMatrix);

	decorationShader->use();
	for (auto i = 0; i < 6; i++) {
		bool bActivated = bActivatedChristmasTreeLight[i];

		if (bActivated) {
			drawChristmasTreeBall(redBalls[i], viewMatrix, projectionMatrix, bToggleChirstmasTreeLights[i][0], glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			drawChristmasTreeBall(goldenBalls[i], viewMatrix, projectionMatrix, bToggleChirstmasTreeLights[i][1], glm::vec4(1.0f, 0.93f, 0.39f, 1.0f));
			drawChristmasTreeBall(purpleBalls[i], viewMatrix, projectionMatrix, bToggleChirstmasTreeLights[i][2], glm::vec4(0.94f, 0.55f, 0.92f, 1.0f));
		}
		else {
			drawChristmasTreeBall(redBalls[i], viewMatrix, projectionMatrix, false, glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));
			drawChristmasTreeBall(goldenBalls[i], viewMatrix, projectionMatrix, false, glm::vec4(1.0f, 0.93f, 0.39f, 1.0f));
			drawChristmasTreeBall(purpleBalls[i], viewMatrix, projectionMatrix, false, glm::vec4(0.94f, 0.55f, 0.92f, 1.0f));
		}
	}
	sceneShader->use();
}

void renderToTexture(uint32_t width = 512, uint32_t height = 512) {

	// Bind to texture's FBO
	glBindFramebuffer(GL_FRAMEBUFFER, renderSceneFBO);
	glViewport(0, 0, width, height); // Viewport for the texture

	glm::mat4 viewMatrix = mainCamera.getViewMatrix();
	mainCamera.perspective(fov, aspect, nearPlane, farPlane);
	glm::mat4 projectionMatrix = mainCamera.getProjectionMatrix();

	clear(clearColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	sceneShader->setUniform("skybox1", currentSkybox->getTextureIndex());
	skyboxShader->setUniform("skybox1", currentSkybox->getTextureIndex());

	drawSkybox(viewMatrix, projectionMatrix);
	drawScene(viewMatrix, projectionMatrix);

	//drawModel(teapot, viewMatrix, projectionMatrix);
	//drawModel(cubemapSphere, viewMatrix, projectionMatrix);
	drawModel(terrain, viewMatrix, projectionMatrix);

	//drawLights(viewMatrix, projectionMatrix);

	//reflectionTextureShader->use();
	//drawReflectionModel(reflectionFloor, viewMatrix, projectionMatrix);

	// Unbind texture's FBO (back to default FB)
	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	sceneShader->use();
}

//void drawScreenQuad(const std::shared_ptr<Texture>& renderTexture) {
//    screenQuadShader->use();
//
//    glBindVertexArray(screenQuadVAO);
//
//    mainCamera.orthographic(0.0f, static_cast<float>(WindowWidth), 0.0f, static_cast<float>(WindowHeight));
//    glm::mat4 projectionMatrix = mainCamera.getProjectionMatrix();
//
//    screenQuadShader->use();
//    screenQuadShader->setUniform("projectionMatrix", projectionMatrix);
//    screenQuadShader->setUniform("renderTexture", renderTexture->getTextureIndex());
//
//    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
//}

void drawScreenQuad(const std::shared_ptr<Texture>& renderTexture) {

	screenQuadShader->use();

	glBindVertexArray(screenQuadVAO);

	screenQuadShader->use();
	screenQuadShader->setUniform("near_plane", 1.0f);
	screenQuadShader->setUniform("far_plane", 7.5f);
	screenQuadShader->setUniform("depthMap", renderTexture->getTextureIndex());

	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void drawParticle(const Particle& particle, const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {

	glBindVertexArray(particleQuadVAO);

	particleShader->setUniform("albedo", snowflakesTexture->getTextureIndex());

	auto forward = glm::normalize(mainCamera.getEye() - particle.getPosition());
	auto up = glm::vec3(0.0f, 1.0f, 0.0f);
	auto right = glm::cross(forward, up);

	//up = glm::cross(right, forward);

	auto rotation = glm::mat4(1.0);

	rotation[0][0] = right.x;
	rotation[1][0] = right.y;
	rotation[2][0] = right.z;

	rotation[0][1] = up.x;
	rotation[1][1] = up.y;
	rotation[2][1] = up.z;

	rotation[0][2] = forward.x;
	rotation[1][2] = forward.y;
	rotation[2][2] = forward.z;

	auto yawTransform = glm::rotate(glm::mat4(1.0f), glm::radians(particle.getRotation()), forward);

	glm::mat4 worldMatrix = glm::translate(glm::mat4(1.0f), particle.getPosition()) * rotation * yawTransform * glm::scale(glm::mat4(1.0f), glm::vec3(0.3f));
	glm::mat4 mvpMatrix = inProjectionMatrix * inViewMaterix * worldMatrix;

	particleShader->setUniform("mvpMatrix", mvpMatrix);

	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
}

void drawParticles(const std::vector<Particle>& inPparticles, const glm::mat4& inViewMaterix, const glm::mat4& inProjectionMatrix) {
	for (const auto& particle : inPparticles) {
		drawParticle(particle, inViewMaterix, inProjectionMatrix);
	}
}

glm::mat4 positiveNearFarProjection(float inFov, float nearZ, float farZ) {

	glm::mat4 projectionMatrix = glm::mat4(1.0f);

	float tanHalfFovy = glm::tan(glm::radians(inFov / 2.0f));
	//float t = tanHalfFovy * (std::abs(nearZ));
	float t = tanHalfFovy * nearZ;
	float b = -t;
	float r = t * aspect;
	float l = -r;

	glm::mat4 translate = glm::mat4(1.0f);

	translate[3][0] = -(r + l) / 2.0f;
	translate[3][1] = -(b + t) / 2.0f;
	translate[3][2] = -(farZ + nearZ) / 2.0f;

	glm::mat4 scale = glm::mat4(1.0f);

	scale[0][0] = 2.0f / (r - l);
	scale[1][1] = 2.0f / (t - b);
	scale[2][2] = -2.0f / (nearZ - farZ);
	//scale[2][3] = 1.0f;

	glm::mat4 orthogonalMatrix = scale * translate;

	//float value = -(farZ + nearZ) / (farZ - nearZ);

	////orthogonalMatrix = glm::ortho(l, r, b, t, nearZ, farZ);

	projectionMatrix[0][0] = nearZ;
	projectionMatrix[1][1] = nearZ;

	projectionMatrix[2][2] = -(nearZ + farZ); // A

	projectionMatrix[2][3] = -1.0f;

	projectionMatrix[3][2] = -nearZ * farZ; // B
	projectionMatrix[3][3] = 0.0f;

	////glm::vec4 np = { 10.0f, 10.0f, near, 1.0f };
	////glm::vec4 fp = { 0.0f, 0.0f, far, 1.0f };

	////np = projectionMatrix * np;
	////fp = projectionMatrix * fp;

	projectionMatrix = orthogonalMatrix * projectionMatrix;

	return projectionMatrix;
}

glm::mat4 negativeNearFarProjection(float inFov, float nearZ, float farZ) {

	glm::mat4 projectionMatrix = glm::mat4(1.0f);

	float tanHalfFovy = glm::tan(glm::radians(inFov / 2.0f));
	float t = tanHalfFovy * (std::abs(nearZ));
	//float t = tanHalfFovy * nearZ;
	float b = -t;
	float r = t * aspect;
	float l = -r;

	glm::mat4 translate = glm::mat4(1.0f);

	translate[3][0] = -(r + l) / 2.0f;
	translate[3][1] = -(b + t) / 2.0f;
	translate[3][2] = (farZ + nearZ) / 2.0f;

	glm::mat4 scale = glm::mat4(1.0f);

	scale[0][0] = 2.0f / (r - l);
	scale[1][1] = 2.0f / (t - b);
	scale[2][2] = 2.0f / (nearZ - farZ);
	//scale[2][3] = 1.0f;

	glm::mat4 orthogonalMatrix = scale * translate;

	//float value = -(farZ + nearZ) / (farZ - nearZ);

	////orthogonalMatrix = glm::ortho(l, r, b, t, nearZ, farZ);

	projectionMatrix[0][0] = -nearZ;
	projectionMatrix[1][1] = -nearZ;

	projectionMatrix[2][2] = nearZ + farZ; // A

	projectionMatrix[2][3] = -1.0f;

	projectionMatrix[3][2] = -nearZ * farZ; // B
	projectionMatrix[3][3] = 0.0f;

	////glm::vec4 np = { 10.0f, 10.0f, near, 1.0f };
	////glm::vec4 fp = { 0.0f, 0.0f, far, 1.0f };

	////np = projectionMatrix * np;
	////fp = projectionMatrix * fp;

	projectionMatrix = orthogonalMatrix * projectionMatrix;

	return projectionMatrix;
}

// renderCube() renders a 1x1 3D cube in NDC.
// -------------------------------------------------
unsigned int cubeVAO = 0;
unsigned int cubeVBO = 0;
void renderCube() {

	// initialize (if necessary)
	if (cubeVAO == 0)
	{
		float vertices[] = {
			// back face
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			 1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 0.0f, // bottom-right         
			 1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 1.0f, 1.0f, // top-right
			-1.0f, -1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 0.0f, // bottom-left
			-1.0f,  1.0f, -1.0f,  0.0f,  0.0f, -1.0f, 0.0f, 1.0f, // top-left
			// front face
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			 1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			 1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 1.0f, // top-right
			-1.0f,  1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 1.0f, // top-left
			-1.0f, -1.0f,  1.0f,  0.0f,  0.0f,  1.0f, 0.0f, 0.0f, // bottom-left
			// left face
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			-1.0f,  1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f, -1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-left
			-1.0f, -1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			-1.0f,  1.0f,  1.0f, -1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-right
			// right face
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 1.0f, // top-right         
			 1.0f, -1.0f, -1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 1.0f, // bottom-right
			 1.0f,  1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 1.0f, 0.0f, // top-left
			 1.0f, -1.0f,  1.0f,  1.0f,  0.0f,  0.0f, 0.0f, 0.0f, // bottom-left     
			 // bottom face
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			  1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 1.0f, // top-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			  1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 1.0f, 0.0f, // bottom-left
			 -1.0f, -1.0f,  1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 0.0f, // bottom-right
			 -1.0f, -1.0f, -1.0f,  0.0f, -1.0f,  0.0f, 0.0f, 1.0f, // top-right
			 // top face
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			  1.0f,  1.0f , 1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			  1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 1.0f, // top-right     
			  1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 1.0f, 0.0f, // bottom-right
			 -1.0f,  1.0f, -1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 1.0f, // top-left
			 -1.0f,  1.0f,  1.0f,  0.0f,  1.0f,  0.0f, 0.0f, 0.0f  // bottom-left        
		};
		glGenVertexArrays(1, &cubeVAO);
		glGenBuffers(1, &cubeVBO);
		// fill buffer
		glBindBuffer(GL_ARRAY_BUFFER, cubeVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
		// link vertex attributes
		glBindVertexArray(cubeVAO);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glBindVertexArray(0);
	}
	// render Cube
	glBindVertexArray(cubeVAO);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);
}


void renderDepthMap() {

	depthShader->use();

	//auto lightView = glm::lookAt(projectorPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	auto lightProjection = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
	//float near_plane = 1.0f, far_plane = 7.5f;
	//auto lightProjection = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, near_plane, far_plane);
	//auto lightView = glm::lookAt(glm::vec3(-2.0f, 4.0f, -1.0f), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));
	auto lightView = glm::lookAt(glm::vec3(-10.0f, 8.0f, -10.0f), glm::vec3(0.0f), glm::vec3(0.0, 1.0, 0.0));

	lightSpaceMatrix = lightProjection * lightView;
	depthShader->setUniform("lightSpaceMatrix", lightSpaceMatrix);

	glViewport(0, 0, ShadowMapWidth, SHadowMapHeight);

	glBindFramebuffer(GL_FRAMEBUFFER, depthMapFBO);

	glClear(GL_DEPTH_BUFFER_BIT);
	glm::mat4 model = glm::mat4(1.0f);
	// cubes
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(0.0f, 1.5f, 0.0));
	model = glm::scale(model, glm::vec3(0.5f));
	depthShader->setUniform("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(2.0f, 0.0f, 1.0));
	model = glm::scale(model, glm::vec3(0.5f));
	depthShader->setUniform("model", model);
	renderCube();
	model = glm::mat4(1.0f);
	model = glm::translate(model, glm::vec3(-1.0f, 0.0f, 2.0));
	model = glm::rotate(model, glm::radians(60.0f), glm::normalize(glm::vec3(1.0, 0.0, 1.0)));
	model = glm::scale(model, glm::vec3(0.25));
	depthShader->setUniform("model", model);
	renderCube();

	drawDepthModels(lightSpaceMatrix);

	drawDepthSmokes(smokesGroup[0], lightSpaceMatrix);
	drawDepthSmokes(smokesGroup[1], lightSpaceMatrix);

	drawDepthModel(terrain, lightSpaceMatrix);

	leftHouse->getMeshes()[0]->setTexture(0, currentHouseTexture);
	rightHouse->getMeshes()[0]->setTexture(0, currentHouseTexture);

	drawDepthModel(leftHouse, lightSpaceMatrix);
	drawDepthModel(rightHouse, lightSpaceMatrix);

	glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void renderScene()
{
	renderDepthMap();

	updateGlobalUniform();

	glViewport(0, 0, WindowWidth, WindowHeight);
	glm::mat4 viewMatrix = mainCamera.getViewMatrix();
	mainCamera.perspective(fov, aspect, nearPlane, farPlane);
	glm::mat4 projectionMatrix = mainCamera.getProjectionMatrix();

	//auto viewMatrix = glm::lookAt(projectorPosition, glm::vec3(0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
	//auto projectionMatrix = glm::perspective(glm::radians(fov), aspect, near, far);

	clear(clearColor, GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	drawSkybox(viewMatrix, projectionMatrix);
	drawScene(viewMatrix, projectionMatrix);

	drawModel(terrain, viewMatrix, projectionMatrix);

	leftHouse->getMeshes()[0]->setTexture(0, currentHouseTexture);
	rightHouse->getMeshes()[0]->setTexture(0, currentHouseTexture);

	drawModel(leftHouse, viewMatrix, projectionMatrix);
	drawModel(rightHouse, viewMatrix, projectionMatrix);

	if (bDrawParticles) {
		particleShader->use();
		drawParticles(particles, viewMatrix, projectionMatrix);
		sceneShader->use();
	}

	//if (bDrawNormals) {
	//	drawNormals(viewMatrix, projectionMatrix);
	//}

	if (bShowDepthMap) {
		glViewport(0, 0, WindowWidth, WindowHeight);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, depthMap);

		drawScreenQuad(depthMapTexture);
	}
}

void render()
{
	renderScene();
	renderImGui();
}

std::shared_ptr<Model> createSphere(float radius, int32_t sliceCount, int32_t stackCount, const glm::vec3& position) {

	auto meshData = geometryGenerator.CreateSphere(radius, sliceCount, stackCount);

	auto mesh = std::make_shared<Mesh>();

	for (size_t i = 0; i < meshData.Vertices.size(); i++) {
		Vertex vertex;
		vertex.position = glm::vec4(meshData.Vertices[i].Position, 1.0);
		vertex.normal = meshData.Vertices[i].Normal;
		vertex.texCoord = meshData.Vertices[i].TexC;
		mesh->addVertex(vertex);
	}

	for (auto index : meshData.Indices32) {
		mesh->addIndex(index);
	}

	auto model = std::make_shared<Model>();
	mesh->setTexture(0, getTexture("Globe"));
	model->addMesh(mesh);

	model->setPosition(position);

	models.push_back(model);

	return model;
}

std::shared_ptr<Model> createSmoke(float radius, const glm::vec3& position) {

	auto smoke = geometryGenerator.CreateGeosphere(radius, 0);

	auto mesh = std::make_shared<Mesh>();

	for (size_t i = 0; i < smoke.Vertices.size(); i++) {
		Vertex vertex;
		vertex.position = glm::vec4(smoke.Vertices[i].Position, 1.0);
		vertex.normal = smoke.Vertices[i].Normal;
		vertex.texCoord = smoke.Vertices[i].TexC;
		mesh->addVertex(vertex);
	}

	for (size_t i = 0; i < smoke.Indices32.size(); i++) {
		mesh->addIndex(smoke.Indices32[i]);
	}

	auto model = std::make_shared<Model>();
	mesh->addTexture(defaultAlbedo);
	model->addMesh(mesh);

	model->setPosition(glm::vec3(position.x, position.y, position.z));

	model->computeTangentSpace();
	model->prepareDraw();

	return model;
}

float testFrameTime = 0.016667f;
int32_t frameCount = 0;

void updateFPSCounter(GLFWwindow* inWindow)
{
	static double previousSeconds = glfwGetTime();
	double currentSeconds = glfwGetTime();
	double elapsedSeconds = currentSeconds - previousSeconds;

	if (elapsedSeconds >= 0.25f)
	{
		previousSeconds = currentSeconds;
		double fps = (double)frameCount / elapsedSeconds;
		char temp[64];
#ifdef __linux__
		sprintf(temp, "COMP3811 - Coursework 2 FPS:%f", fps);
#else
		sprintf_s(temp, "COMP3811 - Coursework 2 FPS:%f", fps);
#endif
		glfwSetWindowTitle(inWindow, temp);
		frameCount = 0;
	}

	frameCount++;
}

void loadModels() {

	auto material = std::make_shared<Material>();

	material->Ka = { 0.37f, 0.37f, 0.37f };
	material->Kd = { 0.87f, 0.87f, 0.87f };
	material->Ks = { 0.49f, 0.49f, 0.49f };
	material->shininess = 32.0f;
	material->reflectionFactor = 0.0f;
	material->refractionFactor = 0.0f;

	// air / glass
	material->eta = 1.0f / 1.5f;

	addMaterial("Common", material);

	auto model = loadModel("./assets/models/cube.obj", "Skybox");
	model->scale(glm::vec3(100.0f));
	models.push_back(model);

	auto terrianMeshData = geometryGenerator.CreateGrid(50.0f, 50.0f, 50, 50, 10.0f);

	auto mesh = std::make_shared<Mesh>();

	for (size_t i = 0; i < terrianMeshData.Vertices.size(); i++) {
		Vertex vertex;
		glm::vec4 newPosition = glm::vec4(terrianMeshData.Vertices[i].Position, 1.0);
		newPosition.y = getSmoothNoise(newPosition.x, newPosition.z);
		vertex.position = newPosition;
		vertex.normal = terrianMeshData.Vertices[i].Normal;
		vertex.texCoord = terrianMeshData.Vertices[i].TexC;
		mesh->addVertex(vertex);
	}

	for (auto index : terrianMeshData.Indices32) {
		mesh->addIndex(index);
	}

	terrain = std::make_shared<Model>();

	mesh->addTexture(defaultAlbedo);
	mesh->setMaterial(material);
	mesh->setTexture(0, getTexture("CartoonSnow"));

	terrain->addMesh(mesh);

	terrain->setPosition(glm::vec3(0.0f, -2.25f, 0.0f));

	model = loadModel("./assets/models/ChristmasTree.obj");
	model->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	model->setPosition(glm::vec3(-5.5f, -1.6f, 2.0f));

	models.push_back(model);

	movingSphere = loadModel("./assets/models/sphere.obj");
	movingSphere->setPosition(glm::vec3(2.0f, 10.0f, 0.0f));

	models.push_back(movingSphere);

	redBalls[0] = loadModel("./assets/models/RedBall.obj");
	redBalls[0]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	redBalls[0]->setPosition(glm::vec3(-5.5f, -1.6f, 2.0f));

	goldenBalls[0] = loadModel("./assets/models/GoldenBall.obj");
	goldenBalls[0]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	goldenBalls[0]->setPosition(glm::vec3(-5.5f, -1.6f, 2.0f));

	purpleBalls[0] = loadModel("./assets/models/PurpleBall.obj");
	purpleBalls[0]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	purpleBalls[0]->setPosition(glm::vec3(-5.5f, -1.6f, 2.0f));

	model = loadModel("./assets/models/ChristmasTree.obj");
	model->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	model->setPosition(glm::vec3(-3.5f, -1.6f, 1.0f));

	models.push_back(model);

	redBalls[1] = loadModel("./assets/models/RedBall.obj");
	redBalls[1]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	redBalls[1]->setPosition(glm::vec3(-3.5f, -1.6f, 1.0f));

	goldenBalls[1] = loadModel("./assets/models/GoldenBall.obj");
	goldenBalls[1]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	goldenBalls[1]->setPosition(glm::vec3(-3.5f, -1.6f, 1.0f));

	purpleBalls[1] = loadModel("./assets/models/PurpleBall.obj");
	purpleBalls[1]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	purpleBalls[1]->setPosition(glm::vec3(-3.5f, -1.6f, 1.0f));

	model = loadModel("./assets/models/ChristmasTree.obj");
	model->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	model->setPosition(glm::vec3(-1.5f, -1.6f, 0.0f));

	models.push_back(model);

	redBalls[2] = loadModel("./assets/models/RedBall.obj");
	redBalls[2]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	redBalls[2]->setPosition(glm::vec3(-1.5f, -1.6f, 0.0f));

	goldenBalls[2] = loadModel("./assets/models/GoldenBall.obj");
	goldenBalls[2]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	goldenBalls[2]->setPosition(glm::vec3(-1.5f, -1.6f, 0.0f));

	purpleBalls[2] = loadModel("./assets/models/PurpleBall.obj");
	purpleBalls[2]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	purpleBalls[2]->setPosition(glm::vec3(-1.5f, -1.6f, 0.0f));

	model = loadModel("./assets/models/ChristmasTree.obj");
	model->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	model->setPosition(glm::vec3(1.5f, -1.6f, 0.0f));

	models.push_back(model);

	redBalls[3] = loadModel("./assets/models/RedBall.obj");
	redBalls[3]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	redBalls[3]->setPosition(glm::vec3(1.5f, -1.6f, 0.0f));

	goldenBalls[3] = loadModel("./assets/models/GoldenBall.obj");
	goldenBalls[3]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	goldenBalls[3]->setPosition(glm::vec3(1.5f, -1.6f, 0.0f));

	purpleBalls[3] = loadModel("./assets/models/PurpleBall.obj");
	purpleBalls[3]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	purpleBalls[3]->setPosition(glm::vec3(1.5f, -1.6f, 0.0f));

	model = loadModel("./assets/models/ChristmasTree.obj");
	model->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	model->setPosition(glm::vec3(3.5f, -1.6f, 1.0f));

	models.push_back(model);

	redBalls[4] = loadModel("./assets/models/RedBall.obj");
	redBalls[4]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	redBalls[4]->setPosition(glm::vec3(3.5f, -1.6f, 1.0f));

	goldenBalls[4] = loadModel("./assets/models/GoldenBall.obj");
	goldenBalls[4]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	goldenBalls[4]->setPosition(glm::vec3(3.5f, -1.6f, 1.0f));

	purpleBalls[4] = loadModel("./assets/models/PurpleBall.obj");
	purpleBalls[4]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	purpleBalls[4]->setPosition(glm::vec3(3.5f, -1.6f, 1.0f));

	model = loadModel("./assets/models/ChristmasTree.obj");
	model->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	model->setPosition(glm::vec3(5.5f, -1.6f, 2.0f));

	models.push_back(model);

	redBalls[5] = loadModel("./assets/models/RedBall.obj");
	redBalls[5]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	redBalls[5]->setPosition(glm::vec3(5.5f, -1.6f, 2.0f));

	goldenBalls[5] = loadModel("./assets/models/GoldenBall.obj");
	goldenBalls[5]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	goldenBalls[5]->setPosition(glm::vec3(5.5f, -1.6f, 2.0f));

	purpleBalls[5] = loadModel("./assets/models/PurpleBall.obj");
	purpleBalls[5]->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	purpleBalls[5]->setPosition(glm::vec3(5.5f, -1.6f, 2.0f));

	leftHouse = loadModel("./assets/models/House.obj");
	leftHouse->scale(glm::vec3(4.0f, 4.0f, 4.0f));
	leftHouse->rotate(-90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	leftHouse->setPosition(glm::vec3(-8.0f, 0.5f, -5.0f));

	rightHouse = loadModel("./assets/models/House.obj");
	rightHouse->scale(glm::vec3(4.0f, 4.0f, 4.0f));
	rightHouse->rotate(-90.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	rightHouse->setPosition(glm::vec3(8.0f, 0.5f, -5.0f));

	smokeSpawnPositions1[0] = { leftHouse->getPosition().x - 2.0f, leftHouse->getPosition().y + 4.5f, leftHouse->getPosition().z + 0.675f };
	smokeSpawnPositions1[1] = { leftHouse->getPosition().x - 2.0f, leftHouse->getPosition().y + 4.0f, leftHouse->getPosition().z + 0.675f };
	smokeSpawnPositions1[2] = { leftHouse->getPosition().x - 2.0f, leftHouse->getPosition().y + 3.5f, leftHouse->getPosition().z + 0.675f };

	smokeSpawnPositions2[0] = { rightHouse->getPosition().x - 2.0f, rightHouse->getPosition().y + 4.5f, rightHouse->getPosition().z + 0.675f };
	smokeSpawnPositions2[1] = { rightHouse->getPosition().x - 2.0f, rightHouse->getPosition().y + 4.0f, rightHouse->getPosition().z + 0.675f };
	smokeSpawnPositions2[2] = { rightHouse->getPosition().x - 2.0f, rightHouse->getPosition().y + 3.5f, rightHouse->getPosition().z + 0.675f };

	leftSnowman = loadModel("./assets/models/SnowmanBodyV2.obj");
	leftSnowman->setPosition(glm::vec3(-2.5f, -1.8f, 5.0f));

	models.push_back(leftSnowman);

	leftSnowmanArm = loadModel("./assets/models/SnowmanArmV2.obj");
	leftSnowmanArm->setPosition(glm::vec3(-2.5f, -1.8f, 5.0f));

	models.push_back(leftSnowmanArm);

	snowmen.push_back(leftSnowman);
	snowmen.push_back(leftSnowmanArm);

	middleSnowman = loadModel("./assets/models/SnowmanBodyV2.obj");
	middleSnowman->setPosition(glm::vec3(0.0f, -1.8f, 5.0f));

	models.push_back(middleSnowman);

	middleSnowmanArm = loadModel("./assets/models/SnowmanArmV2.obj");
	middleSnowmanArm->setPosition(glm::vec3(0.0f, -1.8f, 5.0f));

	models.push_back(middleSnowmanArm);

	snowmen.push_back(middleSnowman);
	snowmen.push_back(middleSnowmanArm);

	rightSnowman = loadModel("./assets/models/SnowmanBodyV2.obj");
	rightSnowman->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	//    model->rotate(-45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	rightSnowman->setPosition(glm::vec3(2.5f, -1.8f, 5.0f));

	models.push_back(rightSnowman);

	rightSnowmanArm = loadModel("./assets/models/SnowmanArmV2.obj");
	rightSnowmanArm->scale(glm::vec3(1.0f, 1.0f, 1.0f));
	//    model->rotate(-45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	rightSnowmanArm->setPosition(glm::vec3(2.5f, -1.8f, 5.0f));

	models.push_back(rightSnowmanArm);

	snowmen.push_back(rightSnowman);
	snowmen.push_back(rightSnowmanArm);

	model = loadModel("./assets/models/Present.obj");
	model->setPosition(glm::vec3(-2.0f, -1.8f, 1.0f));

	models.push_back(model);

	model = loadModel("./assets/models/Present.obj");
	model->rotate(-45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	model->setPosition(glm::vec3(2.0f, -1.8f, 1.0f));

	models.push_back(model);

	model = loadModel("./assets/models/Present.obj");
	model->setPosition(glm::vec3(-5.5f, -1.8f, 2.0f));

	models.push_back(model);

	model = loadModel("./assets/models/plane.obj");
	model->setScale(glm::vec3(1.0f, 1.27f, 1.0f));
	model->rotate(90.0f, glm::vec3(1.0f, 0.0f, 0.0f));
	model->setPosition(glm::vec3(0.0f, 0.0f, -8.5f));

	models.push_back(model);

	model = loadModel("./assets/models/Present.obj");
	model->setPosition(glm::vec3(5.5f, -1.8f, 2.0f));

	models.push_back(model);

	merryChristmasSnowman = loadModel("./assets/models/SnowmanBodyV2.obj");
	merryChristmasSnowman->rotate(180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	merryChristmasSnowman->setPosition(glm::vec3(0.0f, -1.5f, -10.0f));

	merryChristmasSnowmanArm = loadModel("./assets/models/SnowmanArmV2.obj");
	merryChristmasSnowmanArm->rotate(180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	merryChristmasSnowmanArm->setPosition(glm::vec3(0.0f, -1.5f, -10.0f));

	flagpole = loadModel("./assets/models/Flagpole.obj");
	flagpole->rotate(180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	flagpole->setPosition(glm::vec3(0.0f, -1.5f, -10.0f));

	flag = loadModel("./assets/models/Flag.obj");
	flag->rotate(180.0f, glm::vec3(0.0f, 1.0f, 0.0f));
	flag->setPosition(glm::vec3(0.0f, -1.5f, -10.0f));

	smokesGroup[0].push_back(createSmoke(0.125f, smokeSpawnPositions1[0]));
	smokesGroup[0].push_back(createSmoke(0.125f, smokeSpawnPositions1[1]));
	smokesGroup[0].push_back(createSmoke(0.125f, smokeSpawnPositions1[2]));

	smokesGroup[1].push_back(createSmoke(0.125f, smokeSpawnPositions2[0]));
	smokesGroup[1].push_back(createSmoke(0.125f, smokeSpawnPositions2[1]));
	smokesGroup[1].push_back(createSmoke(0.125f, smokeSpawnPositions2[2]));

	giftBoxBody = loadModel("./assets/models/GiftBoxBody.obj");
	giftBoxBody->setPosition(glm::vec3(0.0f, -1.8f, -10.0f));

	models.push_back(giftBoxBody);

	giftBoxCover = loadModel("./assets/models/GiftBoxCover.obj");
	giftBoxCover->setPosition(glm::vec3(0.0f, -1.8f, -10.0f));

	models.push_back(giftBoxCover);

	spawnParticles(2000);

	for (auto& m : models) {
		m->computeTangentSpace();
		m->prepareDraw();
	}

	terrain->computeTangentSpace();
	terrain->prepareDraw();

	leftHouse->computeTangentSpace();
	leftHouse->prepareDraw();

	rightHouse->computeTangentSpace();
	rightHouse->prepareDraw();

	for (auto i = 0; i < 6; i++) {
		redBalls[i]->computeTangentSpace();
		redBalls[i]->prepareDraw();

		goldenBalls[i]->computeTangentSpace();
		goldenBalls[i]->prepareDraw();

		purpleBalls[i]->computeTangentSpace();
		purpleBalls[i]->prepareDraw();
	}

	merryChristmasSnowman->computeTangentSpace();
	merryChristmasSnowman->prepareDraw();

	merryChristmasSnowmanArm->computeTangentSpace();
	merryChristmasSnowmanArm->prepareDraw();

	flagpole->computeTangentSpace();
	flagpole->prepareDraw();

	flag->computeTangentSpace();
	flag->prepareDraw();
}

void prepareTextures()
{
	// Create the render texture
	// addTexture("Error", "./resources/textures/Error.png");

	sceneTexture = std::make_shared<Texture>("sceneTexture", WindowWidth, WindowHeight, GL_LINEAR, false, GL_RGB16F, GL_RGBA, GL_FLOAT);

	defaultAlbedo = std::make_shared<Texture>("defaultAlbedo", 1, 1);

	skyboxDusk = addCubemapTexture("Dusk", "./assets/textures/sunset", GL_CLAMP_TO_EDGE, true);
	skyboxDay = addCubemapTexture("Day", "./assets/textures/day", GL_CLAMP_TO_EDGE, true);
	skyboxNight = addCubemapTexture("Night", "./assets/textures/night", GL_CLAMP_TO_EDGE, true);

	currentSkybox = skyboxDay;

	//addTexture("Projection", "./assets/textures/Kanna.jpg", GL_CLAMP_TO_BORDER);

	addTexture("CartoonGrass", "./assets/textures/CartoonGrass.jpg");
	addTexture("CartoonSnow", "./assets/textures/Snow.jpg");
	addTexture("Markus", "./assets/textures/markus.png");
	houseTexture = addTexture("House", "./assets/textures/House.png");
	houseLightTexture = addTexture("HouseLight", "./assets/textures/HouseLight.png");
	snowflakesTexture = addTexture("Snowflakes", "./assets/textures/Snowflakes.png");

	currentHouseTexture = houseTexture;

	depthMapTexture = std::make_shared<Texture>();
	depthMapTexture->createDepthMap(ShadowMapWidth, SHadowMapHeight);
}

void writeToPNG(const std::string& path, int32_t width, int32_t height, uint8_t* pixelBuffer) 
{
	std::vector<uint8_t> pixelData;

	for (auto y = height - 1; y >= 0; y--) 
	{
		for (auto x = 0; x < width; x++) 
		{
			auto index = y * width + x;
			pixelData.push_back(pixelBuffer[index * 3]);
			pixelData.push_back(pixelBuffer[index * 3 + 1]);
			pixelData.push_back(pixelBuffer[index * 3 + 2]);
			pixelData.push_back(255);
		}
	}

	//Encode the image
	unsigned error = lodepng::encode(path, pixelData, width, height);
	//if there's an error, display it
	if (error) std::cout << "encoder error " << error << ": " << lodepng_error_text(error) << std::endl;
}

int main() try
{
	// Initialize GLFW
	if( GLFW_TRUE != glfwInit() )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwInit() failed with '%s' (%d)", msg, ecode );
	}

	// Ensure that we call glfwTerminate() at the end of the program.
	GLFWCleanupHelper cleanupHelper;

	// Configure GLFW and create window
	glfwSetErrorCallback( &glfw_callback_error_ );

	glfwWindowHint( GLFW_SRGB_CAPABLE, GLFW_TRUE );
	glfwWindowHint( GLFW_DOUBLEBUFFER, GLFW_TRUE );

	//glfwWindowHint( GLFW_RESIZABLE, GLFW_FALSE );

	glfwWindowHint( GLFW_CONTEXT_VERSION_MAJOR, 4 );
	glfwWindowHint( GLFW_CONTEXT_VERSION_MINOR, 3 );
	glfwWindowHint( GLFW_OPENGL_FORWARD_COMPAT, GLFW_TRUE );
	glfwWindowHint( GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE );

	glfwWindowHint( GLFW_DEPTH_BITS, 24 );

#	if !defined(NDEBUG)
	// When building in debug mode, request an OpenGL debug context. This
	// enables additional debugging features. However, this can carry extra
	// overheads. We therefore do not do this for release builds.
	glfwWindowHint( GLFW_OPENGL_DEBUG_CONTEXT, GLFW_TRUE );
#	endif // ~ !NDEBUG

	window = glfwCreateWindow(
		WindowWidth,
		WindowHeight,
		kWindowTitle,
		nullptr, nullptr
	);

	if( !window )
	{
		char const* msg = nullptr;
		int ecode = glfwGetError( &msg );
		throw Error( "glfwCreateWindow() failed with '%s' (%d)", msg, ecode );
	}

	GLFWWindowDeleter windowDeleter{ window };

	// Set up event handling
	glfwSetKeyCallback( window, &glfw_callback_key_ );

	// Set up drawing stuff
	glfwMakeContextCurrent( window );
	glfwSwapInterval( 1 ); // V-Sync is on.

	// Initialize GLAD
	// This will load the OpenGL API. We mustn't make any OpenGL calls before this!
	if( !gladLoadGLLoader( (GLADloadproc)&glfwGetProcAddress ) )
		throw Error( "gladLoaDGLLoader() failed - cannot load GL API!" );

	std::printf( "RENDERER %s\n", glGetString( GL_RENDERER ) );
	std::printf( "VENDOR %s\n", glGetString( GL_VENDOR ) );
	std::printf( "VERSION %s\n", glGetString( GL_VERSION ) );
	std::printf( "SHADING_LANGUAGE_VERSION %s\n", glGetString( GL_SHADING_LANGUAGE_VERSION ) );

	// Ddebug output
#	if !defined(NDEBUG)
	setup_gl_debug_output();
#	endif // ~ !NDEBUG

	// Global GL state
	OGL_CHECKPOINT_ALWAYS();

	// TODO: global GL setup goes here

	OGL_CHECKPOINT_ALWAYS();

	// Get actual framebuffer size.
	// This can be different from the window size, as standard window
	// decorations (title bar, borders, ...) may be included in the window size
	// but not be part of the drawable surface area.
	int iwidth, iheight;
	glfwGetFramebufferSize( window, &iwidth, &iheight );

	glViewport( 0, 0, iwidth, iheight );

	// Other initialization & loading
	OGL_CHECKPOINT_ALWAYS();
	
	// TODO: 

	OGL_CHECKPOINT_ALWAYS();

	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	std::cout << glGetString(GL_VENDOR) << std::endl;
	std::cout << glGetString(GL_RENDERER) << std::endl;

	bindCallbacks();

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_MULTISAMPLE);

	prepareTextures();

	prepareShaderResources();
	sceneShader->printActiveAttributes();
	sceneShader->printActiveUniforms();

	mainCamera.perspective(fov, aspect, nearPlane, farPlane);

	loadModels();

	prepareGeometryData();

	initImGui();

	//SoundPlayer::getInstance()->play("./assets/music/ChristmasEve.wav", true);

	// Main loop
	while( !glfwWindowShouldClose( window ) )
	{
		// Let GLFW process events
		glfwPollEvents();
		
		// Check if window was resized.
		float fbwidth, fbheight;
		{
			int nwidth, nheight;
			glfwGetFramebufferSize( window, &nwidth, &nheight );

			fbwidth = float(nwidth);
			fbheight = float(nheight);

			if( 0 == nwidth || 0 == nheight )
			{
				// Window minimized? Pause until it is unminimized.
				// This is a bit of a hack.
				do
				{
					glfwWaitEvents();
					glfwGetFramebufferSize( window, &nwidth, &nheight );
				} while( 0 == nwidth || 0 == nheight );
			}
		}

		UNUSED(fbwidth);
		UNUSED(fbheight);

		// Update state
		processInput(window);
		updateFPSCounter(window);
		update();
		//TODO: update state
	
		// Draw scene
		OGL_CHECKPOINT_DEBUG();
		render();
		//TODO: draw frame

		OGL_CHECKPOINT_DEBUG();

		// Display results
		glfwSwapBuffers( window );
	}

	// Cleanup.
	//TODO: additional cleanup
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	return 0;
}
catch( std::exception const& eErr )
{
	std::fprintf( stderr, "Top-level Exception (%s):\n", typeid(eErr).name() );
	std::fprintf( stderr, "%s\n", eErr.what() );
	std::fprintf( stderr, "Bye.\n" );
	return 1;
}


namespace
{
	void glfw_callback_error_( int aErrNum, char const* aErrDesc )
	{
		std::fprintf( stderr, "GLFW error: %s (%d)\n", aErrDesc, aErrNum );
	}

	void glfw_callback_key_( GLFWwindow* aWindow, int aKey, int, int aAction, int )
	{
		if( GLFW_KEY_ESCAPE == aKey && GLFW_PRESS == aAction )
		{
			glfwSetWindowShouldClose( aWindow, GLFW_TRUE );
			return;
		}
	}
}

namespace
{
	GLFWCleanupHelper::~GLFWCleanupHelper()
	{
		glfwTerminate();
	}

	GLFWWindowDeleter::~GLFWWindowDeleter()
	{
		if( window )
			glfwDestroyWindow( window );
	}
}


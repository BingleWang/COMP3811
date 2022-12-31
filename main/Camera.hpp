#pragma once

#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Camera {
public:
    Camera(const glm::vec3& inEye, const glm::vec3& inCenter)
    : eye(inEye), center(inCenter) {
        forward = glm::normalize(center - eye);
        Right = glm::cross(forward, up);
        up = glm::cross(Right, forward);

        isDirty = true;

        updateViewMatrix();
    }

    void walk(float delta) {
        eye += forward * cameraSpeed * delta;
        center += forward * cameraSpeed * delta;
        isDirty = true;
    }

    void strafe(float delta) {
        eye += Right * cameraSpeed * delta;
        center += Right * cameraSpeed * delta;
        isDirty = true;
    }

    void raise(float delta) {
        eye += up * cameraSpeed * delta;
        center += up * cameraSpeed * delta;
        isDirty = true;
    }

    void yaw(float delta) {
        // Should rotate around up vector
        glm::mat3 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(delta), worldUp);
        Right = glm::normalize(rotation * Right);
        up = glm::normalize(rotation * up);
        forward = glm::normalize(rotation * forward);

        //up = glm::cross(right, forward);

        center = eye + forward;

        isDirty = true;
    }

    void pitch(float delta) {
        // Should rotate around right vector
        glm::mat3 rotation = glm::rotate(glm::mat4(1.0f), glm::radians(delta), Right);
        forward = glm::normalize(rotation * forward);
        up = glm::normalize(rotation * up);

        center = eye + forward;

        isDirty = true;
    }

    void updateViewMatrix() {
        if (isDirty) {
            viewMatrix = glm::lookAt(eye, center, up);
            Right = { viewMatrix[0][0], viewMatrix[1][0], viewMatrix[2][0] };
            up = { viewMatrix[0][1], viewMatrix[1][1], viewMatrix[2][1] };
            forward = { viewMatrix[0][2], viewMatrix[1][2], viewMatrix[2][2] };
            forward = -forward;
            isDirty = false;
        }
    }

    void buildReflectionMatrix(float height) {
        glm::vec3 position;
        glm::vec3 lookAt;

        // Setup the position of the camera in the world
        // For planar reflection invert the Y position of the camera
        position.x = eye.x;
        position.y = -eye.y + (height * 2.0f);
        position.z = eye.z;

        lookAt.x = center.x;
        lookAt.y = position.y;
        lookAt.z = center.z;

        reflectionViewMatrix = glm::lookAt(position, lookAt, up);
    }

    glm::mat4 getReflectionViewMatrix() {
        return reflectionViewMatrix;
    }

    void perspective(float inFOV, float inAspect, float inNear, float inFar) {
        fov = inFOV;
        aspect = inAspect;
        nearPlane = inNear;
        farPlane = inFar;
        projectionMatrix = glm::perspective(glm::radians(fov), aspect, nearPlane, farPlane);
    }

    void orthographic(float left, float right, float bottom, float top) {
        projectionMatrix = glm::ortho(left, right, bottom, top, -1.0f, 1.0f);
    }

    void setCenter(const glm::vec3& inCenter) {
        center = inCenter;
        isDirty = true;
    }

    void setEye(const glm::vec3& inEye) {
        eye = inEye;
        isDirty = true;
    }

    const glm::mat4& getViewMatrix() const {
        return viewMatrix;
    }

    const glm::mat4& getProjectionMatrix() const {
        return projectionMatrix;
    }

    const glm::vec3 getRight() const {
        return Right;
    }

    const glm::vec3 getUp() const {
        return up;
    }

    const glm::vec3 getForward() const {
        return forward;
    }

    const glm::vec3 getEye() const {
        return eye;
    }

    void setCameraSpeed(float inCameraSpeed) {
        cameraSpeed = inCameraSpeed;
    }

private:
    float fov = 60.0f;
    float nearPlane = 0.1f;
    float farPlane = 100.0f;
    float aspect = 16.0f / 9.0f;
    glm::vec3 eye = { 0.0f, 1.0f, 4.5f };
    glm::vec3 center = { 0.0f, 1.0f, 0.0f };
    float cameraSpeed = 6.0f;
    glm::vec3 up = { 0.0f, 1.0f, 0.0f };
    glm::vec3 worldUp = { 0.0f, 1.0f, 0.0f };
    glm::vec3 forward;
    glm::vec3 Right;
    glm::mat4 viewMatrix;
    glm::mat4 reflectionViewMatrix;
    glm::mat4 projectionMatrix;
    bool isDirty = false;
};


#include "Texture.hpp"
#include <cstring>
#include <memory>
#include <iostream>
#include <filesystem>

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define UNUSED(x) (void)(x)

uint32_t Texture::activeIndex = 0;

std::string Texture::suffixes[] = { "posx", "negx", "posy", "negy", "posz", "negz" };

GLenum targets[] = {
	GL_TEXTURE_CUBE_MAP_POSITIVE_X,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_X,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Y,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Y,
	GL_TEXTURE_CUBE_MAP_POSITIVE_Z,
	GL_TEXTURE_CUBE_MAP_NEGATIVE_Z,
};

Texture::Texture(const std::string& inName, int32_t inWidth, int32_t inHeight, int32_t filter, bool fillData, int32_t internalFormat, int32_t format, GLenum type) {
    UNUSED(inName);

    width = inWidth;
    height = inHeight;

    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE0 + activeIndex++);
    glBindTexture(GL_TEXTURE_2D, id);

    if (fillData) {

        auto* data = new uint8_t[width * height * 4];

        memset(data, 255, width * height * 4);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, data);

        stbi_image_free(data);
    }
    else {
        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type, nullptr);
    }

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filter);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filter);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
}

void Texture::createCubemap(int32_t inWidth, int32_t inHeight) {
    width = inWidth;
    height = inHeight;

    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE0 + activeIndex++);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    for (int32_t i = 0; i < 6; i++) {
		glTexImage2D(targets[i], 0, GL_RGB, 2048, 2048, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
    }

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
}

void Texture::createDepthMap(int32_t inWidth, int32_t inHeight) {
	width = inWidth;
	height = inHeight;

    glGenTextures(1, &id);
    glActiveTexture(GL_TEXTURE0 + activeIndex++);
    glBindTexture(GL_TEXTURE_2D, id);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, inWidth, inHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);

    float borderColor[] = { 1.0f, 1.0f, 1.0f, 1.0f };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);
}

void Texture::load(const std::string& fileName, int32_t wrapMode) {
    if (!std::filesystem::exists(fileName)) {
        std::cout << "File " + fileName << " doesn't exists.\n";
        return;
    }

    int32_t bpp = 0;

    uint8_t* data = stbi_load(fileName.c_str(), &width, &height, &bpp, 4);

    glActiveTexture(GL_TEXTURE0 + activeIndex++);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_2D, id);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, wrapMode);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, wrapMode);

    stbi_image_free(data);
}

void Texture::loadCubemap(const std::string& baseName, int32_t wrapMode, bool hdr) {
    glActiveTexture(GL_TEXTURE0 + activeIndex++);

    glGenTextures(1, &id);
    glBindTexture(GL_TEXTURE_CUBE_MAP, id);

    for (int i = 0; i < 6; i++) {
        width = 0;
        height = 0;
        int32_t bpp = 0;

        std::string ext = ".png";
        
        if (hdr) {
            ext = ".hdr";
        }

        std::string textureName = std::string(baseName) + "_" + suffixes[i] + ext;

        if (!std::filesystem::exists(textureName)) {
            std::cout << "Texture " + textureName + " not found.\n";
            return;
        }

        uint8_t* data = stbi_load(textureName.c_str(), &width, &height, &bpp, 3);

        glTexImage2D(targets[i], 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

        stbi_image_free(data);
    }

    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, wrapMode);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, wrapMode);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, wrapMode);
}

void Texture::use() {
    glBindTexture(GL_TEXTURE_2D, id);
}

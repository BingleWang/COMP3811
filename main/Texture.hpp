#pragma once

#include <string>

#include <glad.h>

class Texture {
public:
    Texture() {}
    Texture(const std::string& name, int32_t inWidth, int32_t inHeight, int32_t filter = GL_LINEAR, bool fillData = true, int32_t internalFormat = GL_RGBA, int32_t format = GL_RGBA, GLenum type = GL_UNSIGNED_BYTE);

    Texture(const std::string& fileName, int32_t wrapMode = GL_REPEAT, bool cubeMap = false, bool hdr = false) {
        if (cubeMap) {
            loadCubemap(fileName, wrapMode, hdr);
        }
        else {
            load(fileName, wrapMode);
        }
    }

    ~Texture() {
        //glDeleteTextures(1, &id);
        activeIndex--;
    }

    void createCubemap(int32_t inWidth, int32_t inHeight);

    void createDepthMap(int32_t inWidth, int32_t inHeight);

    void load(const std::string& fileName, int32_t wrapMode = GL_REPEAT);

    void loadCubemap(const std::string& baseName, int32_t wrapMode = GL_CLAMP_TO_EDGE, bool hdr = false);

    void use();

    uint32_t getTextureId() const {
        return id;
    }

    int32_t getTextureIndex() const {
        return id - 1;
    }

    static uint32_t getActiveIndex() {
        return activeIndex;
    }

    static void increaseActiveIndex() {
        activeIndex++;
    }

    int32_t getWidth() const {
        return width;
    }

    int32_t getHeight() const {
        return height;
    }

    static std::string suffixes[];

private:
    static uint32_t activeIndex;
    uint32_t id = 0;
    int32_t width = 0;
    int32_t height = 0;
};
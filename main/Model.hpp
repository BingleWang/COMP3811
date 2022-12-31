#pragma once

#include <memory>
#include <vector>

#include <glad.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/hash.hpp>

#include "Texture.hpp"
#include "Material.hpp"

struct SimpleVertex {
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 texcoord;
};

struct Vertex {
    glm::vec3 position;
    glm::vec3 tangent;
    glm::vec3 binormal;
    glm::vec3 normal;
    glm::vec2 texCoord;

    bool operator==(const Vertex& other) const {
        return (other.position == position) &&
            (other.normal == normal) &&
            (other.texCoord == texCoord);
    }
};

namespace std {
    template<> struct hash<Vertex> {
        size_t operator()(Vertex const& vertex) const {
            return ((hash<glm::vec3>()(vertex.position)
                ^ (hash<glm::vec3>()(vertex.normal) << 1)) >> 1)
                ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
        }
    };
}

//namespace std {
//    template<> struct hash<Vertex> {
//        size_t operator()(Vertex const& vertex) const {
//            return (hash<glm::vec3>()(vertex.position))
//                 ^ (hash<glm::vec3>()(vertex.normal) << 1)
//                 ^ (hash<glm::vec2>()(vertex.texCoord) << 1);
//        }
//    };
//}

class Mesh {
public:
    Mesh() {

    }

    ~Mesh() {
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &IBO);
        glDeleteBuffers(1, &VAO);
    }

    void addVertex(const Vertex& vertex) {
        vertices.push_back(vertex);
    }

    void addIndex(uint32_t index) {
        indices.push_back(index);
    }

    size_t getVertexBufferByteSize() const {
        return sizeof(Vertex) * vertices.size();
    }

    size_t getIndexBufferByteSize() const {
        return sizeof(uint32_t) * indices.size();
    }

    uint32_t getTriangleCount() const {
        return static_cast<uint32_t>(vertices.size() / 3);
    }

    void addTexture(const std::shared_ptr<Texture>& texture) {
        textures.push_back(texture);
    }

    void setTexture(int32_t index, const std::shared_ptr<Texture>& texture) {
        if (!textures.empty()) {
			if (index <= (static_cast<int32_t>(textures.size()) - 1)) {
				textures[index] = texture;
			}
        }
    }

    int32_t getTextureIndex(uint32_t index) const {
        if (index >= static_cast<uint32_t>(textures.size())) {
            return 0;
        }

        auto texture = textures[index];

        if (texture) {
            return texture->getTextureIndex();
        }

        return 0;
    }

    void setMaterial(const std::shared_ptr<Material>& inMaterial) {
        material = inMaterial;
    }

    const std::shared_ptr<Material>& getMaterial() const {
        return material;
    }

    std::shared_ptr<Material>& getMaterial() {
        return material;
    }

    void setName(const std::string& inName) {
        name = inName;
    }

    const std::string& getName() const {
        return name;
    }

    std::vector<Vertex> getVertices() const {
        return vertices;
    }

    size_t getVertexCount() const {
        return vertices.size();
    }

    const Vertex* getVerticesData() const {
        return vertices.data();
    }

    std::vector<uint32_t> getIndices() const {
        return indices;
    }

    int32_t getIndexCount() const {
        return static_cast<int32_t>(indices.size());
    }

    const uint32_t* getIndicesData() const {
        return indices.data();
    }

    int32_t getNormalIndexCount() const {
        return static_cast<int32_t>(normals.size() * 2);
    }

    void computeTangentSpace();

    void prepareDraw();

    void use() {
        glBindVertexArray(VAO);
    }

    void useNormal() {
        glBindVertexArray(VAONormal);
    }
private:
    std::string name;

    std::vector<Vertex> vertices;
    std::vector<uint32_t> indices;
    std::vector<SimpleVertex> normals;

    uint32_t VBO = 0;
    uint32_t IBO = 0;
    uint32_t VAO = 0;
    uint32_t VBONormal = 0;
    uint32_t VAONormal = 0;

    std::shared_ptr<Material> material;
    std::vector<std::shared_ptr<Texture>> textures;
};

class Model {
public:
    Model() {
        position = glm::vec3(0.0f);
        transform = glm::mat4(1.0f);
    }

    ~Model() {
    }

    uint32_t getTriangleCount() const {
        return 0;
    }

    void scale(const glm::vec3& factor) {
        transform = glm::scale(transform, factor);
    }

    void setScale(const glm::vec3& inScale) {
        transform[0][0] = inScale.x;
        transform[1][1] = inScale.y;
        transform[2][2] = inScale.z;
    }

    glm::vec3 getScale() const {
        return glm::vec3(transform[0][0], transform[1][1], transform[2][2]);
    }

    void translate(const glm::vec3& offset) {
        transform = glm::translate(transform, offset);
    }

    void rotate(float angle, const glm::vec3& axis) {
        auto rotation = glm::rotate(glm::mat4(1.0f), glm::radians(angle), axis);
        auto newForward = rotation * glm::vec4(forward, 0.0f);

        forward.x = newForward.x;
        forward.y = newForward.y;
        forward.z = newForward.z;

        transform *= rotation;
    }

    void setPosition(const glm::vec3& inPosition) {
        position = inPosition;
        transform[3][0] = position.x;
        transform[3][1] = position.y;
        transform[3][2] = position.z;
    }

    glm::vec3 getPosition() const {
        return position;
    }

	glm::vec3& getPosition() {
		return position;
	}

    void setTransform(const glm::mat4& inTransform) {
        transform = inTransform;
        position.x = transform[3][0];
        position.y = transform[3][1];
        position.z = transform[3][2];
    }

    glm::mat4 getTransform() const {
        return transform;
    }

    void computeTangentSpace();
    
    void prepare() {
        computeTangentSpace();
        prepareDraw();
    }

    void prepareDraw();

    void setName(const std::string& inName) {
        name = inName;
    }

    const std::string& getName() const {
        return name;
    }

    void addMesh(const std::shared_ptr<Mesh>& mesh) {
        meshes.push_back(mesh);
        triangleCount += mesh->getTriangleCount();
    }

    const std::vector<std::shared_ptr<Mesh>>& getMeshes() const {
        return meshes;
    }

    size_t getMeshCount() const {
        return meshes.size();
    }

    glm::vec3 getForward() const {
        return forward;
    }

    glm::vec3 getUp() const {
        return up;
    }

    glm::vec3 getRight() const {
        return right;
    }

public:
    float yaw = 0.0f;
    float pitch = 0.0f;
    float roll = 0.0f;

    bool bJumping = false;
    bool bVisible = true;
private:
    glm::vec3 forward = glm::vec3(0.0f, 0.0f, 1.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);;
    glm::vec3 right = glm::vec3(-1.0f, 0.0f, 0.0f);

    std::string name;

    glm::vec3 position;
    glm::mat4 transform;

    std::vector <std::shared_ptr<Mesh>> meshes;

    uint32_t triangleCount = 0;
};

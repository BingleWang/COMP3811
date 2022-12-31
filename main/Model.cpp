#include "Model.hpp"

#define UNUSED(x) (void)(x)

void Model::computeTangentSpace() {
    for (auto& mesh : meshes) {
        mesh->computeTangentSpace();
    }
}

void Model::prepareDraw() {
    for (auto& mesh : meshes) {
        mesh->prepareDraw();
    }
}

void Mesh::computeTangentSpace() {
    std::vector<glm::vec3 > tangents;
    std::vector<glm::vec3 > binormals;

    for (auto i = 0; i < getIndexCount(); i += 3) {

        glm::vec3 vertex0 = vertices.at(indices.at(i + 0)).position;
        glm::vec3 vertex1 = vertices.at(indices.at(i + 1)).position;
        glm::vec3 vertex2 = vertices.at(indices.at(i + 2)).position;

        glm::vec3 normal0 = vertices.at(indices.at(i + 0)).normal;
        glm::vec3 normal1 = vertices.at(indices.at(i + 1)).normal;
        glm::vec3 normal2 = vertices.at(indices.at(i + 2)).normal;

        glm::vec3 normal = glm::normalize(glm::cross((vertex1 - vertex0), (vertex2 - vertex0)));

        normal = (normal0 + normal1 + normal2) / (glm::length(normal0 + normal1 + normal2));

        glm::vec3 deltaPos;
        if (vertex0 == vertex1)
            deltaPos = vertex2 - vertex0;
        else
            deltaPos = vertex1 - vertex0;

        glm::vec2 uv0 = vertices.at(indices.at(i + 0)).texCoord;
        glm::vec2 uv1 = vertices.at(indices.at(i + 1)).texCoord;
        glm::vec2 uv2 = vertices.at(indices.at(i + 2)).texCoord;

        glm::vec2 deltaUV1 = uv1 - uv0;
        glm::vec2 deltaUV2 = uv2 - uv0;

        UNUSED(deltaUV1);
        UNUSED(deltaUV2);

        glm::vec3 tangent; // tangents
        glm::vec3 binormal; // binormal

        // avoid division with 0
        if (deltaUV1.s != 0)
            tangent = deltaPos / deltaUV1.s;
        else
            tangent = deltaPos / 1.0f;

        tangent = glm::normalize(tangent - glm::dot(normal, tangent) * normal);

        binormal = glm::normalize(glm::cross(tangent, normal));

        //vertices[indices.at(i + 0)].normal = { normal.x, normal.y, normal.z };
        //vertices[indices.at(i + 1)].normal = { normal.x, normal.y, normal.z };
        //vertices[indices.at(i + 2)].normal = { normal.x, normal.y, normal.z };

        // write into array - for each vertex of the face the same value
        vertices[indices.at(i + 0)].tangent = { tangent.x, tangent.y, tangent.z };
        vertices[indices.at(i + 1)].tangent = { tangent.x, tangent.y, tangent.z };
        vertices[indices.at(i + 2)].tangent = { tangent.x, tangent.y, tangent.z };

        vertices[indices.at(i + 0)].binormal = { binormal.x, binormal.y, binormal.z };
        vertices[indices.at(i + 1)].binormal = { binormal.x, binormal.y, binormal.z };
        vertices[indices.at(i + 2)].binormal = { binormal.x, binormal.y, binormal.z };
    }
}

void Mesh::prepareDraw() {
    glGenVertexArrays(1, &VAO);
    glBindVertexArray(VAO);

    glGenBuffers(1, &VBO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, getVertexBufferByteSize(), getVerticesData(), GL_STATIC_DRAW);

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

    glGenBuffers(1, &IBO);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, IBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, getIndexBufferByteSize(), getIndicesData(), GL_STATIC_DRAW);

    glGenVertexArrays(1, &VAONormal);
    glBindVertexArray(VAONormal);

    glGenBuffers(1, &VBONormal);
    glBindBuffer(GL_ARRAY_BUFFER, VBONormal);

    for (size_t i = 0; i < vertices.size(); i++) {
        Vertex vertex = vertices[i];
        SimpleVertex simpleVertex;
        simpleVertex.position = vertex.position;
        normals.push_back(simpleVertex);
        auto normal = vertex.normal / 5.0f;
        simpleVertex.position += normal;
        normals.push_back(simpleVertex);
    }

    stride = sizeof(SimpleVertex);

    glBufferData(GL_ARRAY_BUFFER, sizeof(SimpleVertex) * normals.size(), normals.data(), GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (void*)nullptr);
    // Map index 0 to the position buffer
    glEnableVertexAttribArray(0);	// Vertex Position

    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 3));
    // Map index 0 to the position buffer
    glEnableVertexAttribArray(1);	// Vertex Position

    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (void*)(sizeof(float) * 6));
    // Map index 0 to the position buffer
    glEnableVertexAttribArray(2);	// Vertex Position
}

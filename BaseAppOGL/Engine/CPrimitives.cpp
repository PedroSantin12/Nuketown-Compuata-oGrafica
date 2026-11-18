#pragma warning(disable : 4244)
#pragma warning(disable : 4267)

#include "CPrimitives.h"
#include <algorithm>
#include <GL/glew.h>  // or #include <glad/glad.h> depending on your setup

CPrimitives::CPrimitives() : VAO(0), VBO(0), EBO(0), buffersInitialized(false) {
}

CPrimitives::~CPrimitives() {
    cleanup();
}

void CPrimitives::clearData() {
    vertices.clear();
    indices.clear();
}

void CPrimitives::normalizeVector(float* vec) {
    float length = std::sqrt(vec[0] * vec[0] + vec[1] * vec[1] + vec[2] * vec[2]);
    if (length > 0.0f) {
        vec[0] /= length;
        vec[1] /= length;
        vec[2] /= length;
    }
}

void CPrimitives::generateCube(float size) {
    clearData();
    
    float halfSize = size / 2.0f;
    
    // Define the 6 faces of the cube
    // Each face has 4 vertices
    
    // Front face (+Z)
    vertices.push_back({{-halfSize, -halfSize,  halfSize}, {0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}});
    vertices.push_back({{ halfSize, -halfSize,  halfSize}, {1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}});
    vertices.push_back({{ halfSize,  halfSize,  halfSize}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f}});
    vertices.push_back({{-halfSize,  halfSize,  halfSize}, {0.0f, 1.0f}, {0.0f, 0.0f, 1.0f}});
    
    // Back face (-Z)
    vertices.push_back({{ halfSize, -halfSize, -halfSize}, {0.0f, 0.0f}, {0.0f, 0.0f, -1.0f}});
    vertices.push_back({{-halfSize, -halfSize, -halfSize}, {1.0f, 0.0f}, {0.0f, 0.0f, -1.0f}});
    vertices.push_back({{-halfSize,  halfSize, -halfSize}, {1.0f, 1.0f}, {0.0f, 0.0f, -1.0f}});
    vertices.push_back({{ halfSize,  halfSize, -halfSize}, {0.0f, 1.0f}, {0.0f, 0.0f, -1.0f}});
    
    // Top face (+Y)
    vertices.push_back({{-halfSize,  halfSize,  halfSize}, {0.0f, 0.0f}, {0.0f, 1.0f, 0.0f}});
    vertices.push_back({{ halfSize,  halfSize,  halfSize}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f}});
    vertices.push_back({{ halfSize,  halfSize, -halfSize}, {1.0f, 1.0f}, {0.0f, 1.0f, 0.0f}});
    vertices.push_back({{-halfSize,  halfSize, -halfSize}, {0.0f, 1.0f}, {0.0f, 1.0f, 0.0f}});
    
    // Bottom face (-Y)
    vertices.push_back({{-halfSize, -halfSize, -halfSize}, {0.0f, 0.0f}, {0.0f, -1.0f, 0.0f}});
    vertices.push_back({{ halfSize, -halfSize, -halfSize}, {1.0f, 0.0f}, {0.0f, -1.0f, 0.0f}});
    vertices.push_back({{ halfSize, -halfSize,  halfSize}, {1.0f, 1.0f}, {0.0f, -1.0f, 0.0f}});
    vertices.push_back({{-halfSize, -halfSize,  halfSize}, {0.0f, 1.0f}, {0.0f, -1.0f, 0.0f}});
    
    // Right face (+X)
    vertices.push_back({{ halfSize, -halfSize,  halfSize}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
    vertices.push_back({{ halfSize, -halfSize, -halfSize}, {1.0f, 0.0f}, {1.0f, 0.0f, 0.0f}});
    vertices.push_back({{ halfSize,  halfSize, -halfSize}, {1.0f, 1.0f}, {1.0f, 0.0f, 0.0f}});
    vertices.push_back({{ halfSize,  halfSize,  halfSize}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f}});
    
    // Left face (-X)
    vertices.push_back({{-halfSize, -halfSize, -halfSize}, {0.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}});
    vertices.push_back({{-halfSize, -halfSize,  halfSize}, {1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}});
    vertices.push_back({{-halfSize,  halfSize,  halfSize}, {1.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}});
    vertices.push_back({{-halfSize,  halfSize, -halfSize}, {0.0f, 1.0f}, {-1.0f, 0.0f, 0.0f}});
    
    // Define indices for the 6 faces (2 triangles per face)
    for (int i = 0; i < 6; i++) {
        int base = i * 4;
        indices.push_back(base);
        indices.push_back(base + 1);
        indices.push_back(base + 2);
        indices.push_back(base);
        indices.push_back(base + 2);
        indices.push_back(base + 3);
    }
}

void CPrimitives::generateSphere(float radius, int sectors, int stacks) {
    clearData();
    
    float sectorStep = 2 * M_PI / sectors;
    float stackStep = M_PI / stacks;
    
    // Generate vertices
    for (int i = 0; i <= stacks; ++i) {
        float stackAngle = M_PI / 2 - i * stackStep;  // from pi/2 to -pi/2
        float xy = radius * cosf(stackAngle);
        float z = radius * sinf(stackAngle);
        
        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;  // from 0 to 2pi
            
            PrimVertex v;
            // Position
            v.position[0] = xy * cosf(sectorAngle);
            v.position[1] = xy * sinf(sectorAngle);
            v.position[2] = z;
            
            // Texture coordinates
            v.texCoord[0] = (float)j / sectors;
            v.texCoord[1] = (float)i / stacks;
            
            // Normal (normalized position for a sphere centered at origin)
            v.normal[0] = v.position[0] / radius;
            v.normal[1] = v.position[1] / radius;
            v.normal[2] = v.position[2] / radius;
            
            vertices.push_back(v);
        }
    }
    
    // Generate indices
    for (int i = 0; i < stacks; ++i) {
        int k1 = i * (sectors + 1);
        int k2 = k1 + sectors + 1;
        
        for (int j = 0; j < sectors; ++j, ++k1, ++k2) {
            if (i != 0) {
                indices.push_back(k1);
                indices.push_back(k2);
                indices.push_back(k1 + 1);
            }
            
            if (i != (stacks - 1)) {
                indices.push_back(k1 + 1);
                indices.push_back(k2);
                indices.push_back(k2 + 1);
            }
        }
    }
}

void CPrimitives::generateCylinder(float radius, float height, int sectors) {
    clearData();
    
    float halfHeight = height / 2.0f;
    float sectorStep = 2 * M_PI / sectors;
    
    // Generate side vertices
    for (int i = 0; i <= 1; ++i) {
        float y = (i == 0) ? -halfHeight : halfHeight;
        
        for (int j = 0; j <= sectors; ++j) {
            float sectorAngle = j * sectorStep;
            float x = radius * cosf(sectorAngle);
            float z = radius * sinf(sectorAngle);
            
            PrimVertex v;
            v.position[0] = x;
            v.position[1] = y;
            v.position[2] = z;
            
            v.texCoord[0] = (float)j / sectors;
            v.texCoord[1] = (float)i;
            
            // Normal for cylinder side (perpendicular to axis)
            v.normal[0] = x / radius;
            v.normal[1] = 0.0f;
            v.normal[2] = z / radius;
            
            vertices.push_back(v);
        }
    }
    
    // Generate side indices
    for (int i = 0; i < sectors; ++i) {
        int k1 = i;
        int k2 = i + sectors + 1;
        
        indices.push_back(k1);
        indices.push_back(k2);
        indices.push_back(k1 + 1);
        
        indices.push_back(k1 + 1);
        indices.push_back(k2);
        indices.push_back(k2 + 1);
    }
    
    // Generate top and bottom caps
    int baseIndex = vertices.size();
    
    // Bottom cap center
    vertices.push_back({{0.0f, -halfHeight, 0.0f}, {0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}});
    
    for (int i = 0; i <= sectors; ++i) {
        float sectorAngle = i * sectorStep;
        float x = radius * cosf(sectorAngle);
        float z = radius * sinf(sectorAngle);
        
        vertices.push_back({{x, -halfHeight, z}, 
                           {0.5f + 0.5f * cosf(sectorAngle), 0.5f + 0.5f * sinf(sectorAngle)}, 
                           {0.0f, -1.0f, 0.0f}});
    }
    
    // Bottom cap indices
    for (int i = 0; i < sectors; ++i) {
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + i + 2);
        indices.push_back(baseIndex + i + 1);
    }
    
    // Top cap center
    int topBaseIndex = vertices.size();
    vertices.push_back({{0.0f, halfHeight, 0.0f}, {0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}});
    
    for (int i = 0; i <= sectors; ++i) {
        float sectorAngle = i * sectorStep;
        float x = radius * cosf(sectorAngle);
        float z = radius * sinf(sectorAngle);
        
        vertices.push_back({{x, halfHeight, z}, 
                           {0.5f + 0.5f * cosf(sectorAngle), 0.5f + 0.5f * sinf(sectorAngle)}, 
                           {0.0f, 1.0f, 0.0f}});
    }
    
    // Top cap indices
    for (int i = 0; i < sectors; ++i) {
        indices.push_back(topBaseIndex);
        indices.push_back(topBaseIndex + i + 1);
        indices.push_back(topBaseIndex + i + 2);
    }
}

void CPrimitives::generatePlane(float width, float height, int widthSegments, int heightSegments) {
    clearData();
    
    float halfWidth = width / 2.0f;
    float halfHeight = height / 2.0f;
    
    float segmentWidth = width / widthSegments;
    float segmentHeight = height / heightSegments;
    
    // Generate vertices
    for (int i = 0; i <= heightSegments; ++i) {
        float z = -halfHeight + i * segmentHeight;
        float v = (float)i / heightSegments;
        
        for (int j = 0; j <= widthSegments; ++j) {
            float x = -halfWidth + j * segmentWidth;
            float u = (float)j / widthSegments;
            
            PrimVertex vertex;
            vertex.position[0] = x;
            vertex.position[1] = 0.0f;
            vertex.position[2] = z;
            
            vertex.texCoord[0] = u;
            vertex.texCoord[1] = v;
            
            vertex.normal[0] = 0.0f;
            vertex.normal[1] = 1.0f;
            vertex.normal[2] = 0.0f;
            
            vertices.push_back(vertex);
        }
    }
    
    // Generate indices
    for (int i = 0; i < heightSegments; ++i) {
        for (int j = 0; j < widthSegments; ++j) {
            int a = i * (widthSegments + 1) + j;
            int b = a + widthSegments + 1;
            
            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(a + 1);
            
            indices.push_back(a + 1);
            indices.push_back(b);
            indices.push_back(b + 1);
        }
    }
}


void CPrimitives::generatePyramid(float baseSize, float height) {
    clearData();

    float h = baseSize / 2.0f;
    float top = height;

    // Base corners (Y=0)
    // BL, BR, FR, FL  (counter-clockwise from below)
    float base[4][3] = {
        {-h, 0.0f, -h},
        { h, 0.0f, -h},
        { h, 0.0f,  h},
        {-h, 0.0f,  h}
    };

    // ---- Base (facing down) ----
    {
        float n[3] = { 0.0f, 1.0f, 0.0f };
        int baseIdx = (int)vertices.size();
        vertices.push_back({ {base[0][0], base[0][1], base[0][2]}, {0.0f, 0.0f}, {n[0], n[1], n[2]} });
        vertices.push_back({ {base[1][0], base[1][1], base[1][2]}, {1.0f, 0.0f}, {n[0], n[1], n[2]} });
        vertices.push_back({ {base[2][0], base[2][1], base[2][2]}, {1.0f, 1.0f}, {n[0], n[1], n[2]} });
        vertices.push_back({ {base[3][0], base[3][1], base[3][2]}, {0.0f, 1.0f}, {n[0], n[1], n[2]} });
        // Two triangles (winding for downward face)
        indices.push_back(baseIdx + 0); indices.push_back(baseIdx + 1); indices.push_back(baseIdx + 2);
        indices.push_back(baseIdx + 0); indices.push_back(baseIdx + 2); indices.push_back(baseIdx + 3);
    }

    // ---- Four triangular side faces ----
    // Each side: two base corners + apex
    struct Side { int a; int b; };
    Side sides[4] = { {0,1},{1,2},{2,3},{3,0} };
    float apex[3] = { 0.0f, top, 0.0f };

    for (int s = 0; s < 4; ++s) {
        float* p0 = base[sides[s].a];
        float* p1 = base[sides[s].b];

        // Compute face normal via cross product
        float u[3] = { p1[0] - p0[0], p1[1] - p0[1], p1[2] - p0[2] };
        float v[3] = { apex[0] - p0[0], apex[1] - p0[1], apex[2] - p0[2] };
        float n[3] = {
            -(u[1] * v[2] - u[2] * v[1]),
            -(u[2] * v[0] - u[0] * v[2]),
            -(u[0] * v[1] - u[1] * v[0])
        };
        normalizeVector(n);

        int baseIdx = (int)vertices.size();
        vertices.push_back({ {p0[0], p0[1], p0[2]}, {0.0f, 0.0f}, {n[0], n[1], n[2]} });
        vertices.push_back({ {p1[0], p1[1], p1[2]}, {1.0f, 0.0f}, {n[0], n[1], n[2]} });
        vertices.push_back({ {apex[0], apex[1], apex[2]}, {0.5f, 1.0f}, {n[0], n[1], n[2]} });

        indices.push_back(baseIdx + 0);
        indices.push_back(baseIdx + 2);
        indices.push_back(baseIdx + 1);
    }
}


void CPrimitives::generateCone(float radius, float height, int sectors) {
    clearData();

    float halfHeight = height / 2.0f;
    float sectorStep = 2 * M_PI / sectors;
    float slantY = radius / height; // controls how "tilted" the side normal is

    // Side: base ring + apex, duplicated per sector so each triangle gets a smoothly
    // varying normal (same trick used for the cylinder's caps).
    for (int j = 0; j <= sectors; ++j) {
        float sectorAngle = j * sectorStep;
        float cosA = cosf(sectorAngle);
        float sinA = sinf(sectorAngle);

        float n[3] = { cosA, slantY, sinA };
        normalizeVector(n);

        PrimVertex vBase;
        vBase.position[0] = radius * cosA;
        vBase.position[1] = -halfHeight;
        vBase.position[2] = radius * sinA;
        vBase.texCoord[0] = (float)j / sectors;
        vBase.texCoord[1] = 0.0f;
        vBase.normal[0] = n[0]; vBase.normal[1] = n[1]; vBase.normal[2] = n[2];
        vertices.push_back(vBase);

        PrimVertex vApex;
        vApex.position[0] = 0.0f;
        vApex.position[1] = halfHeight;
        vApex.position[2] = 0.0f;
        vApex.texCoord[0] = (float)j / sectors;
        vApex.texCoord[1] = 1.0f;
        vApex.normal[0] = n[0]; vApex.normal[1] = n[1]; vApex.normal[2] = n[2];
        vertices.push_back(vApex);
    }

    for (int j = 0; j < sectors; ++j) {
        int base0 = j * 2;
        int apex0 = base0 + 1;
        int base1 = base0 + 2;

        indices.push_back(base0);
        indices.push_back(base1);
        indices.push_back(apex0);
    }

    // Bottom cap
    int baseIndex = (int)vertices.size();
    vertices.push_back({ {0.0f, -halfHeight, 0.0f}, {0.5f, 0.5f}, {0.0f, -1.0f, 0.0f} });

    for (int j = 0; j <= sectors; ++j) {
        float sectorAngle = j * sectorStep;
        float x = radius * cosf(sectorAngle);
        float z = radius * sinf(sectorAngle);

        vertices.push_back({ {x, -halfHeight, z},
                            {0.5f + 0.5f * cosf(sectorAngle), 0.5f + 0.5f * sinf(sectorAngle)},
                            {0.0f, -1.0f, 0.0f} });
    }

    for (int j = 0; j < sectors; ++j) {
        indices.push_back(baseIndex);
        indices.push_back(baseIndex + j + 2);
        indices.push_back(baseIndex + j + 1);
    }
}

void CPrimitives::generateTorus(float majorRadius, float minorRadius, int majorSegments, int minorSegments) {
    clearData();

    float majorStep = 2 * M_PI / majorSegments;
    float minorStep = 2 * M_PI / minorSegments;

    // Generate vertices
    for (int i = 0; i <= majorSegments; ++i) {
        float majorAngle = i * majorStep;
        float cosMajor = cosf(majorAngle);
        float sinMajor = sinf(majorAngle);

        for (int j = 0; j <= minorSegments; ++j) {
            float minorAngle = j * minorStep;
            float cosMinor = cosf(minorAngle);
            float sinMinor = sinf(minorAngle);

            PrimVertex v;
            v.position[0] = (majorRadius + minorRadius * cosMinor) * cosMajor;
            v.position[1] = minorRadius * sinMinor;
            v.position[2] = (majorRadius + minorRadius * cosMinor) * sinMajor;

            v.texCoord[0] = (float)i / majorSegments;
            v.texCoord[1] = (float)j / minorSegments;

            // Points from the tube's center line outward - already unit length
            v.normal[0] = cosMinor * cosMajor;
            v.normal[1] = sinMinor;
            v.normal[2] = cosMinor * sinMajor;

            vertices.push_back(v);
        }
    }

    // Generate indices
    for (int i = 0; i < majorSegments; ++i) {
        int k1 = i * (minorSegments + 1);
        int k2 = k1 + minorSegments + 1;

        for (int j = 0; j < minorSegments; ++j, ++k1, ++k2) {
            indices.push_back(k1);
            indices.push_back(k2);
            indices.push_back(k1 + 1);

            indices.push_back(k1 + 1);
            indices.push_back(k2);
            indices.push_back(k2 + 1);
        }
    }
}



void CPrimitives::setupBuffers() {
    if (buffersInitialized) {
        cleanup();
    }
    
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);
    glGenBuffers(1, &EBO);
    
    glBindVertexArray(VAO);
    
    // Upload vertex data
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(PrimVertex), vertices.data(), GL_STATIC_DRAW);
    
    // Upload index data
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    
    // Position attribute
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVertex), (void*)offsetof(PrimVertex, position));
    glEnableVertexAttribArray(0);
    
    // Normal attribute
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(PrimVertex), (void*)offsetof(PrimVertex, normal));
    glEnableVertexAttribArray(1);

    // Texture coordinate attribute
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(PrimVertex), (void*)offsetof(PrimVertex, texCoord));
    glEnableVertexAttribArray(2);
    
    glBindVertexArray(0);
    
    buffersInitialized = true;
}

void CPrimitives::render() {
    if (!buffersInitialized) {
        return;
    }
    
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
    glBindVertexArray(0);
}

void CPrimitives::cleanup() {
    if (buffersInitialized) {
        glDeleteVertexArrays(1, &VAO);
        glDeleteBuffers(1, &VBO);
        glDeleteBuffers(1, &EBO);
        buffersInitialized = false;
    }
}


// Small procedural meshes use the same position/UV/normal layout as the class primitives.
namespace
{
    struct BodySection { float z, halfWidth, bottom, top; };

    void LoftBody(std::vector<PrimVertex>& vertices, std::vector<unsigned int>& indices,
        const std::vector<BodySection>& sections)
    {
        constexpr int cornerSteps = 5;
        constexpr int ringSize = 4 * (cornerSteps + 1);
        for (const BodySection& section : sections)
        {
            const float radius = std::min(0.10f, (section.top - section.bottom) * 0.26f);
            for (int corner = 0; corner < 4; ++corner)
            {
                const float cx = (corner == 0 || corner == 3) ? section.halfWidth - radius : -section.halfWidth + radius;
                const float cy = corner < 2 ? section.top - radius : section.bottom + radius;
                for (int step = 0; step <= cornerSteps; ++step)
                {
                    const float angle = static_cast<float>((corner * 90.0 + step * 90.0 / cornerSteps) * M_PI / 180.0);
                    PrimVertex vertex{};
                    vertex.position[0] = cx + radius * std::cos(angle);
                    vertex.position[1] = cy + radius * std::sin(angle);
                    vertex.position[2] = section.z;
                    vertex.texCoord[0] = vertex.position[0] + 0.5f;
                    vertex.texCoord[1] = section.z + 0.5f;
                    vertices.push_back(vertex);
                }
            }
        }
        for (unsigned int section = 0; section + 1 < sections.size(); ++section)
            for (unsigned int point = 0; point < ringSize; ++point)
            {
                const unsigned int a = section * ringSize + point;
                const unsigned int b = section * ringSize + (point + 1) % ringSize;
                const unsigned int c = a + ringSize;
                const unsigned int d = b + ringSize;
                indices.insert(indices.end(), {a, b, c, b, d, c});
            }
        for (int end = 0; end < 2; ++end)
        {
            const unsigned int ring = end == 0 ? 0 : static_cast<unsigned int>((sections.size() - 1) * ringSize);
            PrimVertex center{};
            center.position[1] = (sections[end == 0 ? 0 : sections.size() - 1].bottom + sections[end == 0 ? 0 : sections.size() - 1].top) * 0.5f;
            center.position[2] = sections[end == 0 ? 0 : sections.size() - 1].z;
            center.texCoord[0] = center.texCoord[1] = 0.5f;
            const unsigned int middle = static_cast<unsigned int>(vertices.size());
            vertices.push_back(center);
            for (unsigned int point = 0; point < ringSize; ++point)
            {
                const unsigned int a = ring + point, b = ring + (point + 1) % ringSize;
                if (end == 0) indices.insert(indices.end(), {middle, b, a});
                else indices.insert(indices.end(), {middle, a, b});
            }
        }
        // Averaged face normals smooth the longitudinal shoulders and rounded corners.
        for (size_t triangle = 0; triangle < indices.size(); triangle += 3)
        {
            PrimVertex& a = vertices[indices[triangle]];
            PrimVertex& b = vertices[indices[triangle + 1]];
            PrimVertex& c = vertices[indices[triangle + 2]];
            const float ux = b.position[0] - a.position[0], uy = b.position[1] - a.position[1], uz = b.position[2] - a.position[2];
            const float vx = c.position[0] - a.position[0], vy = c.position[1] - a.position[1], vz = c.position[2] - a.position[2];
            const float n[] = {uy * vz - uz * vy, uz * vx - ux * vz, ux * vy - uy * vx};
            for (PrimVertex* vertex : {&a, &b, &c})
                for (int axis = 0; axis < 3; ++axis) vertex->normal[axis] += n[axis];
        }
        for (PrimVertex& vertex : vertices)
        {
            const float length = std::sqrt(vertex.normal[0] * vertex.normal[0] + vertex.normal[1] * vertex.normal[1] + vertex.normal[2] * vertex.normal[2]);
            if (length > 0.00001f)
                for (float& component : vertex.normal) component /= length;
        }
    }
}

void CPrimitives::generateRoundedCube(float radius, int segments)
{
    clearData();
    radius = std::clamp(radius, 0.001f, 0.45f);
    segments = std::max(1, segments);
    const float inner = 0.5f - radius;
    // Extra samples lie on the bevel, rather than spending the grid on a flat face.
    std::vector<float> coordinates;
    for (int index = 0; index <= segments; ++index)
        coordinates.push_back(-0.5f + radius * index / segments);
    coordinates.push_back(inner);
    for (int index = 1; index <= segments; ++index)
        coordinates.push_back(inner + radius * index / segments);
    const unsigned int side = static_cast<unsigned int>(coordinates.size());
    for (int face = 0; face < 6; ++face)
    {
        const int axis = face / 2;
        const float sign = face % 2 ? -1.0f : 1.0f;
        const unsigned int first = static_cast<unsigned int>(vertices.size());
        for (float v : coordinates)
            for (float u : coordinates)
            {
                float point[3]{};
                point[axis] = sign * 0.5f;
                point[(axis + 1) % 3] = u;
                point[(axis + 2) % 3] = v;
                float nearest[3], delta[3];
                float lengthSquared = 0.0f;
                for (int coordinate = 0; coordinate < 3; ++coordinate)
                {
                    nearest[coordinate] = std::clamp(point[coordinate], -inner, inner);
                    delta[coordinate] = point[coordinate] - nearest[coordinate];
                    lengthSquared += delta[coordinate] * delta[coordinate];
                }
                const float length = std::sqrt(lengthSquared);
                PrimVertex vertex{};
                for (int coordinate = 0; coordinate < 3; ++coordinate)
                {
                    vertex.normal[coordinate] = delta[coordinate] / length;
                    vertex.position[coordinate] = nearest[coordinate] + vertex.normal[coordinate] * radius;
                }
                vertex.texCoord[0] = u + 0.5f; vertex.texCoord[1] = v + 0.5f;
                vertices.push_back(vertex);
            }
        for (unsigned int row = 0; row + 1 < side; ++row)
            for (unsigned int column = 0; column + 1 < side; ++column)
            {
                const unsigned int a = first + row * side + column, b = a + 1, c = a + side, d = c + 1;
                if (sign > 0) indices.insert(indices.end(), {a, b, c, b, d, c});
                else indices.insert(indices.end(), {a, c, b, b, c, d});
            }
    }
}

void CPrimitives::generateCarBody()
{
    clearData();
    LoftBody(vertices, indices, {{-0.50f,0.40f,-0.21f,0.12f}, {-0.47f,0.47f,-0.40f,0.28f},
        {-0.34f,0.50f,-0.48f,0.42f}, {-0.18f,0.50f,-0.50f,0.50f},
        {0.18f,0.50f,-0.50f,0.49f}, {0.38f,0.49f,-0.43f,0.37f},
        {0.47f,0.46f,-0.33f,0.23f}, {0.50f,0.40f,-0.20f,0.09f}});
}

void CPrimitives::generateCarCabin()
{
    clearData();
    // A single glazed shell follows the sloping windscreen and rear screen.
    LoftBody(vertices, indices, {{-0.50f,0.49f,-0.50f,-0.46f}, {-0.32f,0.40f,-0.50f,0.47f},
        {-0.27f,0.40f,-0.50f,0.50f}, {0.20f,0.40f,-0.50f,0.50f},
        {0.26f,0.40f,-0.50f,0.47f}, {0.50f,0.49f,-0.50f,-0.46f}});
}

void CPrimitives::generateTruckHood()
{
    clearData();
    LoftBody(vertices, indices, {{-0.50f,0.48f,-0.50f,0.43f}, {-0.40f,0.50f,-0.50f,0.50f},
        {0.12f,0.49f,-0.48f,0.46f}, {0.35f,0.46f,-0.42f,0.35f},
        {0.47f,0.41f,-0.34f,0.22f}, {0.50f,0.35f,-0.22f,0.05f}});
}

void CPrimitives::generateWheelTorus()
{
    generateTorus(0.76f, 0.24f, 48, 14);
    // The original torus has its axle on Y; wheels use an axle on local Z.
    for (PrimVertex& vertex : vertices)
    {
        const float z = vertex.position[2];
        vertex.position[2] = vertex.position[1]; vertex.position[1] = -z;
        const float normalZ = vertex.normal[2];
        vertex.normal[2] = vertex.normal[1]; vertex.normal[1] = -normalZ;
    }
}


void CPrimitives::generateTerrainRidge()
{
    clearData();
    constexpr int columns = 96, rows = 20;
    const auto height = [](float x, float z)
    {
        const auto hash = [](float xx, float zz)
        {
            const float value = std::sin(xx * 127.1f + zz * 311.7f) * 43758.5453f;
            return value - std::floor(value);
        };
        const auto noise = [&hash](float xx, float zz)
        {
            const float ix = std::floor(xx), iz = std::floor(zz);
            float fx = xx - ix, fz = zz - iz;
            fx = fx * fx * (3.0f - 2.0f * fx);
            fz = fz * fz * (3.0f - 2.0f * fz);
            const float a = hash(ix, iz) * (1.0f - fx) + hash(ix + 1.0f, iz) * fx;
            const float b = hash(ix, iz + 1.0f) * (1.0f - fx) + hash(ix + 1.0f, iz + 1.0f) * fx;
            return a * (1.0f - fz) + b * fz;
        };
        const float relief = 0.38f + noise(x * 9.0f, z * 3.0f) * 0.43f
            + noise(x * 25.0f + 3.0f, z * 8.0f) * 0.15f
            + noise(x * 57.0f, z * 17.0f + 9.0f) * 0.055f;
        const float profile = std::pow(std::max(0.0f, std::sin(z * static_cast<float>(M_PI))), 1.05f);
        return profile * relief;
    };
    for (int row = 0; row <= rows; ++row)
        for (int column = 0; column <= columns; ++column)
        {
            const float x = static_cast<float>(column) / columns;
            const float z = static_cast<float>(row) / rows;
            constexpr float sample = 0.001f;
            float normal[] = {-(height(x + sample, z) - height(x - sample, z)) / (2.0f * sample),
                1.0f, -(height(x, std::min(1.0f, z + sample)) - height(x, std::max(0.0f, z - sample))) / (2.0f * sample)};
            normalizeVector(normal);
            vertices.push_back({{x - 0.5f, height(x,z), z - 0.5f}, {x * 12.0f, z * 3.0f}, {normal[0],normal[1],normal[2]}});
        }
    for (int row = 0; row < rows; ++row)
        for (int column = 0; column < columns; ++column)
        {
            const unsigned int a = row * (columns + 1) + column;
            const unsigned int b = a + 1, c = a + columns + 1, d = c + 1;
            indices.insert(indices.end(), {a, c, b, b, c, d});
        }
}


void CPrimitives::generateDesertBasin(float innerRadius, float outerRadius)
{
    clearData();
    innerRadius = std::max(75.0f, innerRadius);
    outerRadius = std::max(innerRadius + 30.0f, outerRadius);
    constexpr unsigned int sectors = 192, bands = 24;
    constexpr float baseY = -0.06f, bottomY = -2.0f;
    const auto height = [innerRadius, outerRadius](float x, float z)
    {
        const float radius = std::sqrt(x*x + z*z);
        if (radius <= innerRadius || radius >= outerRadius) return baseY;
        const auto hash = [](float xx, float zz)
        {
            const float value = std::sin(xx * 127.1f + zz * 311.7f) * 43758.5453f;
            return value - std::floor(value);
        };
        const auto noise = [&hash](float xx, float zz)
        {
            const float ix=std::floor(xx), iz=std::floor(zz);
            float fx=xx-ix, fz=zz-iz;
            fx=fx*fx*(3.0f-2.0f*fx); fz=fz*fz*(3.0f-2.0f*fz);
            const float a=hash(ix,iz)*(1-fx)+hash(ix+1,iz)*fx;
            const float b=hash(ix,iz+1)*(1-fx)+hash(ix+1,iz+1)*fx;
            return a*(1-fz)+b*fz;
        };
        const float t = (radius-innerRadius)/(outerRadius-innerRadius);
        const float profile=std::pow(std::max(0.0f,std::sin(t*static_cast<float>(M_PI))),1.4f);
        const float relief=18.0f+noise(x*.025f,z*.025f)*10.0f
            +noise(x*.065f+7.0f,z*.065f)*4.0f+noise(x*.15f,z*.15f+3.0f)*2.0f;
        return baseY+profile*relief;
    };
    const unsigned int row=sectors+1;
    for (unsigned int band=0; band<=bands; ++band)
        for (unsigned int sector=0; sector<=sectors; ++sector)
        {
            const float radius=innerRadius+(outerRadius-innerRadius)*band/bands;
            const float angle=sector==sectors?0.0f:static_cast<float>(2.0*M_PI*sector/sectors);
            const float x=radius*std::cos(angle), z=radius*std::sin(angle);
            constexpr float step=.25f;
            float normal[]={-(height(x+step,z)-height(x-step,z))/(2*step),1.0f,
                -(height(x,z+step)-height(x,z-step))/(2*step)};
            normalizeVector(normal);
            vertices.push_back({{x,height(x,z),z},{x*.04f,z*.04f},{normal[0],normal[1],normal[2]}});
        }
    for (unsigned int band=0;band<bands;++band)
        for (unsigned int sector=0;sector<sectors;++sector)
        {
            const unsigned int a=band*row+sector,b=a+1,c=a+row,d=c+1;
            indices.insert(indices.end(),{a,b,c,b,d,c});
        }
    // Vertical skirts and a bottom annulus close the underside of the mountains.
    unsigned int lowerRings[2]{};
    for (int edge=0;edge<2;++edge)
    {
        const float radius=edge==0?innerRadius:outerRadius;
        const unsigned int first=static_cast<unsigned int>(vertices.size());
        lowerRings[edge]=first;
        for (unsigned int sector=0;sector<=sectors;++sector)
        {
            const float angle=sector==sectors?0.0f:static_cast<float>(2.0*M_PI*sector/sectors);
            const float x=radius*std::cos(angle),z=radius*std::sin(angle);
            const float sign=edge==0?-1.0f:1.0f;
            vertices.push_back({{x,baseY,z},{0,0},{sign*std::cos(angle),0,sign*std::sin(angle)}});
            vertices.push_back({{x,bottomY,z},{0,1},{sign*std::cos(angle),0,sign*std::sin(angle)}});
        }
        for (unsigned int sector=0;sector<sectors;++sector)
        {
            const unsigned int a=first+sector*2,b=a+2,c=a+1,d=b+1;
            if(edge==1)indices.insert(indices.end(),{a,b,c,b,d,c});
            else indices.insert(indices.end(),{a,c,b,b,c,d});
        }
    }
    const unsigned int floorStart=static_cast<unsigned int>(vertices.size());
    for(unsigned int sector=0;sector<=sectors;++sector)
    {
        const float angle=sector==sectors?0.0f:static_cast<float>(2.0*M_PI*sector/sectors);
        for(float radius:{innerRadius,outerRadius})
            vertices.push_back({{radius*std::cos(angle),bottomY,radius*std::sin(angle)},{0,0},{0,-1,0}});
    }
    for(unsigned int sector=0;sector<sectors;++sector)
    {
        const unsigned int a=floorStart+sector*2,b=a+2,c=a+1,d=b+1;
        indices.insert(indices.end(),{a,c,b,b,c,d});
    }
}

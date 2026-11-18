#version 330 core

layout (location = 0) in vec3 aPosition;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec2 aTexCoord;

uniform mat4 uModel;
uniform mat4 uViewProjection;
uniform mat4 uLightViewProjection;
uniform mat3 uNormalMatrix;
uniform vec3 uObjectSize;

out vec3 worldPosition;
out vec3 worldNormal;
out vec2 texCoord;
out vec4 lightSpacePosition;
out vec3 materialPosition;
out vec3 materialNormal;

void main()
{
    vec4 position = uModel * vec4(aPosition, 1.0);
    worldPosition = position.xyz;
    lightSpacePosition = uLightViewProjection * position;
    worldNormal = uNormalMatrix * aNormal;
    texCoord = aTexCoord;
    materialPosition = aPosition * uObjectSize;
    materialNormal = aNormal;
    gl_Position = uViewProjection * position;
}

#version 330 core
out vec3 viewRay;
uniform mat4 uInverseViewProjection;
uniform vec3 uCameraPosition;
void main()
{
    vec2 position = vec2((gl_VertexID << 1) & 2, gl_VertexID & 2) * 2.0 - 1.0;
    vec4 world = uInverseViewProjection * vec4(position, 1.0, 1.0);
    viewRay = world.xyz / world.w - uCameraPosition;
    gl_Position = vec4(position, 1.0, 1.0);
}

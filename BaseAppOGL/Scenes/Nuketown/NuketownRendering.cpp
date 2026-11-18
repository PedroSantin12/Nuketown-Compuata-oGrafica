#include "SceneNuketown.h"
#include "Engine/CAsset.h"
#include <stdexcept>

glm::vec3 SceneNuketown::SunDirection()
{
    return glm::normalize(glm::vec3(-0.55f, 0.85f, 0.42f));
}

void SceneNuketown::InitializeRenderingResources()
{
    shader.LoadShader("NuketownSky", "Scenes/Nuketown/NuketownSky.vert", "Scenes/Nuketown/NuketownSky.frag");
    shader.LoadShader("NuketownShadow", "Scenes/Nuketown/NuketownShadow.vert", "Scenes/Nuketown/NuketownShadow.frag");
    glGenVertexArrays(1, &skyVao);
    glGenFramebuffers(1, &shadowFramebuffer);
    glGenTextures(1, &shadowTexture);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, shadowTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 2048, 2048, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    const float border[] = {1.0f, 1.0f, 1.0f, 1.0f};
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, border);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture, 0);
    glDrawBuffer(GL_NONE); glReadBuffer(GL_NONE);
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        throw std::runtime_error("Nuketown shadow framebuffer is incomplete");
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    const glm::vec3 sun = SunDirection();
    const glm::vec3 center(0.0f, 0.0f, -1.0f);
    lightViewProjection = glm::ortho(-43.0f, 43.0f, -36.0f, 36.0f, 1.0f, 140.0f)
        * glm::lookAt(center + sun * 70.0f, center, glm::vec3(0.0f, 1.0f, 0.0f));
    BakeSkyEnvironment();
}

void SceneNuketown::BakeSkyEnvironment()
{
    if (environmentTexture != 0)
        return;

    GLint viewport[4], drawFramebuffer = 0, readFramebuffer = 0;
    GLint polygonMode[2], activeTexture = 0, cube0 = 0, cube9 = 0, vertexArray = 0, program = 0;
    GLboolean depthMask = GL_TRUE, colorMask[4];
    const GLboolean depthTest = glIsEnabled(GL_DEPTH_TEST);
    const GLboolean blending = glIsEnabled(GL_BLEND);
    const GLboolean scissor = glIsEnabled(GL_SCISSOR_TEST);
    const GLboolean culling = glIsEnabled(GL_CULL_FACE);
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer);
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFramebuffer);
    glGetIntegerv(GL_POLYGON_MODE, polygonMode);
    glGetIntegerv(GL_ACTIVE_TEXTURE, &activeTexture);
    glGetIntegerv(GL_VERTEX_ARRAY_BINDING, &vertexArray);
    glGetIntegerv(GL_CURRENT_PROGRAM, &program);
    glGetBooleanv(GL_DEPTH_WRITEMASK, &depthMask);
    glGetBooleanv(GL_COLOR_WRITEMASK, colorMask);
    glActiveTexture(GL_TEXTURE0);
    glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &cube0);
    glActiveTexture(GL_TEXTURE9);
    glGetIntegerv(GL_TEXTURE_BINDING_CUBE_MAP, &cube9);
    // The sky sampler must not read from the face currently being rendered.
    glBindTexture(GL_TEXTURE_CUBE_MAP, 0);
    glActiveTexture(GL_TEXTURE0);

    glGenTextures(1, &environmentTexture);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentTexture);
    constexpr int faceSize = 256;
    for (unsigned int face = 0; face < 6; ++face)
        glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, 0, GL_RGBA8,
            faceSize, faceSize, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
    glEnable(GL_TEXTURE_CUBE_MAP_SEAMLESS);

    GLuint framebuffer = 0;
    glGenFramebuffers(1, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glDrawBuffer(GL_COLOR_ATTACHMENT0);
    glReadBuffer(GL_NONE);
    glViewport(0, 0, faceSize, faceSize);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    glDisable(GL_BLEND);
    glDisable(GL_SCISSOR_TEST);
    glDisable(GL_CULL_FACE);
    glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);

    const auto restore = [&]()
    {
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer);
        glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);
        glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
        glPolygonMode(GL_FRONT_AND_BACK, polygonMode[0]);
        glDepthMask(depthMask);
        glColorMask(colorMask[0], colorMask[1], colorMask[2], colorMask[3]);
        if (depthTest) glEnable(GL_DEPTH_TEST); else glDisable(GL_DEPTH_TEST);
        if (blending) glEnable(GL_BLEND); else glDisable(GL_BLEND);
        if (scissor) glEnable(GL_SCISSOR_TEST); else glDisable(GL_SCISSOR_TEST);
        if (culling) glEnable(GL_CULL_FACE); else glDisable(GL_CULL_FACE);
        glBindVertexArray(vertexArray);
        glUseProgram(program);
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cube0);
        glActiveTexture(GL_TEXTURE9);
        glBindTexture(GL_TEXTURE_CUBE_MAP, cube9);
        glActiveTexture(activeTexture);
    };

    const glm::vec3 directions[] = {
        {1, 0, 0}, {-1, 0, 0}, {0, 1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}
    };
    const glm::vec3 up[] = {
        {0, -1, 0}, {0, -1, 0}, {0, 0, 1}, {0, 0, -1}, {0, -1, 0}, {0, -1, 0}
    };
    const glm::mat4 projection = glm::perspective(glm::radians(90.0f), 1.0f, 0.1f, 10.0f);
    shader.Use("NuketownSky");
    shader.SetBool("uBakeSky", true);
    shader.SetInt("uEnvironmentMap", 9);
    shader.SetVec3("uCameraPosition", glm::vec3(0.0f));
    shader.SetVec3("uSunDirection", SunDirection());
    glBindVertexArray(skyVao);
    for (unsigned int face = 0; face < 6; ++face)
    {
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_CUBE_MAP_POSITIVE_X + face, environmentTexture, 0);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            restore();
            glDeleteFramebuffers(1, &framebuffer);
            glDeleteTextures(1, &environmentTexture);
            environmentTexture = 0;
            throw std::runtime_error("Nuketown sky environment framebuffer is incomplete");
        }
        const glm::mat4 view = glm::lookAt(glm::vec3(0.0f), directions[face], up[face]);
        shader.SetMat4("uInverseViewProjection", glm::inverse(projection * view));
        glDrawArrays(GL_TRIANGLES, 0, 3);
    }
    shader.SetBool("uBakeSky", false);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentTexture);
    glGenerateMipmap(GL_TEXTURE_CUBE_MAP);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    restore();
    glDeleteFramebuffers(1, &framebuffer);
}

void SceneNuketown::RenderShadowMap()
{
    GLint viewport[4], framebuffer = 0;
    glGetIntegerv(GL_VIEWPORT, viewport);
    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &framebuffer);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFramebuffer);
    glViewport(0, 0, 2048, 2048);
    glClear(GL_DEPTH_BUFFER_BIT);
    glEnable(GL_DEPTH_TEST);
    glDepthMask(GL_TRUE);
    glDisable(GL_BLEND);
    glEnable(GL_POLYGON_OFFSET_FILL);
    glPolygonOffset(1.0f, 1.0f);
    shader.Use("NuketownShadow");
    shader.SetMat4("uLightViewProjection", lightViewProjection);
    for (const Part& part : parts)
    {
        if (part.surface == Surface::Shadow || part.surface == Surface::Mountains
            || part.surface == Surface::RefractiveGlass) continue;
        shader.SetMat4("uModel", part.model);
        part.mesh->render();
    }
    if (busAsset)
    {
        shader.SetMat4("uModel", busModel);
        busAsset->Draw(shader.GetProgram("NuketownShadow"));
    }
    glDisable(GL_POLYGON_OFFSET_FILL);
    glBindFramebuffer(GL_FRAMEBUFFER, framebuffer);
    glViewport(viewport[0], viewport[1], viewport[2], viewport[3]);
    glActiveTexture(GL_TEXTURE0);
    // All map geometry is static. Reuse its light-space depth until the map changes.
    shadowReady = true;
}

void SceneNuketown::DrawSky(const glm::mat4& viewProjection, glm::vec3 viewPosition)
{
    GLint polygonMode[2];
    glGetIntegerv(GL_POLYGON_MODE, polygonMode);
    glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
    glDisable(GL_DEPTH_TEST);
    glDepthMask(GL_FALSE);
    shader.Use("NuketownSky");
    shader.SetBool("uBakeSky", false);
    shader.SetInt("uEnvironmentMap", 9);
    glActiveTexture(GL_TEXTURE9);
    glBindTexture(GL_TEXTURE_CUBE_MAP, environmentTexture);
    shader.SetMat4("uInverseViewProjection", glm::inverse(viewProjection));
    shader.SetVec3("uCameraPosition", viewPosition);
    shader.SetVec3("uSunDirection", SunDirection());
    glBindVertexArray(skyVao);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
    glActiveTexture(GL_TEXTURE0);
    glDepthMask(GL_TRUE);
    glEnable(GL_DEPTH_TEST);
    glPolygonMode(GL_FRONT_AND_BACK, polygonMode[0]);
}

void SceneNuketown::CaptureOpaqueScene(int width, int height)
{
    GLint readFramebuffer = 0, drawFramebuffer = 0;
    glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFramebuffer);
    glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFramebuffer);
    if (refractionFramebuffer == 0)
    {
        glGenFramebuffers(1, &refractionFramebuffer);
        glGenTextures(1, &refractionColorTexture);
        glGenTextures(1, &refractionDepthTexture);
    }
    if (refractionWidth != width || refractionHeight != height)
    {
        // Depth blits require matching source and destination formats. Query the
        // GLFW framebuffer instead of assuming a particular driver's depth format.
        GLint depthBits = 24, stencilBits = 0, depthType = GL_UNSIGNED_NORMALIZED;
        glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER,
            readFramebuffer == 0 ? GL_DEPTH : GL_DEPTH_ATTACHMENT,
            GL_FRAMEBUFFER_ATTACHMENT_DEPTH_SIZE, &depthBits);
        glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER,
            readFramebuffer == 0 ? GL_DEPTH : GL_DEPTH_ATTACHMENT,
            GL_FRAMEBUFFER_ATTACHMENT_COMPONENT_TYPE, &depthType);
        glGetFramebufferAttachmentParameteriv(GL_READ_FRAMEBUFFER,
            readFramebuffer == 0 ? GL_STENCIL : GL_STENCIL_ATTACHMENT,
            GL_FRAMEBUFFER_ATTACHMENT_STENCIL_SIZE, &stencilBits);
        glBindFramebuffer(GL_DRAW_FRAMEBUFFER, refractionFramebuffer);
        glDrawBuffer(GL_COLOR_ATTACHMENT0);
        glActiveTexture(GL_TEXTURE11);
        glBindTexture(GL_TEXTURE_2D, refractionColorTexture);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
            GL_TEXTURE_2D, refractionColorTexture, 0);
        glActiveTexture(GL_TEXTURE15);
        glBindTexture(GL_TEXTURE_2D, refractionDepthTexture);
        if (stencilBits > 0)
        {
            const bool floating = depthType == GL_FLOAT;
            glTexImage2D(GL_TEXTURE_2D, 0, floating ? GL_DEPTH32F_STENCIL8 : GL_DEPTH24_STENCIL8,
                width, height, 0, GL_DEPTH_STENCIL,
                floating ? GL_FLOAT_32_UNSIGNED_INT_24_8_REV : GL_UNSIGNED_INT_24_8, nullptr);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT,
                GL_TEXTURE_2D, refractionDepthTexture, 0);
        }
        else
        {
            const GLenum format = depthType == GL_FLOAT ? GL_DEPTH_COMPONENT32F
                : depthBits <= 16 ? GL_DEPTH_COMPONENT16
                : depthBits <= 24 ? GL_DEPTH_COMPONENT24 : GL_DEPTH_COMPONENT32;
            glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0,
                GL_DEPTH_COMPONENT, depthType == GL_FLOAT ? GL_FLOAT : GL_UNSIGNED_INT, nullptr);
            glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, GL_DEPTH_ATTACHMENT,
                GL_TEXTURE_2D, refractionDepthTexture, 0);
        }
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_COMPARE_MODE, GL_NONE);
        if (glCheckFramebufferStatus(GL_DRAW_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
        {
            glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer);
            glActiveTexture(GL_TEXTURE0);
            throw std::runtime_error("Nuketown refraction framebuffer is incomplete");
        }
        refractionWidth = width;
        refractionHeight = height;
    }
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, refractionFramebuffer);
    glBlitFramebuffer(0, 0, width, height, 0, 0, width, height,
        GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT, GL_NEAREST);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, drawFramebuffer);
    glBindFramebuffer(GL_READ_FRAMEBUFFER, readFramebuffer);
    glActiveTexture(GL_TEXTURE0);
}

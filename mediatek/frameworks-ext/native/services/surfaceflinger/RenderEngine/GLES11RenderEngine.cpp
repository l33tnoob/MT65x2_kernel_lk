/*
 * Copyright 2013 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#include <GLES/gl.h>
#include <GLES/glext.h>

#include <utils/String8.h>

#include <cutils/xlog.h>
#include <cutils/compiler.h>

#include "RenderEngine/GLES11RenderEngine.h"
#include "RenderEngine/Mesh.h"
#include "RenderEngine/Texture.h"

#include <SkImageDecoder.h>
#include <SkBitmap.h>

// ---------------------------------------------------------------------------
namespace android {
// ---------------------------------------------------------------------------

uint32_t GLES11RenderEngine::createProtectedImageTextureLocked() {
    SkBitmap bitmap;
    if (false == SkImageDecoder::DecodeFile(DRM_IMAGE_PATH, &bitmap)) {
        XLOGE("Failed to load DRM image");
        return -1U;
    }

    GLuint texName;
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.width(), bitmap.height(), 0,
            GL_RGBA, GL_UNSIGNED_BYTE, bitmap.getPixels());

    return texName;
}

void GLES11RenderEngine::setupLayerProtectedImage() {
    // setup texture
    {
        glBindTexture(GL_TEXTURE_2D, getProtectedImageTexName());
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameterx(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glMatrixMode(GL_TEXTURE);
        glLoadIdentity();
        glMatrixMode(GL_MODELVIEW);
        glDisable(GL_TEXTURE_EXTERNAL_OES);
        glEnable(GL_TEXTURE_2D);
    }

    // setup blending
    {
        glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_COMBINE);
        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_RGB, GL_MODULATE);

        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_RGB, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_RGB, GL_SRC_COLOR);

        glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_RGB, GL_CONSTANT);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_RGB, GL_SRC_COLOR);

        glTexEnvi(GL_TEXTURE_ENV, GL_COMBINE_ALPHA, GL_MODULATE);

        glTexEnvi(GL_TEXTURE_ENV, GL_SRC0_ALPHA, GL_CONSTANT);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND0_ALPHA, GL_SRC_ALPHA);

        glTexEnvi(GL_TEXTURE_ENV, GL_SRC1_ALPHA, GL_TEXTURE);
        glTexEnvi(GL_TEXTURE_ENV, GL_OPERAND1_ALPHA, GL_SRC_ALPHA);

        GLfloat envColor[4];
        envColor[0] = 1;
        envColor[1] = 3;
        envColor[2] = 3;
        envColor[3] = 3;
        glTexEnvfv(GL_TEXTURE_ENV, GL_TEXTURE_ENV_COLOR, envColor);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }
}

void GLES11RenderEngine::setViewportAndProjection(
        const sp<const DisplayDevice>& hw, size_t vpw, size_t vph) {
    glViewport(0, 0, vpw, vph);
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    mat4 m;
    getHwInverseMatrix(hw, m);
    glLoadMatrixf(m.asArray());

    glMatrixMode(GL_MODELVIEW);
}

// ---------------------------------------------------------------------------
}; // namespace android
// ---------------------------------------------------------------------------

#if defined(__gl2_h_)
#error "don't include gl2/gl2.h in this file"
#endif

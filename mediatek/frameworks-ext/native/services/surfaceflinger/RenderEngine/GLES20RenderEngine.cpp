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

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <utils/String8.h>
#include <utils/Trace.h>

#include <cutils/xlog.h>
#include <cutils/compiler.h>

#include "RenderEngine/GLES20RenderEngine.h"
#include "RenderEngine/Program.h"
#include "RenderEngine/ProgramCache.h"
#include "RenderEngine/Description.h"
#include "RenderEngine/Mesh.h"
#include "RenderEngine/Texture.h"

#include <SkImageDecoder.h>
#include <SkBitmap.h>

// ---------------------------------------------------------------------------
namespace android {
// ---------------------------------------------------------------------------

uint32_t GLES20RenderEngine::createProtectedImageTextureLocked() {
    SkBitmap bitmap;
    if (false == SkImageDecoder::DecodeFile(DRM_IMAGE_PATH, &bitmap)) {
        XLOGE("Failed to load DRM image");
        return -1U;
    }

    GLuint texName;
    glGenTextures(1, &texName);
    glBindTexture(GL_TEXTURE_2D, texName);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, bitmap.width(), bitmap.height(), 0,
            GL_RGBA, GL_UNSIGNED_BYTE, bitmap.getPixels());

    return texName;
}

void GLES20RenderEngine::setupLayerProtectedImage() {
    // setup texture
    {
        // TODO: should get from image size
        int w = 1;
        int h = 1;
        glBindTexture(GL_TEXTURE_2D, getProtectedImageTexName());
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        Texture texture(Texture::TEXTURE_2D, mProtectedTexName);
        texture.setDimensions(w, h);
        mState.setTexture(texture);
    }

    // setup blending
    {
        mState.setPlaneAlpha(1.0f);
        mState.setPremultipliedAlpha(true);
        mState.setOpaque(false);
        mState.setColor(3, 3, 3, 1);

        glEnable(GL_BLEND);
        glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
    }
}

void GLES20RenderEngine::setViewportAndProjection(
        const sp<const DisplayDevice>& hw, size_t vpw, size_t vph) {
    glViewport(0, 0, vpw, vph);

    mat4 m;
    getHwInverseMatrix(hw, m);
    mState.setProjectionMatrix(m);

    mVpWidth = vpw;
    mVpHeight = vph;
}

// ---------------------------------------------------------------------------
}; // namespace android
// ---------------------------------------------------------------------------

#if defined(__gl_h_)
#error "don't include gl/gl.h in this file"
#endif

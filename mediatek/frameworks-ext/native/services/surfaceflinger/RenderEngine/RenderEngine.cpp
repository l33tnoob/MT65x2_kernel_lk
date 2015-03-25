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

#include <cutils/xlog.h>
#include <ui/Rect.h>
#include <ui/Region.h>

#include "RenderEngine/RenderEngine.h"
#include "RenderEngine/GLExtensions.h"
#include "RenderEngine/Mesh.h"

#include <private/gui/LayerState.h>
#include <binder/IInterface.h>
#include "DisplayDevice.h"

// ---------------------------------------------------------------------------
namespace android {
// ---------------------------------------------------------------------------

uint32_t RenderEngine::getProtectedImageTexName() {
    Mutex::Autolock _l(mProtectedImageLock);
    if (mProtectedImageTexName == -1U) {
        mProtectedImageTexName = createProtectedImageTextureLocked();
    }
    return mProtectedImageTexName;
}

uint32_t RenderEngine::deleteProtectedImageTexture() {
    Mutex::Autolock _l(mProtectedImageLock);
    GLuint texName = mProtectedImageTexName;
    if (mProtectedImageTexName != -1U) {
        mProtectedImageTexName = -1U;
    }
    return texName;
}

void RenderEngine::getHwInverseMatrix(const sp<const DisplayDevice>& hw, mat4& inv) {
    // get inversed width/height
    const float iw = 2.0 / hw->getWidth();
    const float ih = 2.0 / hw->getHeight();

    // map to required matrix
    // since in display case, we need only orientation, but not free-form
    // so just use pre-calculated cases in switch

    // setup fixed part first
    inv[0][2] =  0;    inv[0][3] =  0;
    inv[1][2] =  0;    inv[1][3] =  0;
    inv[2][2] = -2;    inv[2][3] =  0;
    inv[3][2] = -1;    inv[3][3] =  1;

    // switch map to set x/y matrix entries
    switch (hw->getHwOrientation()) {
        case DisplayState::eOrientationDefault:
            inv[0][0] =  iw;    inv[0][1] =   0;
            inv[1][0] =   0;    inv[1][1] = -ih;
            inv[2][0] =   0;    inv[2][1] =   0;
            inv[3][0] =  -1;    inv[3][1] =   1;
            break;
        case DisplayState::eOrientation90:
            inv[0][0] =   0;    inv[0][1] = -iw;
            inv[1][0] = -ih;    inv[1][1] =   0;
            inv[2][0] =   0;    inv[2][1] =   0;
            inv[3][0] =   1;    inv[3][1] =   1;
            break;
        case DisplayState::eOrientation180:
            inv[0][0] = -iw;    inv[0][1] =   0;
            inv[1][0] =   0;    inv[1][1] =  ih;
            inv[2][0] =   0;    inv[2][1] =   0;
            inv[3][0] =   1;    inv[3][1] =  -1;
            break;
        case DisplayState::eOrientation270:
            inv[0][0] =   0;    inv[0][1] =  iw;
            inv[1][0] =  ih;    inv[1][1] =   0;
            inv[2][0] =   0;    inv[2][1] =   0;
            inv[3][0] =  -1;    inv[3][1] =  -1;
            break;
        default:
            XLOGW("[%s] unknown orientation:%d, set to default",
                __func__, hw->getHwOrientation());
            inv[0][0] =  iw;    inv[0][1] =   0;
            inv[1][0] =   0;    inv[1][1] = -ih;
            inv[2][0] =   0;    inv[2][1] =   0;
            inv[3][0] =  -1;    inv[3][1] =   1;
    }
}

void RenderEngine::drawDebugLine(uint32_t w, uint32_t h) const {
    static uint32_t x = 0;

    glEnable(GL_SCISSOR_TEST);
    glScissor(x, 0, 10, h);
    glClearColor(1.0, 0.0, 0.0, 1.0);
    glClear(GL_COLOR_BUFFER_BIT);
    glDisable(GL_SCISSOR_TEST);

    x += 10;
    if (x > w) {
        x = 0;
    }
}

// ---------------------------------------------------------------------------
}; // namespace android
// ---------------------------------------------------------------------------

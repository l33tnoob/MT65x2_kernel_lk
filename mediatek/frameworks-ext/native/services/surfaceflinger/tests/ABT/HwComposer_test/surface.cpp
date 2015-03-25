/*
 * Copyright (C) 2011 The Android Open Source Project
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
 *
 */

/*
 * Hardware Composer Color Equivalence
 *
 * Synopsis
 *   hwc_colorequiv [options] eFmt
 *
 *     options:
         -v - verbose
 *       -s <0.##, 0.##, 0.##> - Start color (default: <0.0, 0.0, 0.0>
 *       -e <0.##, 0.##, 0.##> - Ending color (default: <1.0, 1.0, 1.0>
 *       -r fmt - reference graphic format
 *       -D #.## - End of test delay
 *
 *     graphic formats:
 *       RGBA8888 (reference frame default)
 *       RGBX8888
 *       RGB888
 *       RGB565
 *       BGRA8888
 *       RGBA5551
 *       RGBA4444
 *       YV12
 *
 * Description
 *   Renders a horizontal blend in two frames.  The first frame is rendered
 *   in the upper third of the display and is called the reference frame.
 *   The second frame is displayed in the middle third and is called the
 *   equivalence frame.  The primary purpose of this utility is to verify
 *   that the colors produced in the reference and equivalence frames are
 *   the same.  The colors are the same when the colors are the same
 *   vertically between the reference and equivalence frames.
 *
 *   By default the reference frame is rendered through the use of the
 *   RGBA8888 graphic format.  The -r option can be used to specify a
 *   non-default reference frame graphic format.  The graphic format of
 *   the equivalence frame is determined by a single required positional
 *   parameter.  Intentionally there is no default for the graphic format
 *   of the equivalence frame.
 *
 *   The horizontal blend in the reference frame is produced from a linear
 *   interpolation from a start color (default: <0.0, 0.0, 0.0> on the left
 *   side to an end color (default <1.0, 1.0, 1.0> on the right side.  Where
 *   possible the equivalence frame is rendered with the equivalent color
 *   from the reference frame.  A color of black is used in the equivalence
 *   frame for cases where an equivalent color does not exist.
 */

#include <algorithm>
#include <assert.h>
#include <cerrno>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <libgen.h>
#include <sched.h>
#include <sstream>
#include <stdint.h>
#include <string.h>
#include <unistd.h>
#include <vector>
#include <list>

#include <sys/syscall.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <EGL/egl.h>
#include <EGL/eglext.h>
#include <GLES2/gl2.h>
#include <GLES2/gl2ext.h>

#include <ui/FramebufferNativeWindow.h>
#include <ui/GraphicBuffer.h>

#define LOG_TAG "hwcColorEquivTest"
#include <utils/Log.h>
#include <testUtil.h>

#include <hardware/hwcomposer.h>

#include "hwcTestLib.h"
#include <SkImageEncoder.h>
#include <SkBitmap.h>
#include "../common/surfaceTestLib.h"

using namespace std;
using namespace android;

// Defaults for command-line options
const bool defaultVerbose = false;
const ColorFract defaultStartColor(0.0, 0.0, 0.0);
const ColorFract defaultEndColor(1.0, 1.0, 1.0);
const char *defaultRefFormat = "RGBA8888";
const float defaultEndDelay = 2.0; // Default delay after rendering graphics

// Defines
#define MAXSTR               100
#define MAXCMD               200
#define BITSPERBYTE            8 // TODO: Obtain from <values.h>, once
                                 // it has been added

#define CMD_STOP_FRAMEWORK   "stop 2>&1"
#define CMD_START_FRAMEWORK  "start 2>&1"

// Macros
#define NUMA(a) (sizeof(a) / sizeof(a [0])) // Num elements in an array
#define MEMCLR(addr, size) do { \
        memset((addr), 0, (size)); \
    } while (0)

// Globals
static const int texUsage = GraphicBuffer::USAGE_HW_TEXTURE |
        GraphicBuffer::USAGE_SW_WRITE_RARELY;
static hwc_composer_device_t *hwcDevice;
static EGLDisplay dpy;
static EGLSurface surface;
//static EGLint width, height;
static bool verbose = defaultVerbose;

#define DRAW_FHD_W 1080
#define DRAW_FHD_H 1920


#define ENABLE_HWC_RECT_TEST
#ifdef ENABLE_HWC_RECT_TEST
const uint32_t   defaultFormat = HAL_PIXEL_FORMAT_RGBA_8888;
const int32_t    defaultTransform = 0;
const uint32_t   defaultBlend = HWC_BLENDING_NONE;
const ColorFract defaultColor(0.5, 0.5, 0.5);
const float      defaultAlpha = 1.0; // Opaque
const HwcTestDim defaultSourceDim(1, 1);
const struct hwc_rect defaultSourceCrop = {0, 0, 1, 1};
const struct hwc_rect defaultDisplayFrame = {0, 0, 100, 100};

// Local types
class Rectangle {
public:
    Rectangle() : format(defaultFormat), transform(defaultTransform),
                  blend(defaultBlend), color(defaultColor),
                  alpha(defaultAlpha), sourceDim(defaultSourceDim),
                  sourceCrop(defaultSourceCrop),
                  displayFrame(defaultDisplayFrame) {};

    uint32_t     format;
    uint32_t     transform;
    int32_t      blend;
    ColorFract   color;
    float        alpha;
    HwcTestDim   sourceDim;
    struct hwc_rect   sourceCrop;
    struct hwc_rect   displayFrame;

    sp<GraphicBuffer> texture;
};


// Globals
list<Rectangle> rectangle;


// Parse string description of rectangle and add it to list of rectangles
// to be rendered.
static Rectangle parseRect(string rectStr)
{
    int rv;
    string str;
    bool   error;
    istringstream in(rectStr);
    const struct hwcTestGraphicFormat *format;
    Rectangle rect;
    struct hwc_rect hwcRect;

    // Graphic Format
    in >> str;
    if (!in) {
        testPrintE("Error parsing format from: %s", rectStr.c_str());
        exit(20);
    }
    format = hwcTestGraphicFormatLookup(str.c_str());
    if (format == NULL) {
        testPrintE("Unknown graphic format in: %s", rectStr.c_str());
        exit(21);
    }
    rect.format = format->format;

    // Display Frame
    rect.displayFrame = hwcTestParseHwcRect(in, error);
    if (error) {
        testPrintE("Invalid display frame in: %s", rectStr.c_str());
        exit(22);
    }

    // Set default sourceDim and sourceCrop based on size of display frame.
    // Default is source size equal to the size of the display frame, with
    // the source crop being the entire size of the source frame.
    rect.sourceDim = HwcTestDim(rect.displayFrame.right
                                     - rect.displayFrame.left,
                                 rect.displayFrame.bottom
                                     - rect.displayFrame.top);
    rect.sourceCrop.left = 0;
    rect.sourceCrop.top = 0;
    rect.sourceCrop.right = rect.sourceDim.width();
    rect.sourceCrop.bottom = rect.sourceDim.height();

    // Optional settings
    while ((in.tellg() < (streampos) in.str().length())
           && (in.tellg() != (streampos) -1)) {
        string attrName;

        in >> attrName;
        if (in.eof()) { break; }
        if (!in) {
            testPrintE("Error reading attribute name in: %s",
                       rectStr.c_str());
            exit(23);
        }

        // Transform
        if (attrName == "transform:") { // Transform
            string str;

            in >> str;
            if (str == "none") {
                rect.transform = 0;
            } else if (str == "fliph") {
                rect.transform = HWC_TRANSFORM_FLIP_H;
            } else if (str == "flipv") {
                rect.transform = HWC_TRANSFORM_FLIP_V;
            } else if (str == "rot90") {
                rect.transform = HWC_TRANSFORM_ROT_90;
            } else if (str == "rot180") {
                rect.transform = HWC_TRANSFORM_ROT_180;
            } else if (str == "rot270") {
                rect.transform = HWC_TRANSFORM_ROT_270;
            } else {
                testPrintE("Unknown transform of \"%s\" in: %s", str.c_str(),
                           rectStr.c_str());
                exit(24);
            }
        } else if (attrName == "blend:") { // Blend
            string str;

            in >> str;
            if (str == string("none")) {
                rect.blend = HWC_BLENDING_NONE;
            } else if (str == "premult") {
                rect.blend = HWC_BLENDING_PREMULT;
            } else if (str == "coverage") {
                rect.blend = HWC_BLENDING_COVERAGE;
            } else {
                testPrintE("Unknown blend of \"%s\" in: %s", str.c_str(),
                           rectStr.c_str());
                exit(25);
            }
        } else if (attrName == "color:") { // Color
            rect.color = hwcTestParseColor(in, error);
            if (error) {
                testPrintE("Error parsing color in: %s", rectStr.c_str());
                exit(26);
            }
        } else if (attrName == "alpha:") { // Alpha
            in >> rect.alpha;
            if (!in) {
                testPrintE("Error parsing value for alpha attribute in: %s",
                           rectStr.c_str());
                exit(27);
            }
        } else if (attrName == "sourceDim:") { // Source Dimension
           rect.sourceDim = hwcTestParseDim(in, error);
            if (error) {
                testPrintE("Error parsing source dimenision in: %s",
                           rectStr.c_str());
                exit(28);
            }
        } else if (attrName == "sourceCrop:") { // Source Crop
            rect.sourceCrop = hwcTestParseHwcRect(in, error);
            if (error) {
                testPrintE("Error parsing source crop in: %s",
                           rectStr.c_str());
                exit(29);
            }
        } else { // Unknown attribute
            testPrintE("Unknown attribute of \"%s\" in: %s", attrName.c_str(),
                       rectStr.c_str());
            exit(30);
        }
    }

    // Validate
    if (((uint32_t) rect.sourceCrop.left >= rect.sourceDim.width())
        || ((uint32_t) rect.sourceCrop.right > rect.sourceDim.width())
        || ((uint32_t) rect.sourceCrop.top >= rect.sourceDim.height())
        || ((uint32_t) rect.sourceCrop.bottom > rect.sourceDim.height())) {
        testPrintE("Invalid source crop in: %s", rectStr.c_str());
        exit(31);
    }
    if ((rect.displayFrame.left >= DRAW_FHD_W)
        || (rect.displayFrame.right > DRAW_FHD_W)
        || (rect.displayFrame.top >= DRAW_FHD_H)
        || (rect.displayFrame.bottom > DRAW_FHD_H)) {
        testPrintE("Invalid display frame in: %s", rectStr.c_str());
        exit(32);
    }
    if ((rect.alpha < 0.0) || (rect.alpha > 1.0)) {
        testPrintE("Invalid alpha in: %s", rectStr.c_str());
        exit(33);
    }

    // Create source texture
    rect.texture = new GraphicBuffer(rect.sourceDim.width(),
                                     rect.sourceDim.height(),
                                     rect.format, texUsage);
    if ((rv = rect.texture->initCheck()) != NO_ERROR) {
        testPrintE("source texture initCheck failed, rv: %i", rv);
        testPrintE("  %s", rectStr.c_str());

    }

    // Fill with uniform color
    hwcTestFillColor(rect.texture.get(), rect.color, rect.alpha);
    if (verbose) {
        testPrintI("    buf: %p handle: %p format: %s width: %u height: %u "
                   "color: %s alpha: %f",
                   rect.texture.get(), rect.texture->handle, format->desc,
                   rect.sourceDim.width(), rect.sourceDim.height(),
                   string(rect.color).c_str(), rect.alpha);
    }

    return rect;
}

#endif
// Functions prototypes
void init(void);
void printSyntax(const char *cmd);

char cmd_path[128]={"/data/hwcomposer_test"};

// Command-line option settings
static ColorFract startRefColor = defaultStartColor;
static ColorFract endRefColor = defaultEndColor;
static float endDelay = defaultEndDelay;
static const struct hwcTestGraphicFormat *refFormat
    = hwcTestGraphicFormatLookup(defaultRefFormat);
static const struct hwcTestGraphicFormat *equivFormat
    = hwcTestGraphicFormatLookup(defaultRefFormat);

/*
 * Main
 *
 * Performs the following high-level sequence of operations:
 *
 *   1. Command-line parsing
 *
 *   2. Stop framework
 *
 *   3. Initialization
 *
 *   4. Create Hardware Composer description of reference and equivalence frames
 *
 *   5. Have Hardware Composer render the reference and equivalence frames
 *
 *   6. Delay for amount of time given by endDelay
 *
 *   7. Start framework
 */
int
main(int argc, char *argv[])
{
    int rv, opt;
    bool error;
    char *chptr;
    unsigned int pass;
    char cmd[MAXCMD];
    string str;
    bool saveGolden = false;
    static char golden_data_path[256]={0};
    char *golden_data = NULL;

    testSetLogCatTag(LOG_TAG);

    assert(refFormat != NULL);

    testSetLogCatTag(LOG_TAG);

    // Parse command line arguments
    while ((opt = getopt(argc, argv, "vg?h")) != -1) {
        switch (opt) {

          case 'v': // Verbose
            verbose = true;
            break;

          case 'g': // save golden data
            saveGolden = true;
            if(optarg)
            {
                testPrintI("optarg: %s", optarg);
                golden_data = golden_data_path;
                strcpy(golden_data, optarg);
            }            
           
            break;

          case 'h': // Help
          case '?':
          default:
            printSyntax(basename(argv[0]));
            exit(((optopt == 0) || (optopt == '?')) ? 0 : 5);
        }
    }

    testPrintI("refFormat: %u %s", refFormat->format, refFormat->desc);
    testPrintI("equivFormat: %u %s", equivFormat->format, equivFormat->desc);
    testPrintI("startRefColor: %s", ((string) startRefColor).c_str());
    testPrintI("endRefColor: %s", ((string) endRefColor).c_str());
    testPrintI("endDelay: %f", endDelay);

    // Stop framework
    rv = snprintf(cmd, sizeof(cmd), "%s", CMD_STOP_FRAMEWORK);
    if (rv >= (signed) sizeof(cmd) - 1) {
        testPrintE("Command too long for: %s", CMD_STOP_FRAMEWORK);
        exit(8);
    }
    testExecCmd(cmd);
    testDelay(1.0); // TODO - needs means to query whether asynchronous stop
                    // framework operation has completed.  For now, just wait
                    // a long time.

    init();

    // Use the upper third of the display for the reference frame and
    // the middle third for the equivalence frame.
    unsigned int refHeight = DRAW_FHD_H / 3;
    unsigned int refPosY = 0; // Reference frame Y position
    unsigned int refPosX = 0; // Reference frame X position
    unsigned int refWidth = DRAW_FHD_W - refPosX;
    if ((refWidth & refFormat->wMod) != 0) {
        refWidth += refFormat->wMod - (refWidth % refFormat->wMod);
    }
    unsigned int equivHeight = DRAW_FHD_H / 3;
    unsigned int equivPosY = refHeight; // Equivalence frame Y position
    unsigned int equivPosX = 0;         // Equivalence frame X position
    unsigned int equivWidth = DRAW_FHD_W - equivPosX;
    if ((equivWidth & equivFormat->wMod) != 0) {
        equivWidth += equivFormat->wMod - (equivWidth % equivFormat->wMod);
    }

    // Create reference and equivalence graphic buffers
    const unsigned int numFrames = 2;
    sp<GraphicBuffer> refFrame;
    refFrame = new GraphicBuffer(refWidth, refHeight,
                                 refFormat->format, texUsage);
    if ((rv = refFrame->initCheck()) != NO_ERROR) {
        testPrintE("refFrame initCheck failed, rv: %i", rv);
        testPrintE("  width %u height: %u format: %u %s", refWidth, refHeight,
                   refFormat->format,
                   hwcTestGraphicFormat2str(refFormat->format));
        exit(9);
    }
    testPrintI("refFrame width: %u height: %u format: %u %s",
               refWidth, refHeight, refFormat->format,
               hwcTestGraphicFormat2str(refFormat->format));

    sp<GraphicBuffer> equivFrame;
    equivFrame = new GraphicBuffer(equivWidth, equivHeight,
                                   equivFormat->format, texUsage);
    if ((rv = equivFrame->initCheck()) != NO_ERROR) {
        testPrintE("refFrame initCheck failed, rv: %i", rv);
        testPrintE("  width %u height: %u format: %u %s", refWidth, refHeight,
                   equivFormat->format,
                   hwcTestGraphicFormat2str(equivFormat->format));
        exit(10);
    }
    testPrintI("equivFrame width: %u height: %u format: %u %s",
               equivWidth, equivHeight, equivFormat->format,
               hwcTestGraphicFormat2str(equivFormat->format));

    // Fill the frames with a horizontal blend
    hwcTestFillColorHBlend(refFrame.get(), refFormat->format,
                           startRefColor, endRefColor);
    hwcTestFillColorHBlend(equivFrame.get(), refFormat->format,
                           startRefColor, endRefColor);

#ifdef ENABLE_HWC_RECT_TEST
    string rectDesc(""); // String description of a single rectangle

    rectDesc = string("RGBA8888 [50, 80, 200, 300] transform: none color: [1.0, 0.5, 0.5]");

    // Parse string description of rectangle
    Rectangle rect = parseRect(rectDesc);

    // Add to the list of rectangles
    rectangle.push_back(rect);

    rectDesc = string("RGBA8888 [100, 150, 300, 400] blend: coverage color: [0.251, 0.878, 0.816] alpha: 0.7");

    // Parse string description of rectangle
    rect = parseRect(rectDesc);

    // Add to the list of rectangles
    rectangle.push_back(rect);    


#endif
    hwc_layer_list_t *list;

    list = hwcTestCreateLayerList(rectangle.size()+numFrames);
    if (list == NULL) {
        testPrintE("hwcTestCreateLayerList failed");
        exit(5);
    }

    testPrintE("rectangle.size() =%d,numFrames=%d,list->numHwLayers=%d",rectangle.size(),numFrames,list->numHwLayers);    

    list->flags = HWC_GEOMETRY_CHANGED;
    //list->numHwLayers = numFrames;

    hwc_layer_t *layer = &list->hwLayers[0];
    layer->handle = refFrame->handle;
    layer->blending = HWC_BLENDING_NONE;
    layer->sourceCrop.left = 0;
    layer->sourceCrop.top = 0;
    layer->sourceCrop.right = DRAW_FHD_W;
    layer->sourceCrop.bottom = refHeight;
    layer->displayFrame.left = 0;
    layer->displayFrame.top = 0;
    layer->displayFrame.right = DRAW_FHD_W;
    layer->displayFrame.bottom = refHeight;
    layer->visibleRegionScreen.numRects = 1;
    layer->visibleRegionScreen.rects = &layer->displayFrame;
    // *** MediaTek ******************************************************* //
    layer->mtkData.buffer = refFrame;
    layer->mtkData.connectApi = NATIVE_WINDOW_API_CPU;//force to flush cache
    //layer->mtkData.connectApi = mSurfaceTexture->getConnectedApi();

    //if (mBufferDirty || mBufferRefCount <= 1 || contentDirty)
        layer->mtkData.mflags |= HWC_DIRTY_LAYER;
    //else
   //     layer->mtkData.mflags &= ~HWC_DIRTY_LAYER;

    //layer->mtkData.layerFlags = mDrawingState.flags;
    //layer->mtkData.s3dFlags = mDrawingState.s3d_flags;

    layer++;
    layer->handle = equivFrame->handle;
    layer->blending = HWC_BLENDING_NONE;
    layer->sourceCrop.left = 0;
    layer->sourceCrop.top = 0;
    layer->sourceCrop.right = DRAW_FHD_W;
    layer->sourceCrop.bottom = equivHeight;
    layer->displayFrame.left = 0;
    layer->displayFrame.top = refHeight;
    layer->displayFrame.right = DRAW_FHD_W;
    layer->displayFrame.bottom = layer->displayFrame.top + equivHeight;
    layer->visibleRegionScreen.numRects = 1;
    layer->visibleRegionScreen.rects = &layer->displayFrame;
     // *** MediaTek ******************************************************* //
    layer->mtkData.buffer = equivFrame;
    layer->mtkData.connectApi = NATIVE_WINDOW_API_CPU;//force to flush cache
     //layer->mtkData.connectApi = mSurfaceTexture->getConnectedApi();
    
     //if (mBufferDirty || mBufferRefCount <= 1 || contentDirty)
         layer->mtkData.mflags |= HWC_DIRTY_LAYER;
     //else
    //     layer->mtkData.mflags &= ~HWC_DIRTY_LAYER;
    
     //layer->mtkData.layerFlags = mDrawingState.flags;
     //layer->mtkData.s3dFlags = mDrawingState.s3d_flags;
     layer = &list->hwLayers[numFrames];

     for (std::list<Rectangle>::iterator it = rectangle.begin();
          it != rectangle.end(); ++it, ++layer) {
         layer->handle = it->texture->handle;
         layer->blending = it->blend;
         layer->transform = it->transform;
         layer->sourceCrop = it->sourceCrop;
         layer->displayFrame = it->displayFrame;
     
         layer->visibleRegionScreen.numRects = 1;
         layer->visibleRegionScreen.rects = &layer->displayFrame;
         //MTK
         layer->mtkData.buffer = it->texture;
         layer->mtkData.connectApi = NATIVE_WINDOW_API_CPU;//force to flush cache
         
         layer->mtkData.mflags |= HWC_DIRTY_LAYER;  
         layer->mtkData.fillColor.a = 0xff;
     }

    // Perform prepare operation
    if (verbose) { testPrintI("Prepare:"); hwcTestDisplayList(list); }
    if (hwcDevice->common.version >= HWC_DEVICE_API_VERSION_0_3)
        hwcDevice->methods->eventControl(hwcDevice,HWC_EVENT_VSYNC, 1);
    hwcDevice->prepare(hwcDevice, list);
    if (verbose) {
        testPrintI("Post Prepare:");
        hwcTestDisplayListPrepareModifiable(list);
    }

    // Turn off the geometry changed flag
    list->flags &= ~HWC_GEOMETRY_CHANGED;

    if (verbose) {hwcTestDisplayListHandles(list); }
    hwcDevice->set(hwcDevice, dpy, surface, list);
    testDelay(1.0);


    captureScreen(saveGolden, golden_data, cmd_path);
    
    if(!saveGolden)
    {
        bool result = doCheck(0, cmd_path);
        printf("check done\n\n");

        if(true != result) {
            printf("test case fail\n");
        } else {
            printf("test case pass\n");
        }
    }

    testDelay(endDelay);

    // Start framework
    rv = snprintf(cmd, sizeof(cmd), "%s", CMD_START_FRAMEWORK);
    if (rv >= (signed) sizeof(cmd) - 1) {
        testPrintE("Command too long for: %s", CMD_START_FRAMEWORK);
        exit(12);
    }
    testExecCmd(cmd);

    return 0;
}

void init(void)
{
    // Seed pseudo random number generator
    // Seeding causes fill horizontal blend to fill the pad area with
    // a deterministic set of values.
    srand48(0);
    static EGLint width, height;

    hwcTestInitDisplay(verbose, &dpy, &surface, &width, &height);

    hwcTestOpenHwc(&hwcDevice);
}

void printSyntax(const char *cmd)
{
    testPrintE(
    "usage: %s [-g] [FILENAME]\n"
    "   -g: save golden data\n"
    "If FILENAME is not given, the results will be saved to /data/hwcomposer_test_gold.bgra.\n",cmd);

}

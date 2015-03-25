/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2010. All rights reserved.
 *
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER ON
 * AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL WARRANTIES,
 * EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR NONINFRINGEMENT.
 * NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH RESPECT TO THE
 * SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY, INCORPORATED IN, OR
 * SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES TO LOOK ONLY TO SUCH
 * THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO. RECEIVER EXPRESSLY ACKNOWLEDGES
 * THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES
 * CONTAINED IN MEDIATEK SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK
 * SOFTWARE RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S ENTIRE AND
 * CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE RELEASED HEREUNDER WILL BE,
 * AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE MEDIATEK SOFTWARE AT ISSUE,
 * OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE CHARGE PAID BY RECEIVER TO
 * MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek Software")
 * have been modified by MediaTek Inc. All revisions are subject to any receiver's
 * applicable license agreements with MediaTek Inc.
 */

package com.mediatek.stereo3d;

import com.mediatek.xlog.Xlog;

import java.io.ByteArrayOutputStream;
import java.io.File;
import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.IOException;

/**
 * JpsParser is the class that detects 3D content by parsing the stereoscopic data descriptor
 * embedded in the Jps header.
 */
@SuppressWarnings("PMD.GodClass")
public final class JpsParser {
    private static final String TAG = "JpsParser";
    private static final int MARKER_SOI = 0;
    private static final int MARKER_APP3 = 3;
    private static final int MARKER_APPN = 16;
    private static final int HEADER_MIN_LENGTH = 14;
    private static final int HEADER_MAX_LENGTH = 40;

    // type
    /**
     * Jps file with this media type is a monoscopic image.
     * @hide
     */
    public static final int MONOSCOPIC = 0;

    /**
     * Jps file with this media type is a stereoscopic image.
     */
    public static final int STEREOSCOPIC = 1;

    // display for monoscopic
    /**
     * Display the same image in both eyes.
     * @hide
     */
    public static final int S3D_EYE_BOTH = 0;

    /**
     * Display the image in the left eye only.
     * @hide
     */
    public static final int S3D_EYE_LEFT = 1;

    /**
     * Display the image in the right eye only.
     * @hide
     */
    public static final int S3D_EYE_RIGHT = 2;

    // layout for stereoscopic
    /**
     * Image data are in alternating line format.
     */
    public static final int S3D_LAYOUT_INTERLEAVED = 1;

    /**
     * Image data are in side by side format.
     */
    public static final int S3D_LAYOUT_SIDE_BY_SIDE = 2;

    /**
     * Image data are in top and bottom format.
     */
    public static final int S3D_LAYOUT_TOP_AND_BOTTOM = 3;

    /**
     * Image data are in analyph format.
     */
    public static final int S3D_LAYOUT_ANAGLYPH = 4;

    // misc flags
    /**
     * Image data is full height.
     * @hide
     */
    public static final int S3D_FULL_HEIGHT = 0;

    /**
     * Image data is half height.
     * @hide
     */
    public static final int S3D_HALF_HEIGHT = 1;

    /**
     * Image data is full width.
     * @hide
     */
    public static final int S3D_FULL_WIDTH = 0;

    /**
     * Image data is half width.
     * @hide
     */
    public static final int S3D_HALF_WIDTH = 1;

    /**
     * Jps file stores the left and right images with the right view first.
     */
    public static final int S3D_RIGHT_FIELD_FIRST = 0;

    /**
     * Jps file stores the left and right images with the left view first.
     */
    public static final int S3D_LEFT_FIELD_FIRST = 1;

    private int mType = -1;
    private int mDisplay = -1;
    private int mLayout = -1;
    private int mHeightType = -1;
    private int mWidthType = -1;
    private int mFieldOrder = -1;
    private boolean mIsMtk3d = false;

    private JpsParser() {
        // private constructor;
    }

    /**
     * Reads streams of bytes from an image file.
     *
     * @param imgFile The name of the image file
     * @return The byte array
     */
    private byte[] readBytesFromFileHeader(File imgFile) {
        FileInputStream in = null;
        byte[] imgBytes = null;

        try {
            in = new FileInputStream(imgFile);

            if (in != null) {
                ByteArrayOutputStream bout = new ByteArrayOutputStream();
                byte[] buffer = new byte[(int)imgFile.length()];

                try {
                    int readBytes = in.read(buffer);
                    if (readBytes != -1) {
                        bout.write(buffer, 0, readBytes);  // write to ByteArrayOutputStream
                    }
                    imgBytes = bout.toByteArray(); // convert to image byte array
                    bout.close();
                } catch (IOException ex) {
                    Xlog.e(TAG, "IOException: " + ex.toString());
                }

                in.close();
            }
        } catch (FileNotFoundException ex) {
            Xlog.e(TAG, "FileNotFoundException: " + ex.toString());
        } catch (IOException ex) {
            Xlog.e(TAG, "IOException: " + ex.toString());
        }
        return imgBytes;
    }

    /**
     * Extracts the Jps file header metadata, including type, layout, and field order.
     *
     * @param file Image file
     * @return JpsParser object that contains metadata
     */
    public static JpsParser parse(File file) {
        boolean foundApp3 = false;
        int offset = 0;

        JpsParser parser = new JpsParser();

        // read image into byte array
        byte[] imgBytes = parser.readBytesFromFileHeader(file);

        if (imgBytes != null) {
            Xlog.i(TAG, "Parse Jps header with image size: " + imgBytes.length);

            MarkerPair pair = new MarkerPair();

            // find SOI APPn first
            while (offset < imgBytes.length) {
                parser.findStartOfImageMarker(imgBytes, offset, pair);

                if (pair.getMarker() == MARKER_APP3) {
                    // +6 to skip SOI, APP3, and the length info bytes
                    parser.processStereoscopicDescriptor(imgBytes, pair.getOffset() + 6);
                    foundApp3 = true;
                    break;
                } else if (pair.getMarker() == MARKER_APPN) {
                    // +4 to skip SOI, and APPn bytes
                    int length = parser.getBlockLength(imgBytes, pair.getOffset() + 4);
                    offset = pair.getOffset() + 4 + length;
                    break;
                }

                offset += 4;

                if (offset > HEADER_MAX_LENGTH) {
                    Xlog.i(TAG, "Break marker searching");
                    break;
                }
            }

            // if SOI APP3 is not found, try to find SOI APPn...APP3
            if (!foundApp3) {
                while (offset < imgBytes.length) {
                    if (parser.findMarker(imgBytes, offset, MARKER_APP3)) {
                        // +4 to skip APP3, and the length info bytes
                        parser.processStereoscopicDescriptor(imgBytes, offset + 4);
                        break;
                    }

                    offset += 2;

                    if (offset > HEADER_MAX_LENGTH + 10) { // search extra 10 bytes
                        Xlog.i(TAG, "Break marker APP3 searching");
                        break;
                    }
                }
            }
        }

        return parser;
    }

    /**
     * Extracts the first appeared marker, e.g. SOI APP1.
     *
     * @param fileBytes The byte array
     * @param startOffset The starting position to search for the marker
     * @param pair The storage for the result
     */
    private void findStartOfImageMarker(byte[] fileBytes, int startOffset, MarkerPair pair) {
        int offset = startOffset;

        while (offset < fileBytes.length) {
            if (findMarker(fileBytes, offset, MARKER_SOI)) {
                if (findMarker(fileBytes, offset + 2, MARKER_APP3)) {
                    pair.setMarker(MARKER_APP3);
                    pair.setOffset(offset);
                    break;
                } else if (findMarker(fileBytes, offset + 2, MARKER_APPN)) {
                    pair.setMarker(MARKER_APPN);
                    pair.setOffset(offset);
                    break;
                }
            }

            offset += 2;
        }
    }

    /**
     * Gets the length of a block for the marker in the specified offset.
     *
     * @param fileBytes The byte array
     * @param offset The position to extract the length information
     * @return The length of a block
     */
    private int getBlockLength(byte[] fileBytes, int offset) {
        // read two bytes into integer
        return ((int)(fileBytes[offset] & 0xFF) * 256) + (int)(fileBytes[offset + 1] & 0xFF);
    }

    /**
     * Processes the stereoscopic descriptor embedded in the Jps file header.
     *
     * @param fileBytes The byte array of the file
     * @param offset The starting position to search for the stereoscopic descriptor
     */
    private void processStereoscopicDescriptor(byte[] fileBytes, int pos) {
        int offset = pos;
        int stereoLen; // length of the stereoscopic descriptor
        int commentsLen; // length of the comments

        if (offset + HEADER_MIN_LENGTH <= fileBytes.length) {
            if ((fileBytes[offset] == (byte) 0x5F) && (fileBytes[offset + 1] == (byte) 0x4A)
                    && (fileBytes[offset + 2] == (byte) 0x50) && (fileBytes[offset + 3] == (byte) 0x53)
                    && (fileBytes[offset + 4] == (byte) 0x4A) && (fileBytes[offset + 5] == (byte) 0x50)
                    && (fileBytes[offset + 6] == (byte) 0x53) && (fileBytes[offset + 7] == (byte) 0x5F)) {
                // found _JPSJPS_ identifier for a stereoscopic descriptor
                offset += 8;
                // get the length of the stereoscopic descriptor
                // note that the length includes itself (2 bytes), e.g. if length block is 00 06, it means 
                // 2 (for length storage) + 4 bytes (for stereoscopic descriptor storage)
                stereoLen = getBlockLength(fileBytes, offset);
                offset += 2; // skip 2 bytes value that determines the length of a block

                setType(fileBytes[offset++]);

                if (mType == MONOSCOPIC) {
                    setDisplay(fileBytes[offset++]);
                } else if (mType == STEREOSCOPIC) {
                    setLayout(fileBytes[offset++]);
                }

                setMiscFlags(fileBytes[offset]);

                // parse _MTK_AC_ tag to check if a 3D image is generated by MTK N3D (dual cam) or not
                // pos is at the byte 0 of a JPS descriptor, skip 8 bytes for _JPSJPS_
                int commentsOffset = pos + 8 + stereoLen;

                // +2 to check if length block (2bytes) for comments exists or not
                if (commentsOffset + 2 <= fileBytes.length) {
                    // found comments and determine the length for comments
                    commentsLen = getBlockLength(fileBytes, commentsOffset);
                    // skip 2 bytes value that determines the length of the comments block
                    commentsOffset += 2;

                    int i = 0;

                    // 1. iterate times: -10 (-2 for 2 bytes length; -8 for 8 bytes _MTK_AC_)
                    // 2. Make sure commentsOffset does not go beyond the bound
                    while ((i <= commentsLen - 10) && (commentsOffset + 8 <= fileBytes.length)) {
                        if ((fileBytes[commentsOffset] == (byte) 0x5F) && (fileBytes[commentsOffset + 1] == (byte) 0x4D)
                                && (fileBytes[commentsOffset + 2] == (byte) 0x54) && (fileBytes[commentsOffset + 3] == (byte) 0x4B)
                                && (fileBytes[commentsOffset + 4] == (byte) 0x5F) && (fileBytes[commentsOffset + 5] == (byte) 0x41)
                                && (fileBytes[commentsOffset + 6] == (byte) 0x43) && (fileBytes[commentsOffset + 7] == (byte) 0x5F)) {
                            // found _MTK_AC_ tag
                            mIsMtk3d = true;
                            break;
                        }
                        i++;
                        commentsOffset++;
                    }
                }
            }
        }
    }

    /**
     * Reads two bytes at once and determines the type of the marker.
     *
     * @param fileBytes The byte array
     * @param offset The starting position to search for the marker
     * @param type The type of the marker
     * @return True if the specified marker is found, false, otherwise.
     */
    private boolean findMarker(byte[] fileBytes, int offset, int type) {
        boolean result = false;

        switch (type) {
        case MARKER_SOI:
            result = ((fileBytes[offset] == (byte) 0xFF) && (fileBytes[offset + 1] == (byte) 0xD8));
            break;
        case MARKER_APP3:
            result = ((fileBytes[offset] == (byte) 0xFF) && (fileBytes[offset + 1] == (byte) 0xE3));
            break;
        case MARKER_APPN:
            result = ((fileBytes[offset] == (byte) 0xFF) && (fileBytes[offset + 1] >= (byte) 0xE1)
                      && (fileBytes[offset + 1] <= (byte) 0xEF));
            break;
        default:
            break;
        }

        return result;
    }

    /**
     * Sets up the image type according to Jps header.
     *
     * @param b The byte value
     */
    private void setType(byte b) {
        if (b == (byte) 0x00) {
            mType = MONOSCOPIC;
        } else if (b == (byte) 0x01) {
            mType = STEREOSCOPIC;
        }
    }

    /**
     * Sets up the layout type according to Jps header.
     * Layout is only for stereoscopic.
     *
     * @param b The byte value
     */
    private void setLayout(byte b) {
        if (b == (byte) 0x01) {
            mLayout = S3D_LAYOUT_INTERLEAVED;
        } else if (b == (byte) 0x02) {
            mLayout = S3D_LAYOUT_SIDE_BY_SIDE;
        } else if (b == (byte) 0x03) {
            mLayout = S3D_LAYOUT_TOP_AND_BOTTOM;
        } else if (b == (byte) 0x04) {
            mLayout = S3D_LAYOUT_ANAGLYPH;
        }
    }

    /**
     * Sets up the display type according to Jps header.
     * Display is only for monoscopic.
     *
     * @param b The byte value
     */
    private void setDisplay(byte b) {
        if (b == (byte) 0x00) {
            mDisplay = S3D_EYE_BOTH;
        } else if (b == (byte) 0x01) {
            mDisplay = S3D_EYE_LEFT;
        } else if (b == (byte) 0x02) {
            mDisplay = S3D_EYE_RIGHT;
        }
    }

    /**
     * Sets up the image dimension type according to Jps header.
     *
     * @param b The byte value
     */
    private void setMiscFlags(byte b) {
        // height bit
        int bit = 1 & b >> 0;
        if (bit == 0) {
            mHeightType = S3D_FULL_HEIGHT;
        } else {
            mHeightType = S3D_HALF_HEIGHT;
        }

        // width bit
        bit = 1 & b >> 1;
        if (bit == 0) {
            mWidthType = S3D_FULL_WIDTH;
        } else {
            mWidthType = S3D_HALF_WIDTH;
        }

        // field order bit
        bit = 1 & b >> 2;
        if (bit == 0) {
            mFieldOrder = S3D_RIGHT_FIELD_FIRST;
        } else {
            mFieldOrder = S3D_LEFT_FIELD_FIRST;
        }
    }

    /**
     * Retrieves the image type of Jps file, such as stereoscopic type.
     *
     * @return The image type. The type may be STEREOSCOPIC or -1 if there is no type defined
     * in the Jps header.
     */
    public int getType() {
        return mType;
    }

    /**
     * Retrieves the image layout of Jps file, such as side-by-side layout.
     *
     * @return The image layout. The layout may be S3D_LAYOUT_INTERLEAVED, S3D_LAYOUT_SIDE_BY_SIDE,
     * S3D_LAYOUT_TOP_AND_BOTTOM, S3D_LAYOUT_ANAGLYPH, or -1 if there is no layout defined
     * in the Jps header.
     */
    public int getLayout() {
        return mLayout;
    }

    /**
     * Retrieves the display type of Jps file, such as display the image in the left eye only.
     * Display is only for monoscopic.
     *
     * @return The image display. The display may be S3D_EYE_BOTH, S3D_EYE_LEFT, S3D_EYE_RIGHT, or -1
     * if there is no display defined
     * @hide
     */
    public int getDisplay() {
        return mDisplay;
    }

    /**
     * Retrieves the width type of the image.
     *
     * @return The width type
     * @hide
     */
    public int getWidthType() {
        return mWidthType;
    }

    /**
     * Retrieves the height type of the image.
     *
     * @return The height type
     * @hide
     */
    public int getHeightType() {
        return mHeightType;
    }

    /**
     * Retrieves the field order of the image, such as the left view stored first.
     *
     * @return The field order. The order may be S3D_RIGHT_FIELD_FIRST, S3D_LEFT_FIELD_FIRST,
     * or -1 if there is no order defined in the Jps header.
     */
    public int getFieldOrder() {
        return mFieldOrder;
    }

    /**
     * Check if there exists a MTK 3D tag in Jps header.
     *
     * @return True if MTK 3D tag is found (e.g. image is generated by N3D), false, otherwise.
     */
    public boolean isMtk3d() {
        return mIsMtk3d;
    }
}

class MarkerPair {
    private int mMarker;
    private int mOffset;

    MarkerPair() {
        mMarker = -1;
        mOffset = -1;
    }

    /**
     * Sets the marker type
     *
     * @param type The type of the marker
     */
    protected void setMarker(int type) {
        mMarker = type;
    }

    /**
     * Sets the position where the marker is located
     *
     * @param offset The position of the marker
     */
    protected void setOffset(int offset) {
        mOffset = offset;
    }

    /**
     * Gets the type of the marker
     *
     * @return The type of the marker
     */
    protected int getMarker() {
        return mMarker;
    }

    /**
     * Gets the position of the marker
     *
     * @return The position of the marker
     */
    protected int getOffset() {
        return mOffset;
    }
}
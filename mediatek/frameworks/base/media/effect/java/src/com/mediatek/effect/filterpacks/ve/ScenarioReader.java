/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2013. All rights reserved.
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

package com.mediatek.effect.filterpacks.ve;

import java.io.IOException;
import java.io.StringBufferInputStream;
import java.util.Arrays;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;
import javax.xml.parsers.ParserConfigurationException;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;
import org.xml.sax.SAXException;

import com.mediatek.effect.filterpacks.MyUtility;

import android.content.Context;
import android.graphics.Bitmap;

/**
 * @hide
 */
public class ScenarioReader {
    public ScenarioReader() {
    }

    private static long parseLong(String value) {
        return (Long.parseLong(value) * 1000000L); // change ms to ns
    }

    public static VideoScenario getScenario(Context context, String xml) {
        return getScenario(context, xml, null, null);
    }

    public static VideoScenario getScenario(Context context, String xml, Object object1, Object object2) {
        VideoScenario mScenario = new VideoScenario();
        long theEndTime = 0L;

        mScenario.put("xml", xml.trim());

        boolean isObjTheSame = false;
        if (object1 == object2) {
            isObjTheSame = true;
        }

        DocumentBuilderFactory docBuilderFactory = DocumentBuilderFactory.newInstance();
        DocumentBuilder docBuilder;
        try {
            StringBufferInputStream myXML = new StringBufferInputStream(xml);

            docBuilder = docBuilderFactory.newDocumentBuilder();
            Document doc = docBuilder.parse(myXML);

            doc.getDocumentElement().normalize();
            System.out.println("Root element " + doc.getDocumentElement().getNodeName());

            NodeList sizeLst = doc.getElementsByTagName("size");
            Element sizeElement = (Element) sizeLst.item(0);

            int orientation = 0;

            if (null != sizeElement) {
                System.out.println("size:" + sizeElement.getAttribute("owidth") + "x"
                    + sizeElement.getAttribute("oheight"));
                System.out.println("orientation:" + sizeElement.getAttribute("orientation"));

                mScenario.put("video_ow", Integer.parseInt(sizeElement.getAttribute("owidth").toString()));
                mScenario.put("video_oh", Integer.parseInt(sizeElement.getAttribute("oheight").toString()));

                try {
                    orientation = Integer.parseInt(sizeElement.getAttribute("orientation").toString());
                } catch(NumberFormatException e) {
                    orientation = 0;
                }
                mScenario.put("orientation", orientation);
            } else {
                mScenario.clear();
                mScenario = null;
                return mScenario;
            }

            NodeList videoLst = doc.getElementsByTagName("video");
            Element videoElement = (Element) videoLst.item(0);
            if (null != videoElement) {
                System.out.println("video:" + videoElement.getTextContent());
                mScenario.put("video1", videoElement.getTextContent());

                videoElement = (Element) videoLst.item(1);
                if (null != videoElement) {
                    System.out.println("video:" + videoElement.getTextContent());
                    mScenario.put("video2", videoElement.getTextContent());
                    mScenario.put("video2_init_offset", videoElement.getAttribute("init_offset"));
                }
            } else {
                mScenario.clear();
                mScenario = null;
                return mScenario;
            }

            NodeList edgeLst = doc.getElementsByTagName("edge");
            Element edgeElement = (Element) edgeLst.item(0);
            if (null != edgeElement) {
                System.out.println("edge:" + edgeElement.getTextContent());
                mScenario.put("edge", edgeElement.getTextContent());
            } else {
                mScenario.clear();
                mScenario = null;
                return mScenario;
            }

            NodeList outputvideoLst = doc.getElementsByTagName("outputvideo");
            Element outputvideoElement = (Element) outputvideoLst.item(0);
            if (null != outputvideoElement) {
                System.out.println("outputvideo:" + outputvideoElement.getTextContent() +
                    " fps:" + outputvideoElement.getAttribute("fps") +
                    " bitrate:" + outputvideoElement.getAttribute("bitrate"));
                mScenario.put("outputvideo", outputvideoElement.getTextContent());
                mScenario.put("outputvideo_fps", outputvideoElement.getAttribute("fps"));
                mScenario.put("outputvideo_bitrate", outputvideoElement.getAttribute("bitrate"));
                mScenario.put("livephoto", outputvideoElement.getAttribute("livephoto"));

                String truncate = outputvideoElement.getAttribute("truncate");

                if (truncate.length() > 0) {

                    mScenario.put("truncate", true);

                    Bitmap tmp1;
                    Bitmap tmp2;

                    if (object1 instanceof Bitmap) {
                        tmp1 = MyUtility.getCutBitmap((Bitmap) object1,
                            Integer.parseInt(mScenario.get("video_ow") + ""),
                            Integer.parseInt(mScenario.get("video_oh") + ""), true);
                        object1 = tmp1;
                    }

                    if (isObjTheSame == true) {
                        object2 = object1;
                    } else if (object2 instanceof Bitmap) {
                        tmp2 = MyUtility.getCutBitmap((Bitmap) object2,
                            Integer.parseInt(mScenario.get("video_ow") + ""),
                            Integer.parseInt(mScenario.get("video_oh") + ""), true);
                        object2 = tmp2;
                    }
                }
            }

            /*if (orientation != 0) {
                int orient = 0;
                if (object1 instanceof Bitmap && orient != 0) {
                    object1 = MyUtility.generateRotateImage((Bitmap) object1, orient, true);
                }

                if (isObjTheSame == true) {
                    object2 = object1;
                } else if (object2 instanceof Bitmap && orient != 0) {
                    object1 = MyUtility.generateRotateImage((Bitmap) object2, orient, true);
                }
            }*/

            NodeList nodeLst = doc.getElementsByTagName("videoevent");
            System.out.println("Information of all Video-Event");

            VideoEvent ve;
            theEndTime = 0L;
            for (int s = 0; s < nodeLst.getLength(); s++) {
                Node fstNode = nodeLst.item(s);
                if (fstNode.getNodeType() == Node.ELEMENT_NODE) {
                    Element veElement = (Element) fstNode;

                    if (null != veElement) {
                        System.out.println("type:" + veElement.getAttribute("type") + ", name:" + veElement.getAttribute("name"));
                        System.out.println("  time:" + veElement.getAttribute("start") + "~" + veElement.getAttribute("end"));

                        NodeList showTimeElementList = veElement.getElementsByTagName("showtime");
                        NodeList thumbnailElementList = veElement.getElementsByTagName("thumbnail");
                        NodeList backgroundElementList = veElement.getElementsByTagName("background");
                        Element showTimeElement = (Element) showTimeElementList.item(0);
                        Element thumbnailElement = (Element) thumbnailElementList.item(0);
                        Element backgroundElement = (Element) backgroundElementList.item(0);

                        if (null != showTimeElement)
                            System.out.println("  showtime:" + showTimeElement.getAttribute("related_start") + "~ (" + showTimeElement.getAttribute("length") + ")");

                        if (null != thumbnailElement)
                            System.out.println("  thumbnail:" +
                                thumbnailElement.getTextContent() + " m:" +
                                thumbnailElement.getAttribute("move"));

                        if (null != backgroundElement)
                            System.out.println("  background:" +
                                backgroundElement.getTextContent() + " still:" +
                                backgroundElement.getAttribute("still") + " fade_in:" +
                                backgroundElement.getAttribute("fade_in"));

                        ve = null;
                        if ("still".equalsIgnoreCase(veElement.getAttribute("type"))) {
                            ve = new VideoEventStill(veElement.getAttribute("name"),
                                parseLong(veElement.getAttribute("start")),
                                parseLong(veElement.getAttribute("end")));
                        } else if ("overlay".equalsIgnoreCase(veElement.getAttribute("type"))) {
                            ve = new VideoEventOverlay(veElement.getAttribute("name"),
                                parseLong(veElement.getAttribute("start")),
                                parseLong(veElement.getAttribute("end")));
                                ve.put("scale", thumbnailElement.getAttribute("scale"));
                                ve.put("x", thumbnailElement.getAttribute("x"));
                                ve.put("y", thumbnailElement.getAttribute("y"));
                        } else if ("camera".equalsIgnoreCase(veElement.getAttribute("type"))) {
                            NodeList cameraElementList = veElement.getElementsByTagName("camera_photo");
                            Element cameraElement = (Element) cameraElementList.item(0);
                            if (cameraElement != null) {
                                System.out.println("  camera_photo: " + cameraElement.getAttribute("related_start"));
                                ve = new VideoEventCamera(veElement.getAttribute("name"),
                                    parseLong(veElement.getAttribute("start")),
                                    parseLong(veElement.getAttribute("end")));
                                ve.put("camera_photo", parseLong(cameraElement.getAttribute("related_start")));
                            }
                        }

                        if (null != ve) {
                            if (null != showTimeElement) {
                                ve.setDurationEffectRelatedTime(
                                    parseLong(showTimeElement.getAttribute("related_start")),
                                    parseLong(showTimeElement.getAttribute("length")));
                            }

                            if (null != thumbnailElement) {
                                Object myBitmap = thumbnailElement.getTextContent();
                                if (myBitmap instanceof String) {
                                    if ("object1".equalsIgnoreCase((String) myBitmap)) {
                                        myBitmap = object1;
                                    } else if ("object2".equalsIgnoreCase((String) myBitmap)) {
                                        myBitmap = object2;
                                    }
                                }
                                ve.putThumbnail(context.getResources(),
                                    myBitmap,
                                    thumbnailElement.getAttribute("move"),
                                    thumbnailElement.getAttribute("fade_out"));
                            }

                            if (null != backgroundElement) {
                                Object myBitmap = backgroundElement.getTextContent();
                                if (myBitmap instanceof String) {
                                    if ("object1".equalsIgnoreCase((String) myBitmap)) {
                                        myBitmap = object1;
                                    } else if ("object2".equalsIgnoreCase((String) myBitmap)) {
                                        myBitmap = object2;
                                    }
                                }
                                ve.putBackground(context.getResources(),
                                    myBitmap,
                                    backgroundElement.getAttribute("still"),
                                    backgroundElement.getAttribute("fade_in"),
                                    backgroundElement.getAttribute("init_offset"));
                            }

                            if (null != edgeElement) {
                                Object myBitmap = edgeElement.getTextContent();
                                if (myBitmap instanceof String) {
                                    if ("object1".equalsIgnoreCase((String) myBitmap)) {
                                        myBitmap = object1;
                                    } else if ("object2".equalsIgnoreCase((String) myBitmap)) {
                                        myBitmap = object2;
                                    }
                                }
                                ve.putEdge(context.getResources(), myBitmap);
                            }

                            mScenario.put(ve.getKey(), ve);

                            if( theEndTime < ve.getEndTime()) {
                                theEndTime = ve.getEndTime();
                            }
                        }
                    }

                }
            }
        } catch (IOException e) {
            mScenario.clear();
            mScenario = null;
            e.printStackTrace();
        } catch (SAXException e) {
            mScenario.clear();
            mScenario = null;
            e.printStackTrace();
        }  catch (ParserConfigurationException e) {
            mScenario.clear();
            mScenario = null;
            e.printStackTrace();
        } catch (NullPointerException e) {
            mScenario.clear();
            mScenario = null;
            e.printStackTrace();
        }

        if (null != mScenario)
            mScenario.put("THEENDTIME", theEndTime);

        tmp = dump(mScenario);

        System.out.print(tmp);

        return mScenario;
    }

    public static String tmp;

    public static String dump(VideoScenario scenario) {
        String result = "";
        if (scenario != null) {
            Object[] mKeys = scenario.keySet().toArray();
            Arrays.sort(mKeys);
            result = "Total: " + mKeys.length + "\n";
            for (Object key : mKeys) {
                Object obj = scenario.get(key);
                result += key + ": " + obj + "\n";
            }
        }
        return result;
    }
}



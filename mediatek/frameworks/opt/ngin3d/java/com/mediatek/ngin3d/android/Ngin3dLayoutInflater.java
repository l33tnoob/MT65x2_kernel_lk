/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein is
 * confidential and proprietary to MediaTek Inc. and/or its licensors. Without
 * the prior written permission of MediaTek inc. and/or its licensors, any
 * reproduction, modification, use or disclosure of MediaTek Software, and
 * information contained herein, in whole or in part, shall be strictly
 * prohibited.
 * 
 * MediaTek Inc. (C) 2010. All rights reserved.
 * 
 * BY OPENING THIS FILE, RECEIVER HEREBY UNEQUIVOCALLY ACKNOWLEDGES AND AGREES
 * THAT THE SOFTWARE/FIRMWARE AND ITS DOCUMENTATIONS ("MEDIATEK SOFTWARE")
 * RECEIVED FROM MEDIATEK AND/OR ITS REPRESENTATIVES ARE PROVIDED TO RECEIVER
 * ON AN "AS-IS" BASIS ONLY. MEDIATEK EXPRESSLY DISCLAIMS ANY AND ALL
 * WARRANTIES, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE IMPLIED
 * WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE OR
 * NONINFRINGEMENT. NEITHER DOES MEDIATEK PROVIDE ANY WARRANTY WHATSOEVER WITH
 * RESPECT TO THE SOFTWARE OF ANY THIRD PARTY WHICH MAY BE USED BY,
 * INCORPORATED IN, OR SUPPLIED WITH THE MEDIATEK SOFTWARE, AND RECEIVER AGREES
 * TO LOOK ONLY TO SUCH THIRD PARTY FOR ANY WARRANTY CLAIM RELATING THERETO.
 * RECEIVER EXPRESSLY ACKNOWLEDGES THAT IT IS RECEIVER'S SOLE RESPONSIBILITY TO
 * OBTAIN FROM ANY THIRD PARTY ALL PROPER LICENSES CONTAINED IN MEDIATEK
 * SOFTWARE. MEDIATEK SHALL ALSO NOT BE RESPONSIBLE FOR ANY MEDIATEK SOFTWARE
 * RELEASES MADE TO RECEIVER'S SPECIFICATION OR TO CONFORM TO A PARTICULAR
 * STANDARD OR OPEN FORUM. RECEIVER'S SOLE AND EXCLUSIVE REMEDY AND MEDIATEK'S
 * ENTIRE AND CUMULATIVE LIABILITY WITH RESPECT TO THE MEDIATEK SOFTWARE
 * RELEASED HEREUNDER WILL BE, AT MEDIATEK'S OPTION, TO REVISE OR REPLACE THE
 * MEDIATEK SOFTWARE AT ISSUE, OR REFUND ANY SOFTWARE LICENSE FEES OR SERVICE
 * CHARGE PAID BY RECEIVER TO MEDIATEK FOR SUCH MEDIATEK SOFTWARE AT ISSUE.
 *
 * The following software/firmware and/or related documentation ("MediaTek
 * Software") have been modified by MediaTek Inc. All revisions are subject to
 * any receiver's applicable license agreements with MediaTek Inc.
 */

package com.mediatek.ngin3d.android;

import android.content.Context;
import android.content.res.TypedArray;
import android.content.res.XmlResourceParser;
import android.graphics.Typeface;
import android.util.Log;
import com.mediatek.ngin3d.Actor;
import com.mediatek.ngin3d.BitmapFont;
import com.mediatek.ngin3d.BitmapText;
import com.mediatek.ngin3d.Color;
import com.mediatek.ngin3d.Container;
import com.mediatek.ngin3d.Dimension;
import com.mediatek.ngin3d.Image;
import com.mediatek.ngin3d.Plane;
import com.mediatek.ngin3d.Point;
import com.mediatek.ngin3d.Scale;
import com.mediatek.ngin3d.Sphere;
import com.mediatek.ngin3d.Stage;
import com.mediatek.ngin3d.Text;
import org.xmlpull.v1.XmlPullParser;
import org.xmlpull.v1.XmlPullParserException;

import java.io.IOException;
import java.util.Stack;

public final class Ngin3dLayoutInflater {

    public static final boolean DEBUG = true;
    private static final String TAG = "Ngin3dLayoutInflater";

    /**
     * The method to print log
     *
     * @param tag the tag of the class
     * @param msg the log message to print
     */
    private static void log(String tag, String msg) {
        if (Ngin3dLayoutInflater.DEBUG) {
            Log.d(tag, msg);
        }
    }

    /* xml Tags */
    private static final String XML_TAG_STAGE = "Stage";
    private static final String XML_TAG_TEXT = "Text";
    private static final String XML_TAG_IMAGE = "Image";
    private static final String XML_TAG_SPHERE = "Sphere";
    private static final String XML_TAG_BITMAPTEXT = "BitmapText";
    private static final String XML_TAG_CONTAINER = "Container";

    /* text Style */
    private static final int TEXT_STYLE_NORMAL = 0;
    private static final int TEXT_STYLE_BOLD = 1;
    private static final int TEXT_STYLE_ITALIC = 2;
    private static final int TEXT_STYLE_BOLDITALIC = 3;

    /* text typeface */
    private static final int TEXT_TYPEFACE_NORMAL = 0;
    private static final int TEXT_TYPEFACE_SANS = 1;
    private static final int TEXT_TYPEFACE_SERIF = 2;
    private static final int TEXT_TYPEFACE_MONOSPACE = 3;
    private static StyleableResolver sStyleable;

    private Ngin3dLayoutInflater() {
        // do nothing.
    }

    /**
     * inflate 3d layout from the given xml file
     *
     * @param context       : Application context used to access resources
     * @param xmlResId      : The resource id of the xml file
     * @param rootContainer : the container that caller wants to add all actors in the xml into
     * @return The root container/actor which is described in the xml
     * @throws IllegalArgumentException or RuntimeException while error occurs
     */
    public static Actor inflateLayout(Context context, int xmlResId, Container rootContainer) {

        XmlResourceParser parser = context.getResources().getXml(xmlResId);
        Actor xmlRootContainer = null;

        // Must create Styleable Resolver before inflating
        sStyleable = new StyleableResolver(context);

        /* use mContainerList to support/check nested container in xml */
        final Stack<Container> containerList = new Stack<Container>();

        try {
            int xmlEventType;

            while ((xmlEventType = parser.next()) != XmlResourceParser.END_DOCUMENT) {
                switch (xmlEventType) {
                case XmlPullParser.START_DOCUMENT:
                    log(TAG, "Start document");
                    break;
                case XmlPullParser.START_TAG:
                    String tag = parser.getName();
                    log(TAG, "Start Tag: " + tag);

                    Actor actor = newActorByTag(tag, context, parser, rootContainer);

                    if (xmlRootContainer == null) {
                        xmlRootContainer = actor;
                        log(TAG, "xmlRootContainer:  " + xmlRootContainer);

                        if (rootContainer != null) {
                            rootContainer.add(xmlRootContainer);
                            log(TAG, "add xmlRootContainer into rootContainer.");
                        }
                    } else {
                        if (actor instanceof Stage) {
                            throw new IllegalArgumentException("Stage should be the root container.");
                        }

                        /* handle nested container xml */
                        /* since BitmapText extends container, but it is not allowed to add actor into. */
                        if ((xmlRootContainer instanceof Container) && !(xmlRootContainer instanceof BitmapText)) {

                            /* find a proper container to add actor into */
                            if (containerList.isEmpty()) {
                                ((Container) xmlRootContainer).add(actor);
                                log(TAG, "no proper sub container. add actor into xmlRootContainer:  " + actor);
                            } else {
                                log(TAG, "containerList size :  " + containerList.size());
                                Container properContainer = containerList.peek();
                                properContainer.add(actor);
                                log(TAG, "add actor into proper sub container.  actor:  " + actor + ", properContainer: " + properContainer);
                            }

                            /* since BitmapText extends container, but it is not allowed to add actor into. */
                            if ((actor instanceof Container) && !(actor instanceof BitmapText)) {
                                /* this actor is subContainer in xml file */
                                containerList.push((Container) actor);
                                log(TAG, "actor is a Container, add into ContainerList.");
                            }
                        } else {
                            throw new IllegalArgumentException("xmlRootContainer is not a container, can't add actor into it.");
                        }
                    }
                    break;
                case XmlPullParser.END_TAG:
                    log(TAG, "End Tag: " + parser.getName());

                    if (parser.getName().equalsIgnoreCase(XML_TAG_CONTAINER)) {
                        /* remove the latest container */
                        log(TAG, "End Tag -- containerList size :  " + containerList.size());
                        if (!containerList.isEmpty()) {
                            containerList.pop();
                        }
                    }
                    break;
                case XmlPullParser.TEXT:
                    log(TAG, "Text: " + parser.getText());
                    break;
                default:
                    break;
                }
            }
        } catch (XmlPullParserException e) {
            Log.e(TAG, "Parser Error: " + e.getMessage());
        } catch (IOException e) {
            Log.e(TAG, "IOException occurs. Unable to read resource file");
        }

        return xmlRootContainer;
    }

    /**
     * new actor by tag
     *
     * @param tag            : the tag string in xml file
     * @param context        : the application environment
     * @param parser:        the xml resource parser created from the xml file
     * @param rootContainer: the container that caller wants to add all actors in the xml into
     * @return the Actor object
     */
    private static Actor newActorByTag(String tag, Context context, XmlResourceParser parser, Container rootContainer) {

        Actor actor = null;

        if (tag.equalsIgnoreCase(XML_TAG_STAGE)) {
            /* Stage */
            if (rootContainer != null) {
                throw new IllegalArgumentException("Stage should be the root container.");
            }
            actor = parseStage(context, parser);
        } else if (tag.equalsIgnoreCase(XML_TAG_TEXT)) {
            /* Text */
            actor = parseText(context, parser);

        } else if (tag.equalsIgnoreCase(XML_TAG_IMAGE)) {
            /* Image */
            actor = parseImage(context, parser);

        } else if (tag.equalsIgnoreCase(XML_TAG_CONTAINER)) {
            /* Container */
            actor = parseContainer(context, parser);

        } else if (tag.equalsIgnoreCase(XML_TAG_SPHERE)) {
            /* Sphere */
            actor = parseSphere(context, parser);

        } else if (tag.equalsIgnoreCase(XML_TAG_BITMAPTEXT)) {
            /* BitmapText */
            actor = parseBitmapText(context, parser);

        } else {
            // TODO : unknow tag , it might be a custom Actor. 
            // How to support custom Actor
            throw new IllegalArgumentException("unknown Actor tag in xml file.");
        }
        return actor;
    }

    /**
     * parse attribute of stage in the xml parser
     *
     * @param context : the application environment
     * @param parser: the xml resource parser created from the xml file
     * @return the stage object
     */
    private static Stage parseStage(Context context, XmlResourceParser parser) {
        /* new Stage */
        Stage stage = new Stage(AndroidUiHandler.create());

        /* get attribute array */
        TypedArray ta = context.obtainStyledAttributes(parser, sStyleable.resolveIntArray("Stage"));

        /* set StageBgColor */
        int stageBgColor = ta.getColor(sStyleable.resolveInt("Stage_backgroundColor"), -1);
        log(TAG, "Stage stageBgColor : " + stageBgColor);
        if (stageBgColor != -1) {
            stage.setBackgroundColor(new Color(stageBgColor));
        }

        ta.recycle();
        return stage;
    }

    /**
     * parse attribute of text in the xml parser
     *
     * @param context : the application environment
     * @param parser: the xml resource parser created from the xml file
     * @return the text object
     */
    private static Text parseText(Context context, XmlResourceParser parser) {
        /* get the android res id */
        TypedArray a = context.obtainStyledAttributes(parser, com.android.internal.R.styleable.View);
        int tagId = a.getResourceId(com.android.internal.R.styleable.View_id, -1);
        log(TAG, "Text tagId : " + tagId);
        a.recycle();

        /* new Text */
        TypedArray ta = context.obtainStyledAttributes(parser, sStyleable.resolveIntArray("Text"));
        String textString = ta.getString(sStyleable.resolveInt("Text_text"));
        if (textString == null) {
            ta.recycle();
            throw new IllegalArgumentException("text string is null, create text failed!");
        }

        Text text = new Text(textString);
        text.setTag(tagId);

        /* set Text color */
        int textColor = ta.getColor(sStyleable.resolveInt("Text_textColor"), -1);
        if (textColor != -1) {
            log(TAG, "Text textColor : " + textColor);
            text.setTextColor(new Color(textColor));
        }

        /* set Text size */
        float textSize = ta.getDimension(sStyleable.resolveInt("Text_textSize"), -1);
        if (textSize != -1) {
            log(TAG, "Text textSize : " + textSize);
            text.setTextSize(textSize);
        }

        /* set Text style & set Text typeface */
        int textStyle = ta.getInt(sStyleable.resolveInt("Text_textStyle"), -1);
        if (textStyle == -1) {
            textStyle = Typeface.NORMAL;
        } else {
            log(TAG, "Text textStyle : " + textStyle);
            switch (textStyle) {
            case TEXT_STYLE_NORMAL:
                textStyle = Typeface.NORMAL;
                break;
            case TEXT_STYLE_BOLD:
                textStyle = Typeface.BOLD;
                break;
            case TEXT_STYLE_ITALIC:
                textStyle = Typeface.ITALIC;
                break;
            case TEXT_STYLE_BOLDITALIC:
                textStyle = Typeface.BOLD_ITALIC;
                break;
            default:
                textStyle = Typeface.NORMAL;
                break;
            }
        }

        int textTypeface = ta.getInt(sStyleable.resolveInt("Text_textTypeface"), -1);
        Typeface tf = null;
        if (textTypeface == -1) {
            tf = Typeface.create(Typeface.DEFAULT, textStyle);
        } else {
            log(TAG, "Text textTypeface : " + textTypeface);
            switch (textTypeface) {
            case TEXT_TYPEFACE_NORMAL:
                tf = Typeface.create(Typeface.DEFAULT, textStyle);
                break;
            case TEXT_TYPEFACE_SANS:
                tf = Typeface.create(Typeface.SANS_SERIF, textStyle);
                break;
            case TEXT_TYPEFACE_SERIF:
                tf = Typeface.create(Typeface.SERIF, textStyle);
                break;
            case TEXT_TYPEFACE_MONOSPACE:
                tf = Typeface.create(Typeface.MONOSPACE, textStyle);
                break;
            default:
                tf = Typeface.create(Typeface.DEFAULT, textStyle);
                break;
            }
        }
        text.setTypeface(tf);

        /* set common actor attributes : scale, visible, position, anchorPoint */
        float textScale = ta.getFloat(sStyleable.resolveInt("Text_scale"), -1);
        boolean textVisible = ta.getBoolean(sStyleable.resolveInt("Text_visible"), true);
        String positionString = ta.getString(sStyleable.resolveInt("Text_position"));
        String anchorString = ta.getString(sStyleable.resolveInt("Text_anchorPoint"));
        setActorCommonAttribute(text, textScale, textVisible, positionString, anchorString);

        ta.recycle();
        return text;
    }

    /**
     * parse attribute of image in the xml parser
     *
     * @param context : the application environment
     * @param parser: the xml resource parser created from the xml file
     * @return the image object
     */
    private static Image parseImage(Context context, XmlResourceParser parser) {
        /* get the android res id */
        TypedArray a = context.obtainStyledAttributes(parser, com.android.internal.R.styleable.View);
        int tagId = a.getResourceId(com.android.internal.R.styleable.View_id, -1);
        log(TAG, "Image  tagId : " + tagId);
        a.recycle();

        /* new image */
        TypedArray ta = context.obtainStyledAttributes(parser, sStyleable.resolveIntArray("Image"));
        int resId = ta.getResourceId(sStyleable.resolveInt("Image_src"), -1);
        log(TAG, "Image  resId : " + resId);
        if (resId == -1) {
            ta.recycle();
            throw new IllegalArgumentException("image resource is null, create Image failed! ");
        }

        Image image = Image.createFromResource(context.getResources(), resId);
        image.setTag(tagId);

        /* set image size */
        float width = ta.getDimension(sStyleable.resolveInt("Image_width"), -1);
        float height = ta.getDimension(sStyleable.resolveInt("Image_height"), -1);
        if (width != -1 && height != -1) {
            log(TAG, "Image  width : " + width + " , Image height :" + height);
            image.setSize(new Dimension(width, height));
        }

        /* set common actor attributes : scale, visible, position, anchorPoint */
        float scale = ta.getFloat(sStyleable.resolveInt("Image_scale"), -1);
        boolean visible = ta.getBoolean(sStyleable.resolveInt("Image_visible"), true);
        String positionString = ta.getString(sStyleable.resolveInt("Image_position"));
        String anchorString = ta.getString(sStyleable.resolveInt("Image_anchorPoint"));
        setActorCommonAttribute(image, scale, visible, positionString, anchorString);

        ta.recycle();
        return image;
    }

    /**
     * parse attribute of shpere in the xml parser
     *
     * @param context : the application environment
     * @param parser: the xml resource parser created from the xml file
     * @return the Sphere object
     */
    private static Sphere parseSphere(Context context, XmlResourceParser parser) {
        /* get the android res id */
        TypedArray a = context.obtainStyledAttributes(parser, com.android.internal.R.styleable.View);
        int tagId = a.getResourceId(com.android.internal.R.styleable.View_id, -1);
        log(TAG, "Sphere  tagId : " + tagId);
        a.recycle();

        /* new Sphere */
        TypedArray ta = context.obtainStyledAttributes(parser, sStyleable.resolveIntArray("Sphere"));
        int resId = ta.getResourceId(sStyleable.resolveInt("Sphere_src"), -1);
        log(TAG, "Sphere  resId : " + resId);
        if (resId == -1) {
            ta.recycle();
            throw new IllegalArgumentException("image resource is null, create Sphere failed!");
        }

        Sphere sphere = Sphere.createFromResource(context.getResources(), resId);
        sphere.setTag(tagId);

        /* set common actor attributes : scale, visible, position, anchorPoint */
        float scale = ta.getFloat(sStyleable.resolveInt("Sphere_scale"), -1);
        boolean visible = ta.getBoolean(sStyleable.resolveInt("Sphere_visible"), true);
        String positionString = ta.getString(sStyleable.resolveInt("Sphere_position"));
        String anchorString = ta.getString(sStyleable.resolveInt("Sphere_anchorPoint"));
        setActorCommonAttribute(sphere, scale, visible, positionString, anchorString);

        ta.recycle();
        return sphere;
    }

    /**
     * parse attribute of BitmapText in the xml parser
     *
     * @param context : the application environment
     * @param parser: the xml resource parser created from the xml file
     * @return the BitmapText object
     */
    private static BitmapText parseBitmapText(Context context, XmlResourceParser parser) {
        /* get the android res id */
        TypedArray a = context.obtainStyledAttributes(parser, com.android.internal.R.styleable.View);
        int tagId = a.getResourceId(com.android.internal.R.styleable.View_id, -1);
        log(TAG, "BitmapText  tagId : " + tagId);
        a.recycle();

        /* get Bitmap text */
        TypedArray ta = context.obtainStyledAttributes(parser, sStyleable.resolveIntArray("BitmapText"));
        String text = ta.getString(sStyleable.resolveInt("BitmapText_text"));
        log(TAG, "BitmapText  text : " + text);
        if (text == null) {
            ta.recycle();
            throw new IllegalArgumentException("invalid/null text of BitmapText is not allowed!");
        }

        /* get Bitmap font info & new BitmapText */
        int fontResId = ta.getResourceId(sStyleable.resolveInt("BitmapText_fontSrc"), -1);
        int fontDescriptionResId = ta.getResourceId(sStyleable.resolveInt("BitmapText_fontDescriptionSrc"), -1);
        log(TAG, "BitmapText  fontResId : " + fontResId + ", fontDescriptionResId :" + fontDescriptionResId);
        BitmapText bitmapText;
        if (fontResId == -1 || fontDescriptionResId == -1) {
            bitmapText = new BitmapText(text);
        } else {
            bitmapText = new BitmapText(text, new BitmapFont(context.getResources(), fontDescriptionResId, fontResId));
        }
        bitmapText.setTag(tagId);

        /* set common actor attributes : scale, visible, position, anchorPoint */
        float scale = ta.getFloat(sStyleable.resolveInt("BitmapText_scale"), -1);
        boolean visible = ta.getBoolean(sStyleable.resolveInt("BitmapText_visible"), true);
        String positionString = ta.getString(sStyleable.resolveInt("BitmapText_position"));
        String anchorString = ta.getString(sStyleable.resolveInt("BitmapText_anchorPoint"));
        setActorCommonAttribute(bitmapText, scale, visible, positionString, anchorString);

        ta.recycle();
        return bitmapText;
    }

    /**
     * parse attribute of Container in the xml parser
     *
     * @param context : the application environment
     * @param parser: the xml resource parser created from the xml file
     * @return the Container object
     */
    private static Container parseContainer(Context context, XmlResourceParser parser) {
        /* get the android res id */
        TypedArray a = context.obtainStyledAttributes(parser, com.android.internal.R.styleable.View);
        int tagId = a.getResourceId(com.android.internal.R.styleable.View_id, -1);
        log(TAG, "Container  tagId : " + tagId);
        a.recycle();

        /* get Bitmap text */
        TypedArray ta = context.obtainStyledAttributes(parser, sStyleable.resolveIntArray("Container"));
        Container container = new Container();
        container.setTag(tagId);

        /* set common actor attributes : scale, visible, position, anchorPoint */
        float scale = ta.getFloat(sStyleable.resolveInt("Container_scale"), -1);
        boolean visible = ta.getBoolean(sStyleable.resolveInt("Container_visible"), true);
        String positionString = ta.getString(sStyleable.resolveInt("Container_position"));
        String anchorString = ta.getString(sStyleable.resolveInt("Container_anchorPoint"));
        setActorCommonAttribute(container, scale, visible, positionString, anchorString);

        ta.recycle();
        return container;
    }

    /**
     * parse attribute of Container in the xml parser
     *
     * @param actor           : the target actor
     * @param scale:          scale value
     * @param visible:        visible or not
     * @param positionString: position in string
     * @param anchorString:   anchor point in string
     */
    private static void setActorCommonAttribute(Actor actor, float scale, boolean visible, String positionString, String anchorString) {
        /* set scale */
        if (scale != -1) {
            log(TAG, "scale: " + scale);
            actor.setScale(new Scale(scale, scale, scale));
        }

        /* set visible */
        log(TAG, "Text  visible: " + visible);
        actor.setVisible(visible);

        /* set position */
        if (positionString != null) {
            actor.setPosition(Point.newFromString(positionString));
        }

        /* set anchorPoint */
        if (anchorString != null) {
            if (actor instanceof Plane) {
                ((Plane)actor).setAnchorPoint(Point.newFromString(anchorString));
            } else if (actor instanceof BitmapText) {
                ((BitmapText)actor).setAnchorPoint(Point.newFromString(anchorString));
            }

        }
    }

}

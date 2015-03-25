/* Copyright Statement:
 *
 * This software/firmware and related documentation ("MediaTek Software") are
 * protected under relevant copyright laws. The information contained herein
 * is confidential and proprietary to MediaTek Inc. and/or its licensors.
 * Without the prior written permission of MediaTek inc. and/or its licensors,
 * any reproduction, modification, use or disclosure of MediaTek Software,
 * and information contained herein, in whole or in part, shall be strictly prohibited.
 *
 * MediaTek Inc. (C) 2012. All rights reserved.
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

package com.orangelabs.rcs.core.ims.service.presence.simplefilter;

import android.text.TextUtils;

import com.orangelabs.rcs.utils.XMLUtils.ElementInfo;
import com.orangelabs.rcs.utils.logger.Logger;

import java.util.List;

/**
 * SimpleFilterDocument provides methods ,which used to create elements in XML
 * document ,for creating simple-filter.xml. The user who uses this class to
 * create simple-filter.xml must use methods to create elements needed,and
 * indicate the relationships among elements by setting elements'
 * childElements,then use SAXUtils to create document.This class refers RFC4661
 */
public class SimpleFilterDocument {
    /**
     * Class name tag.
     */
    private static final String TAG = "SimpleFilterDocument";

    // The logger
    private static final Logger logger = Logger.getLogger(TAG);

    /**
     * The Simple-Filter document selector, it's the URI of an initial XML
     * document that to be patched.
     */
    private String mInitialXMLDocumentSelector;
    private static final String EMPTY_STRING = "";
    // value of the XMLNS attribute of the <filer-set> element
    private static final String XMLNS_VALUE = "urn:ietf:params:xml:ns:simple-filter";

    /**
     * element name
     */
    // <filter-set> element name
    public static final String ROOT_ELEMENT = "filter-set";
    // <ns-bindings> element name
    public static final String ELEMENT_NSBINDINGS = "ns-bindings";
    // <ns-binding> element name
    public static final String ELEMENT_NSBINDING = "ns-binding";
    // <filter> element name
    public static final String ELEMENT_FILTER = "filter";
    // <what> element name
    public static final String ELEMENT_WHAT = "what";
    // <trigger> element name
    public static final String ELEMENT_TRIGGER = "trigger";
    // <include> element name
    public static final String ELEMENT_INCLUDE = "include";
    // <exclude> element name
    public static final String ELEMENT_EXCLUDE = "exclude";
    // <changed> element name
    public static final String ELEMENT_CHANGED = "changed";
    // <added> element name
    public static final String ELEMENT_ADDED = "added";
    // <removed> element name
    public static final String ELEMENT_REMOVED = "removed";

    class ElementAttributeName {
        // <filter-set> attribute name
        public static final String XMLNS = "xmlns";

        // <ns-binding> attribute name
        public static final String PREFIX = "prefix";
        public static final String URN = "urn";

        // <filter> attribute name
        public static final String ID = "id";
        public static final String URI = "uri";
        public static final String DOMAIN = "domain";
        public static final String REMOVE = "remove";
        public static final String ENABLED = "enabled";

        // <include> ,<exclude> attribute name
        public static final String TYPE = "type";

        // <changed> attribute name
        public static final String FROM = "from";
        public static final String TO = "to";
        public static final String BY = "by";

    }

    /**
     * Constructor
     * 
     * @param initialDocmentSelector The initial document selector
     */
    public SimpleFilterDocument(String initialDocmentSelector) {
        mInitialXMLDocumentSelector = initialDocmentSelector;
    }

    /**
     * Create a <filer-set> element(root element)
     * 
     * @param childElements ChildElements of the <filer-set> element
     */
    public ElementInfo createFilterSetElement(List<ElementInfo> childElements) {
        if (childElements == null) {
            if(logger.isActivated()){
                logger.debug("createFilterSetElement():childElements is null");
            }
            return null;
        }
        ElementInfo element = new ElementInfo(ROOT_ELEMENT);
        element.addAttribute(ElementAttributeName.XMLNS, XMLNS_VALUE);
        element.setChildElements(childElements);
        return element;
    }

    /**
     * Create a <ns-bindings> element
     * 
     * @param childElements ChildElements of the <ns-bindings> element
     */
    public ElementInfo createNsBindingsElement(List<ElementInfo> childElements) {
        if (childElements == null) {
            if(logger.isActivated()){
                logger.debug("createNsBindingsElement():childElements is null");
            }
            return null;
        }
        ElementInfo element = new ElementInfo(ELEMENT_NSBINDINGS);
        element.setChildElements(childElements);
        return element;
    }

    /**
     * Create a <ns-binding> element
     * 
     * @param prefix Prefix is the value of 'prefix' attribute used to qualify
     *            the elements pointed to by expression,must have a not empty
     *            value,value of prefix may be 'pidf','rpidf','wi'
     * @param urn Urn identifies the namespace that the prefix represented,can
     *            not be null or empty,value of urn may be
     *            'urn:ietf:params:xml:ns:pidf',
     *            'urn:ietf:params:xml:ns:pidf:rpidf-tuple',
     *            'urn:ietf:params:xml:ns:wathcerinfo'
     */
    public ElementInfo createNsBindingElement(String prefix, String urn) {
        if (null == prefix || EMPTY_STRING.equals(prefix.trim()) || null == urn
                || EMPTY_STRING.equals(urn.trim())) {
            if(logger.isActivated()){
                logger.debug("createNsBindingElement():the <ns-binding> element not be created "
                        + "if prefix or urn attribute has no value");
            }
            return null;
        }
        ElementInfo element = new ElementInfo(ELEMENT_NSBINDING);
        element.addAttribute(ElementAttributeName.PREFIX, prefix);
        element.addAttribute(ElementAttributeName.URN, urn);
        return element;
    }

    /**
     * Create a <filter> element
     * 
     * @param id Id must be unique within the <filter> element
     * @param uri The <filter> element may have a 'uri' attribute, the value of
     *            'uri' attribute is the URI of the resource to which the filter
     *            applies,the value of 'uri' may be 'sip:buddylist@example.com'
     * @param domain The <filter> element may have a 'domain' attribute, the
     *            value of 'domain' attribute is the domain of the resource to
     *            which the filter applies,the value of 'domain' may be
     *            'example.com'
     * @param remove The <filter> element may have a 'remove' attribute that
     *            together with the 'id' attribute indicates the existing filter
     *            to be removed, Remove is optional,can be null
     * @param enabled The <filter> element may have a 'enabled' attribute that
     *            indicates whether a filter is enabled or disabled,Enabled is
     *            optional,can be null
     * @param childElements ChildElements of the <filter> element
     */
    public ElementInfo createFilterElement(String id, String uri, String domain, Boolean remove,
            Boolean enabled, List<ElementInfo> childElements) {
        if (childElements == null) {
            if(logger.isActivated()){
                logger.debug("createFilterElement():childElements is null");
            }
            return null;
        }
        /**
         * Indicate whether child elements of the 'filter' element contains the
         * 'what' element or the 'trigger' element.
         */
        boolean whateOrTriggerFlag = false;

        // all attributes ,which must not be null or empty, have value
        if (null == id || EMPTY_STRING.equals(id.trim())) {
            if(logger.isActivated()){
                logger.debug("createFilterElement():the 'id' must have value in the <filter> element"
                        + " ,or not create <filter> element");
            }
            return null;
        }

        /**
         * the 'uri' attribute and the 'domain' attribute must not appear
         * together in the <filter> element,but must have one of them
         */
        if ((null != uri && !EMPTY_STRING.equals(uri.trim()) && (null != domain && !EMPTY_STRING
                .equals(domain.trim())))
                || ((null == domain || EMPTY_STRING.equals(domain.trim())) && (null == uri || EMPTY_STRING
                        .equals(uri.trim())))) {
            if(logger.isActivated()){
                logger.debug("createFilterElement():the 'uri' attribute and the 'domain' attribute "
                        + "must not appear together in the <filter> element and must have one apprear"
                        + " ,or  not create <filter>element");
            }
            return null;
        }

        for (ElementInfo element : childElements) {
            if (element.getLocalName().equals(ELEMENT_WHAT)
                    || element.getLocalName().equals(ELEMENT_TRIGGER)) {
                whateOrTriggerFlag = true;
                break;
            }
        }

        if (whateOrTriggerFlag == false) {
            if(logger.isActivated()){
                logger.debug("createFilterElement:<filter> element must contains either the 'what' element or 'trigger'");
            }
            return null;
        }

        if(logger.isActivated()){
            logger.debug("createFilterElement: match all conditions, the <filter> element will be created");
        }

        ElementInfo element = new ElementInfo(ELEMENT_FILTER);
        element.addAttribute(ElementAttributeName.ID, id);
        if ((null != uri && !uri.trim().equals(EMPTY_STRING))) {
            element.addAttribute(ElementAttributeName.URI, uri);
        } else {
            element.addAttribute(ElementAttributeName.DOMAIN, domain);
        }
        if (null != remove) {
            element.addAttribute(ElementAttributeName.REMOVE, remove.toString());
        }
        if (null != enabled) {
            element.addAttribute(ElementAttributeName.ENABLED, enabled.toString());
        }
        element.setChildElements(childElements);
        return element;
    }

    /**
     * Create a <what> element
     * 
     * @param childElements ChildElements of the <what> element
     */
    public ElementInfo createWhatElement(List<ElementInfo> childElements) {
        if (childElements == null) {
            if(logger.isActivated()){
                logger.debug("createWhatElement():childElements is null");
            }
            return null;
        }
        ElementInfo element = new ElementInfo(ELEMENT_WHAT);
        element.setChildElements(childElements);
        return element;
    }

    /**
     * Create a <include> element
     * 
     * @param characters Characters between '<include>' and </include>,must have
     *            a not empty value
     * @param type Type is optional,indicates the type of the content to be
     *            delivered,can be null empty.if omitted,the default value is
     *            "xpath"
     */
    public ElementInfo createIncludeElement(String characters, String type) {
        if (null == characters || EMPTY_STRING.equals(characters.trim())) {
            if(logger.isActivated()){
                logger.debug("createIncludeElement():the <include> element not be created if characters have no value");
            }
            return null;
        }
        ElementInfo element = new ElementInfo(ELEMENT_INCLUDE);
        element.setContentText(characters);
        if (null == type || EMPTY_STRING.equals(type.trim())) {
            if(logger.isActivated()){
                logger.debug("createIncludeElement():type is optional attribute,if ommited, the default value is 'xpath'");
            }
        } else {
            element.addAttribute(ElementAttributeName.TYPE, type);
        }
        return element;
    }

    /**
     * Create a <exclude> element
     * 
     * @param characters Characters between '<exclude>' and </exclude>,must have
     *            a not empty value
     * @param type Type is optional,can be null empty.if omitted,the default
     *            value is "xpath"
     */
    public ElementInfo createExcludeElement(String characters, String type) {
        if (null == characters || EMPTY_STRING.equals(characters.trim())) {
            if(logger.isActivated()){
                logger.debug("createExcludeElement():the <exclude> element not be created if characters have no value");
            }
            return null;
        }
        ElementInfo element = new ElementInfo(ELEMENT_EXCLUDE);
        element.setContentText(characters);
        if (null == type || EMPTY_STRING.equals(type.trim())) {
            if(logger.isActivated()){
                logger.debug("createExcludeElement():type is optional attribute,if ommited, the default value is 'xpath'");
            }
        } else {
            element.addAttribute(ElementAttributeName.TYPE, type);
        }
        return element;
    }

    /**
     * Create a <trigger> element
     * 
     * @param childElements ChildElements of the <trigger> element
     */
    public ElementInfo createTriggerElement(List<ElementInfo> childElements) {
        if (childElements == null) {
            if(logger.isActivated()){
                logger.debug("createTriggerElement():childElements is null");
            }
            return null;
        }

        /**
         * Indicate whether child elements of the <trigger> element contains the
         * <added> element or the <changed> element or the <removed> element.
         */
        boolean flag_exist = false;

        for (ElementInfo element : childElements) {
            if (element.getLocalName().equals(ELEMENT_ADDED)
                    || element.getLocalName().equals(ELEMENT_CHANGED)
                    || element.getLocalName().equals(ELEMENT_REMOVED)) {
                flag_exist = true;
                break;
            }
        }
        if (flag_exist == true) {
            ElementInfo element = new ElementInfo(ELEMENT_TRIGGER);
            element.setChildElements(childElements);
            return element;
        }
        if(logger.isActivated()){
            logger.debug("createTriggerElement():<trigger> element must contains at at least one of "
                    + "three elements(<added>,<changed>,<removed>),or the <trigger> element  not be created");
        }
        return null;

    }

    /**
     * Create a <changed> element
     * 
     * @param characters Characters between '<changed>' and </changed>,must have
     *            a not empty value
     * @param from From is optional,can be null empty.A trigger is active when
     *            the XML element or attribute identified with the <changed>
     *            element has changed from the value indicated by this attribute
     *            to a different value.
     * @param to To is optional,can be null empty.A trigger is active when the
     *            XML element or attribute identified with the <changed> element
     *            has changed to the value indicated by this attribute from a
     *            different value.
     * @param by By is optional,can be null empty.A trigger is active when the
     *            XML element or attribute identified with the <changed> element
     *            has changed by at least the amount indicated by this attribute
     *            from a different value. That is, the 'by¡' attribute applies
     *            only to numerical values and indicates a delta with respect to
     *            the current value that an attribute or element (identified in
     *            the <changed> element) needs to change before it is selected.
     *            For example, if the 'by' attribute is set to 2 and the current
     *            value of the element/attribute is 6, the element/attribute is
     *            selected when it reaches (or exceeds) the value 8 or when it
     *            decreases to 4 or a lower value.
     */
    public ElementInfo createChangedElement(String characters, String from, String to, String by) {
        if (null == characters || EMPTY_STRING.equals(characters.trim())) {
            if(logger.isActivated()){
                logger.debug("createChangedElement():the <changed> element not be created if characters have no value");
            }
            return null;
        }

        // if the attribute 'by' has a not empty value and is not a digit,return
        if (null != by && !EMPTY_STRING.equals(by) && !TextUtils.isDigitsOnly(by)) {
            if(logger.isActivated()){
                logger.debug("createChangedElement():the <changed> element not be created if the attribute 'by' is not dtigit");
            }
            return null;
        }

        ElementInfo element = new ElementInfo(ELEMENT_CHANGED);
        element.setContentText(characters);

        if (null != by && !EMPTY_STRING.equals(by) && TextUtils.isDigitsOnly(by)) {
            element.addAttribute(ElementAttributeName.BY, by);
        }
        if (null != from && !EMPTY_STRING.equals(from)) {
            element.addAttribute(ElementAttributeName.FROM, from);
        }
        if (null != to && !EMPTY_STRING.equals(to)) {
            element.addAttribute(ElementAttributeName.TO, to);
        }

        return element;
    }

    /**
     * Create a <added> element
     * 
     * @param characters Characters between '<add>' and </add>
     */
    public ElementInfo createAddedElement(String characters) {
        if (null == characters || EMPTY_STRING.equals(characters.trim())) {
            if(logger.isActivated()){
                logger.debug("createAddedElement():the <added> element not be created if characters have no value");
            }
            return null;
        }
        ElementInfo element = new ElementInfo(ELEMENT_ADDED);
        element.setContentText(characters);
        return element;
    }

    /**
     * Create a <removed> element
     * 
     * @param characters Characters between '<removed>' and </removed>
     */
    public ElementInfo createRemovedElement(String characters) {
        if (null == characters || EMPTY_STRING.equals(characters.trim())) {
            if(logger.isActivated()){
                logger.debug("createRemovedElement():the <removed> element not be created if characters have no value");
            }
            return null;
        }
        ElementInfo element = new ElementInfo(ELEMENT_REMOVED);
        element.setContentText(characters);
        return element;
    }

}

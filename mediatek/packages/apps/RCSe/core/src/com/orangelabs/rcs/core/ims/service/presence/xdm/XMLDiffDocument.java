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

package com.orangelabs.rcs.core.ims.service.presence.xdm;

import java.util.ArrayList;
import java.util.List;
import java.util.Vector;

import org.xml.sax.Attributes;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.DefaultHandler;

import com.orangelabs.rcs.utils.XMLUtils.ElementInfo;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * An XML diff document that contains patch operation elements, namespace
 * declarations and all the document content changes that are needed in order to
 * transform an initial XML document into a new patched XML document.(Refer
 * RFC5261)
 */
public class XMLDiffDocument {
	/**
	 * Class name tag.
	 */
	private static final String TAG = "XMLDiffDocument";

	/**
     * The logger
     */
	private static Logger sLogger = Logger.getLogger(TAG);
	
	/**
	 * The XCAP document selector, it's the URI of an initial XML document that
	 * to be patched.
	 */
	private String mInitialXMLDocumentSelector;

	/**
	 * Container stores the XCAP aggregation of update commands, contains <add>,
	 * <replace> and <remove> element.
	 * 
	 */
	private List<ElementInfo> mPatchOperationList = new ArrayList<ElementInfo>();

	/**
	 * 
	 * Patch operation <diff> element.
	 * 
	 */
	public static class ElementDiff extends ElementInfo {
		/**
		 * Element flag.
		 */
		public static final String LOCAL_NAME = "diff";

		public ElementDiff() {
			super(LOCAL_NAME);
		}
	}

	/**
	 * 
	 * Patch operation <add> element.
	 * 
	 */
	public static class ElementAdd extends ElementInfo {
		/**
		 * Element flag.
		 */
		public static final String LOCAL_NAME = "add";

		public static final String ATTRIBUTE_POS_PREPEND = "prepend";
		public static final String ATTRIBUTE_POS_BEFORE = "before";
		public static final String ATTRIBUTE_POS_AFTER = "after";

		/**
		 * Element attribute.
		 */
		public static final String ATTRIBUTE_SEL = "sel";
		public static final String ATTRIBUTE_TYPE = "type";
		public static final String ATTRIBUTE_POS = "pos";

		public ElementAdd() {
			super(LOCAL_NAME);
		}
	}

	/**
	 * 
	 * Patch operation <replace> element.
	 * 
	 */
	public static class ElementReplace extends ElementInfo {
		/**
		 * Element flag.
		 */
		public static final String LOCAL_NAME = "replace";
		public static final String ATTRIBUTE_SEL = "sel";

		public ElementReplace() {
			super(LOCAL_NAME);
		}
	}

	/**
	 * 
	 * Patch operation <remove> element.
	 * 
	 */
	public static class ElementRemove extends ElementInfo {
		/**
		 * Element flag.
		 */
		public static final String LOCAL_NAME = "remove";

		/**
		 * Element attribute.
		 */
		public static final String ATTRIBUTE_SEL = "sel";
		public static final String ATTRIBUTE_WS = "ws";

		public static final String ATTRIBUTE_WS_BEFORE = "before";
		public static final String ATTRIBUTE_WS_AFTER = "after";
		public static final String ATTRIBUTE_WS_BOTH = "both";

		public ElementRemove() {
			super(LOCAL_NAME);
		}
	}

	/**
	 * Constructor
	 * 
	 * @param initialDocmentSelector
	 *            The initial document selector, this document will be update by
	 *            the XCAP aggregation update command.
	 */
	public XMLDiffDocument(String initialDocmentSelector) {
		if (null != initialDocmentSelector) {
			sLogger.debug("XMLDiffDocument: initialDocmentSelector = "
					+ initialDocmentSelector);
			mInitialXMLDocumentSelector = initialDocmentSelector;
		}

	}

	/**
	 * Get the patch operation root element.
	 * 
	 * @return Patch operation root element that contains <add>, <replace> and <remove> child element
	 *         and so on.
	 */
	public ElementInfo getDocumentRootElement() {
		sLogger.debug("getDocumentRootElement");
		ElementDiff rootElementDiff = new ElementDiff();
		rootElementDiff.setLocalName(ElementDiff.LOCAL_NAME);
		rootElementDiff.setQName(ElementDiff.LOCAL_NAME);
		rootElementDiff.setChildElements(mPatchOperationList);
		
		return rootElementDiff;
	}

	/**
	 * Add the patch operation into list.
	 * 
	 * @param Patch
	 *            operation list that contains <add>, <replace> and <remove>
	 *            element.
	 */
	private void addPatchOperation(ElementInfo patchOperation) {
		sLogger.debug("addPatchOperation");
		if (null != patchOperation) {
			synchronized (mPatchOperationList) {
				mPatchOperationList.add(patchOperation);
			}
		} else {
			sLogger.debug("addPatchOperation: patchOperation is null.");
		}
	}

	/**
	 * Clear the patch operation list.
	 */
	public void clearPatchOperation() {
		sLogger.debug("clearPatchOperation");

		synchronized (mPatchOperationList) {
			mPatchOperationList.clear();
		}
	}

	/**
	 * Create document for the xml-diff document.
	 * 
	 * @param localName
	 *            Element local name. It's alse the element tag name.
	 * @return element
	 */
	public static ElementInfo createElement(String localName) {
		ElementInfo element = null ;
		
		if (null != localName) {
			sLogger.debug("createElement: localName = " + localName);
		}else {
			sLogger.debug("createElement: localName is null.");
			return element;
		}

		if (localName.equals(ElementDiff.LOCAL_NAME)) {
			element = new ElementDiff();
		} else if (localName.equals(ElementAdd.LOCAL_NAME)) {
			element = new ElementAdd();
		} else if (localName.equals(ElementReplace.LOCAL_NAME)) {
			element = new ElementReplace();
		} else if (localName.equals(ElementRemove.LOCAL_NAME)) {
			element = new ElementRemove();
		} else {
			element = new ElementInfo(localName);
		}

		return element;
	}

	/**
	 * Create a <add> element
	 * @param uri
	 * @param localName
	 * @param qName
	 * @param attrSel
	 * @param attrType
	 * @param attrPos
	 * @param childElements
	 */
	public void addAddingPatchOpration(String uri, String localName,
			String qName, String attrSel, String attrType, String attrPos,
			List<ElementInfo> childElements) {
		sLogger.debug("addAddingPatchOpration: uri = " + uri + ", localname = "
				+ localName + ", qName = " + qName);

		ElementAdd element = (ElementAdd) createElement(ElementAdd.LOCAL_NAME);
		element.setUri(uri);
		element.setLocalName(ElementAdd.LOCAL_NAME);
		element.setQName(ElementAdd.LOCAL_NAME);
		element.addAttribute(ElementAdd.ATTRIBUTE_SEL, attrSel);
		element.addAttribute(ElementAdd.ATTRIBUTE_TYPE, attrType);
		element.addAttribute(ElementAdd.ATTRIBUTE_POS, attrPos);
		element.setChildElements(childElements);
		addPatchOperation(element);
	}

	/**
	 * Create a <replace> element
	 * 
	 * @param uri
	 * @param localName
	 * @param qName
	 * @param attrSel
	 *            Node selector to locate the target of a patch
	 * @param childElements
	 */
	public void addReplacingPatchOpration(String uri, String localName,
			String qName, String attrSel, List<ElementInfo> childElements) {
		sLogger.debug("addReplacingPatchOpration: uri = " + uri
				+ ", localname = " + localName + ", qName = " + qName);
		ElementReplace element = (ElementReplace) createElement(ElementReplace.LOCAL_NAME);
		element.setUri(uri);
		element.setLocalName(ElementReplace.LOCAL_NAME);
		element.setQName(ElementReplace.LOCAL_NAME);
		element.addAttribute(ElementReplace.ATTRIBUTE_SEL, attrSel);
		element.setChildElements(childElements);
		addPatchOperation(element);
	}

	/**
	 * Create a <remove> element
	 * 
	 * @param uri
	 * @param localName
	 * @param qName
	 * @param attrSel
	 *            Node selector to locate the target of a patch
	 * @param attrWs
	 */
	public void addRemovingPatchOpration(String uri, String localName,
			String qName, String attrSel, String attrWs) {
		sLogger.debug("addRemovingPatchOpration: uri = " + uri
				+ ", localname = " + localName + ", qName = " + qName);
		ElementRemove element = (ElementRemove) createElement(ElementRemove.LOCAL_NAME);
		element.setUri(uri);
		element.setLocalName(ElementRemove.LOCAL_NAME);
		element.setQName(ElementRemove.LOCAL_NAME);
		element.addAttribute(ElementRemove.ATTRIBUTE_SEL, attrSel);
		element.addAttribute(ElementRemove.ATTRIBUTE_WS, attrWs);
		addPatchOperation(element);
	}

	/**
	 * Default Handler, use it to parse Diff-XML document.
	 * 
	 */
	public static class SaxParsDiffDocumentHandler extends DefaultHandler {
		/**
		 * Class name tag.
		 */
		private static final String TAG = "SaxParsDiffDocumentHandler";

		/**
		 * Container stores the XCAP aggregation of update commands, contains
		 * <add>, <replace> and <remove> element.
		 */
		private ElementInfo mPatchOperationRootElement = null;

		/**
		 * A reference that refer one of <add>, <replace> and <remove> element
		 */
		private List<ElementInfo> mTempHierachy;

		/**
		 * A reference refer any element.
		 */
		private ElementInfo mTempElement;

		/**
		 * Get the patch operation root element.
		 * 
		 * @return root element.
		 */
		public ElementInfo getPatchOperationRootElement() {
			return mPatchOperationRootElement;
		}

		@Override
		public void startDocument() throws SAXException {
			sLogger.debug("startDocument");
			mTempHierachy = new Vector<ElementInfo>();
		}

		@Override
		public void startElement(String uri, String localName, String qName,
				Attributes attributes) throws SAXException {
			sLogger.debug("startElement: uri = " + uri + ", localName = "
					+ localName + ", qName = " + qName);
				mTempElement = createElement(localName);
				mTempElement.setUri(uri);
				mTempElement.setLocalName(localName);
				mTempElement.setQName(qName);
				
				int attrSize = attributes.getLength();
				
				for (int i = 0; i < attrSize; i++) {
					mTempElement.addAttribute(attributes.getQName(i), attributes.getValue(i));
				}
				
				mTempHierachy.add(mTempElement);
		}

		@Override
		public void endElement(String uri, String localName, String qName)
				throws SAXException {
			sLogger.debug("endElement: uri = " + uri + ", localName = "
					+ localName + ", qName = " + qName);
			if (!mTempHierachy.isEmpty()) {
				int size = mTempHierachy.size();
				int lastIndex = size - 1;
				int last2ndIndex = size -2;
				ElementInfo elementInfo;
				if (0 < lastIndex) {
					elementInfo = mTempHierachy.get(lastIndex);
					if (0 <= last2ndIndex) {
						mTempHierachy.get(last2ndIndex).addChildElement(elementInfo);
						mTempHierachy.remove(lastIndex);
					}
				}else if (1 == size) {
					mPatchOperationRootElement = mTempHierachy.get(0);
				}
			}
			
			mTempElement = null;
		}

		@Override
		public void characters(char[] ch, int start, int length)
				throws SAXException {
			String charcters = new String(ch, start, length);
			sLogger.debug("characters: charcters = " + charcters);
			if (mTempElement != null) {
				mTempElement.setContentText(charcters);
			}
		}

		@Override
		public void endDocument() throws SAXException {
			sLogger.debug("endDocument");
			super.endDocument();
		}
	}
}

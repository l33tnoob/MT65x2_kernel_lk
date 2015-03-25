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

import java.io.ByteArrayOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.util.HashMap;
import java.util.List;
import java.util.Map;
import java.util.Map.Entry;
import java.util.Set;

import org.dom4j.Attribute;
import org.dom4j.Document;
import org.dom4j.Element;
import org.dom4j.Namespace;
import org.dom4j.Node;
import org.dom4j.io.SAXReader;

import com.orangelabs.rcs.core.ims.service.presence.xdm.XMLDiffDocument.ElementAdd;
import com.orangelabs.rcs.core.ims.service.presence.xdm.XMLDiffDocument.ElementRemove;
import com.orangelabs.rcs.core.ims.service.presence.xdm.XMLDiffDocument.ElementReplace;
import com.orangelabs.rcs.utils.XMLUtils;
import com.orangelabs.rcs.utils.XMLUtils.ElementInfo;
import com.orangelabs.rcs.utils.logger.Logger;

/**
 * It uses the XML tool 'Dom4j' and 'XPath' to modify a existing XML document.
 */
public class XMLPatchedWithDom4jAndXPath {
	/**
	 * class name tag.
	 */
	private static final String TAG = "XMLPatchedWithDom4jAndXPath";

	/**
	 * The logger
	 */
	private static final Logger sLogger = Logger.getLogger(TAG);

	/**
	 * Constants
	 */
	private static final String AT_SYMBOL = "@";
	private static final String SLASH_SYMBOL = "/";
	private static final String COLON_SYMBOL = ":";
	private static final String STAR_SYMBOL = "*";
	private static final String NAMESPACE_PREF = "namespace::";
	private static final String DEFUALT_NAMESPACE_KEY = "xmlns_defualt";
	private static final String EMPTY_STRING = "";
	private static final String XPATH_ID_ATTRIBUTE = "@id";
	private static final String XPATH_TEXT_FUNCTION = "text()";
	private static final String XPATH_COMMENT_FUNCTION = "comment()";
	private static final String XPATH_PROCESSING_INSTRUCTION_FUNCTION = "processing-instruction()";

	/**
	 * The reference to the XML document object.
	 */
	private Document mTargetDocument = null;

	/**
	 * A SAXReader object.
	 */
	private SAXReader mSaxReader;

	/**
	 * Target XML document namespace declaration.
	 */
	private Map<String, String> mTargetNamespaceMap;

	/**
	 * The namespace declaration comes from diff XML document.
	 */
	private Map<String, String> mDiffNamespaceMap;

	/**
	 * The root element of the patch operation XML.
	 */
	private ElementInfo mPatchOperationRootElement;

	/**
	 * Constructor
	 */
	public XMLPatchedWithDom4jAndXPath() {
	}

	/**
	 * Modify the target XML document by the XML diff document.
	 * 
	 * @param targetInputStream
	 * @param diffInputStream
	 * @return the target output stream.
	 */
	public OutputStream patch(InputStream targetInputStream,
			InputStream diffInputStream) {
		OutputStream targetOutputStream = null;

		if (null == targetInputStream || null == diffInputStream) {
			sLogger.debug("patch: targetInputStream or diffInputStream is null.");
			return targetOutputStream;
		}

		mSaxReader = new SAXReader();
		mTargetDocument = XMLUtils.dom4jReadXMLToDocument(targetInputStream,
				mSaxReader);

		// SaxParsDiffDocumentHandler diffDocumentParseHandler = null;
		// diffDocumentParseHandler = new SaxParsDiffDocumentHandler();
		// XMLUtils.saxParseXML(diffInputStream, diffDocumentParseHandler);
		//
		// mPatchOperationRootElement = diffDocumentParseHandler
		// .getPatchOperationRootElement();

		// get target XML document namespace.
		getTargetXmlNamespaceMap();
		mPatchOperationRootElement = XMLUtils.dom4jParseXML(diffInputStream);
		getDiffXmlNamespaceMap();
		addNamespaceToXPath();
		addNewNamespaceToTarget();

		patch();

		targetOutputStream = new ByteArrayOutputStream();

		XMLUtils.dom4jWriteDocumentToXML(targetOutputStream, mTargetDocument);

		return targetOutputStream;
	}

	/**
	 * Get target XML namespace declaration.
	 * @return namespace uri map.
	 */
	private Map<String, String> getTargetXmlNamespaceMap() {
		sLogger.debug("getTargetXmlNamespaceMap");

		if (null == mTargetNamespaceMap) {
			mTargetNamespaceMap = new HashMap<String, String>();
		}

		Element root = mTargetDocument.getRootElement();
		// firstly, set the root element itself namespace.
		String rootNamespaPrefic = root.getNamespacePrefix();

		if (null == rootNamespaPrefic || EMPTY_STRING.equals(rootNamespaPrefic)) {
			mTargetNamespaceMap.put(DEFUALT_NAMESPACE_KEY,
					root.getNamespaceURI());
		} else {
			mTargetNamespaceMap.put(rootNamespaPrefic, root.getNamespaceURI());
		}

		// secondly, set additional namespace.
		List<Namespace> namespaceList = root.additionalNamespaces();

		for (Namespace namespace : namespaceList) {
			if (null == namespace.getPrefix()
					|| EMPTY_STRING.equals(namespace.getPrefix())) {
				mTargetNamespaceMap.put(DEFUALT_NAMESPACE_KEY,
						namespace.getURI());
			} else {
				mTargetNamespaceMap.put(namespace.getPrefix(),
						namespace.getURI());
			}
		}

		return mTargetNamespaceMap;
	}

	/**
	 * Add new namespace declaration into XPath for selecting node.
	 */
	private void addNamespaceToXPath() {
		sLogger.debug("addNamespaceToXPath");
		Map<String, String> namespaceUris = new HashMap<String, String>();
		namespaceUris.putAll(mTargetNamespaceMap);
		namespaceUris.putAll(mDiffNamespaceMap);
		mSaxReader.getDocumentFactory().setXPathNamespaceURIs(namespaceUris);
	}

	/**
	 * Add new namespace declaration into targe XML document.
	 */
	private void addNewNamespaceToTarget() {
		if (null == mDiffNamespaceMap) {
			sLogger.debug("addNewNamespaceForTarget: mDiffNamespaceMap is null.");
			return;
		}

		Set<Entry<String, String>> entries = mDiffNamespaceMap.entrySet();
		Element targetRootElement = mTargetDocument.getRootElement();
		String key;
		for (Entry<String, String> entry : entries) {
			key = entry.getKey();
			// Don't add the XML diff document default namesapce URI into target
			// document.
			if (null == key || EMPTY_STRING.equals(key)) {
				continue;
			} else {
				targetRootElement.addNamespace(key, entry.getValue());
			}
		}

	}

	/**
	 * Get the namespace declaration from diff XML document.
	 * @return namespace uri map.
	 */
	private Map<String, String> getDiffXmlNamespaceMap() {
		sLogger.debug("getDiffXmlNamespaceMap");

		mDiffNamespaceMap = mPatchOperationRootElement.getNamespaceUriMap();

		return mDiffNamespaceMap;
	}

	/**
	 * Modify the target XML document by the XML diff document.
	 */
	private void patch() {
		ElementInfo patchOperationRootElement = mPatchOperationRootElement;
		List<ElementInfo> childElements;
		if (null == patchOperationRootElement) {
			sLogger.debug("patch: patchOperationElement is null.");
			return;
		} else {
			childElements = patchOperationRootElement.getChildElements();
			sLogger.debug("patch: patchOperationRootElement child elements size = "
					+ childElements.size());
		}

		this.mPatchOperationRootElement = patchOperationRootElement;

		Map<String, String> attrMap = patchOperationRootElement
				.getAttributeMap();
		Set<Entry<String, String>> entrySet = attrMap.entrySet();
		for (Entry<String, String> entry : entrySet) {
			sLogger.debug("entry.getKey() = " + entry.getKey()
					+ ", entry.getValue()" + entry.getValue());
		}

		for (ElementInfo element : childElements) {
			if (element.getLocalName().contains(ElementAdd.LOCAL_NAME)) {
				patchAdd(element);
			} else if (element.getLocalName().contains(
					ElementReplace.LOCAL_NAME)) {
				patchReplace(element);
			} else if (element.getLocalName()
					.contains(ElementRemove.LOCAL_NAME)) {
				patchRemove(element);
			}
		}
	}

	/**
	 * Add element, attribute or namesapce at the specified location in target
	 * XML document.
	 * 
	 * @param elementAdd
	 */
	private void patchAdd(ElementInfo elementAdd) {
		String attrSel = null;

		if (null == elementAdd) {
			sLogger.debug("patchAdd: elementAdd is null.");
			return;
		} else {
			attrSel = elementAdd.getAttribute(ElementAdd.ATTRIBUTE_SEL);
			sLogger.debug("patchAdd: elementAdd attribue selector = " + attrSel);
		}
		String attrType = elementAdd.getAttribute(ElementAdd.ATTRIBUTE_TYPE);
		String attrPos = elementAdd.getAttribute(ElementAdd.ATTRIBUTE_POS);
		List<ElementInfo> childElements = elementAdd.getChildElements();

		Node selectedNode = selectNode(attrSel);
		short nodeType = 0;

		if (null == selectedNode) {
			sLogger.debug("patchAdd: selectedNode is null.");
			return;
		} else {
			nodeType = selectedNode.getNodeType();
			sLogger.debug("patchAdd: selectedNode type = " + nodeType);
		}

		switch (nodeType) {
		case Node.ATTRIBUTE_NODE:

			break;
		case Node.COMMENT_NODE:

			break;
		case Node.ELEMENT_NODE:
			if (null != attrType) {
				if (attrType.contains(AT_SYMBOL)) {
					addAttribute((Element) selectedNode, elementAdd, attrType);
				} else if (attrType.contains(NAMESPACE_PREF)) {
					addNamespace((Element) selectedNode, elementAdd, attrType);
				}
			} else {
				addElement((Element) selectedNode, elementAdd, attrPos);
			}
			break;
		case Node.TEXT_NODE:

			break;
		case Node.NAMESPACE_NODE:

			break;
		}

	}

	/**
	 * Select a node by dom4j.
	 * 
	 * @param xPathExpression
	 * @return selected node.
	 */
	private Node selectNode(String xPathExpression) {
		sLogger.debug("selectNode: xPathExpression = " + xPathExpression);
		xPathExpression = processXPathByNamespace(xPathExpression);
		return mTargetDocument.selectSingleNode(xPathExpression);
	}

	/**
	 * Process the XPath expression if there is default namespace declaration in
	 * XML document.
	 * 
	 * @param xPathExpression
	 * @return
	 */
	private String processXPathByNamespace(String xPathExpression) {
		sLogger.debug("processXPathByNamespace: xPathExpression = "
				+ xPathExpression);
		String resultXpathExpression = null;
		String[] subPathArray = xPathExpression.split(SLASH_SYMBOL);
		String subPath;
		for (int i = 0; i < subPathArray.length; i++) {
			subPath = subPathArray[i];
			if (subPath.equals(EMPTY_STRING) || subPath.equals(STAR_SYMBOL)
					|| subPath.contains(COLON_SYMBOL)
					|| subPath.contains(XPATH_ID_ATTRIBUTE)
					|| subPath.contains(XPATH_TEXT_FUNCTION)
					|| subPath.contains(XPATH_COMMENT_FUNCTION)
					|| subPath.contains(XPATH_PROCESSING_INSTRUCTION_FUNCTION)) {
				// donothing.
			} else {
				subPath = DEFUALT_NAMESPACE_KEY + COLON_SYMBOL + subPath;
			}

			if (0 == i) {
				if (xPathExpression.startsWith(SLASH_SYMBOL)
						&& !EMPTY_STRING.equals(subPath)) {
					resultXpathExpression = SLASH_SYMBOL + subPath;
				} else {
					resultXpathExpression = subPath;
				}
			} else {
				resultXpathExpression = resultXpathExpression + SLASH_SYMBOL
						+ subPath;
			}

		}

		sLogger.debug("processXPathByNamespace: After processing, the resultXpathExpression = "
				+ resultXpathExpression);
		return resultXpathExpression;
	}

	/**
	 * Add a attribute.
	 * @param selectedElement
	 * @param elementAdd
	 * @param attrType
	 */
	private void addAttribute(Element selectedElement, ElementInfo elementAdd,
			String attrType) {
		if (null == selectedElement || null == elementAdd) {
			sLogger.debug("addAttribute: Parameter selectedElement or elementAdd is null.");
			return;
		}
		
		String attrName = attrType.substring(attrType.indexOf(AT_SYMBOL) + 1);
		String attrValue = elementAdd.getContentText();
		selectedElement.addAttribute(attrName, attrValue);
	}

	/**
	 * Add a namespace declaration.
	 * @param selectedElement
	 * @param elementAdd
	 * @param attrType
	 */
	private void addNamespace(Element selectedElement, ElementInfo elementAdd,
			String attrType) {
		if (null == selectedElement || null == elementAdd) {
			sLogger.debug("addNamespace: Parameter selectedElement or elementAdd is null.");
			return;
		}
		String nsPrifex = attrType.substring(NAMESPACE_PREF.length());
		String nsURI = elementAdd.getContentText();
		selectedElement.addNamespace(nsPrifex, nsURI);
	}

	/**
	 * Add a element.
	 * @param selectedElement
	 * @param elementAdd
	 * @param attrPos
	 */
	private void addElement(Element selectedElement, ElementInfo elementAdd,
			String attrPos) {
		if (null == selectedElement || null == elementAdd) {
			sLogger.debug("addNamespace: Parameter selectedElement or elementAdd is null.");
			return;
		}
		Element rootElement = mTargetDocument.getRootElement();
		List rootElementList = rootElement.content();
		ElementInfo myElement = null;
		List<ElementInfo> childElements = elementAdd.getChildElements();

		int size = childElements.size();

		if (null != attrPos) {
			if (attrPos.equals(ElementAdd.ATTRIBUTE_POS_BEFORE)) {
				for (int i = 0; i < size; i++) {
					myElement = childElements.get(i);
					rootElementList.add(
							rootElementList.indexOf(selectedElement),
							myElement.getQName());
				}

			} else if (attrPos.equals(ElementAdd.ATTRIBUTE_POS_AFTER)) {
				for (int i = 0; i < size; i++) {
					myElement = childElements.get(i);
					rootElementList.add(
							rootElementList.indexOf(selectedElement) + 1 + i,
							myElement.getQName());
				}
			} else if (attrPos.equals(ElementAdd.ATTRIBUTE_POS_PREPEND)) {
				List selectedElementList = null;

				for (int i = 0; i < size; i++) {
					myElement = childElements.get(i);
					selectedElementList = selectedElement.content();
					selectedElementList.add(i, myElement.getQName());
				}

				selectedElement.setContent(selectedElementList);
			}
		} else {
			for (int i = 0; i < size; i++) {
				myElement = childElements.get(i);
				addElement(selectedElement, myElement);
			}
		}

		rootElement.setContent(rootElementList);
	}

	/**
	 * Add a element.
	 * @param targetElement
	 * @param needAddElement
	 */
	private void addElement(Element targetElement, ElementInfo needAddedElement) {
		if (null == targetElement || null == needAddedElement) {
			sLogger.debug("addElement: Parameter targetElement or needAddedElement is null.");
			return;
		}
		
		Element newElement = targetElement
				.addElement(needAddedElement.getQName());
		Map<String, String> attrMap = needAddedElement.getAttributeMap();
		Set<Entry<String, String>> entrySet = attrMap.entrySet();
		for (Entry<String, String> entry : entrySet) {
			newElement.addAttribute(entry.getKey(), entry.getValue());
		}

		if (!needAddedElement.getChildElements().isEmpty()) {
			List<ElementInfo> childElements = needAddedElement.getChildElements();
			for (ElementInfo element : childElements) {
				addElement(newElement, element);
			}
		}
	}

	/**
	 * Modify the selected element, attribute or namesapce in target XML
	 * document.
	 * 
	 * @param elementReplace
	 */
	private void patchReplace(ElementInfo elementReplace) {
		if (null == elementReplace) {
			sLogger.debug("patchReplace: Parameter elementReplace is null.");
			return;
		}
		String attrSel = null;
		if (null == elementReplace) {
			sLogger.debug("patchReplace: elementReplace is null.");
			return;
		} else {
			attrSel = elementReplace.getAttribute(ElementReplace.ATTRIBUTE_SEL);
			sLogger.debug("patchReplace: elementReplace attribute selector = "
					+ attrSel);
		}

		Node selectedNode = selectNode(attrSel);
		short nodeType = 0;
		if (null == selectedNode) {
			sLogger.debug("patchReplace: selectedNode is null.");
			return;
		} else {
			nodeType = selectedNode.getNodeType();
			sLogger.debug("patchReplace: selectedNode type = " + nodeType);
		}

		switch (nodeType) {
		case Node.ATTRIBUTE_NODE:
			selectedNode.setText(elementReplace.getContentText());
			break;
		case Node.COMMENT_NODE:

			break;
		case Node.ELEMENT_NODE:
			replaceElement((Element) selectedNode, elementReplace);
			break;
		case Node.TEXT_NODE:
			selectedNode.setText(elementReplace.getContentText());
			break;
		case Node.NAMESPACE_NODE:
			selectedNode.setText(elementReplace.getContentText());
			break;
		}
	}

	/**
	 * Modify the target XML document.
	 * @param selectedElement
	 * @param elementReplace
	 */
	private void replaceElement(Element selectedElement,
			ElementInfo elementReplace) {
		if (null == elementReplace || null == elementReplace) {
			sLogger.debug("patchReplace: Parameter elementReplace or elementReplace is null.");
			return;
		}
		Element rootElement = mTargetDocument.getRootElement();
		List rootElementList = rootElement.content();
		List<ElementInfo> childElements = elementReplace.getChildElements();

		for (ElementInfo myElement : childElements) {
			rootElementList.add(rootElementList.indexOf(selectedElement),
					myElement.getQName());
		}

		rootElementList.remove(rootElementList.indexOf(selectedElement));
		rootElement.setContent(rootElementList);
	}

	/**
	 * Remove the selected element, attribute or namesapce in target XML
	 * document.
	 * 
	 * @param elementRemove
	 */
	private void patchRemove(ElementInfo elementRemove) {
		String attrSel;
		if (null == elementRemove) {
			sLogger.debug("patchRemove: elementRemove is null.");
			return;
		} else {
			attrSel = elementRemove.getAttribute(ElementAdd.ATTRIBUTE_SEL);
			sLogger.debug("patchRemove: elementRemove attribute selector = "
					+ attrSel);
		}

		String attrWs = elementRemove.getAttribute(ElementRemove.ATTRIBUTE_WS);

		Node selectedNode = selectNode(attrSel);

		short nodeType = 0;
		if (null == selectedNode) {
			sLogger.debug("patchRemove: selectedNode is null.");
			return;
		} else {
			nodeType = selectedNode.getNodeType();
			sLogger.debug("patchRemove: selectedNode type = " + nodeType);
		}

		switch (nodeType) {
		case Node.ATTRIBUTE_NODE:
			Attribute attribute = selectedNode.getParent().attribute(
					attrSel.substring(attrSel.indexOf(AT_SYMBOL)));
			selectedNode.getParent().remove(attribute);
			break;
		case Node.COMMENT_NODE:

			break;
		case Node.ELEMENT_NODE:
			selectedNode.getParent().remove(selectedNode);
			break;
		case Node.TEXT_NODE:
			selectedNode.getParent().setText(EMPTY_STRING);
			break;
		case Node.NAMESPACE_NODE:
			Namespace namespace = selectedNode.getParent()
					.getNamespaceForPrefix(
							attrSel.substring(attrSel.indexOf(NAMESPACE_PREF)));
			selectedNode.getParent().remove(namespace);
			break;

		default:
			break;
		}
	}
}
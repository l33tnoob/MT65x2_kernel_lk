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

package com.orangelabs.rcs.utils;

import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.io.UnsupportedEncodingException;
import java.util.ArrayList;
import java.util.HashMap;
import java.util.Iterator;
import java.util.List;
import java.util.Map;
import java.util.Set;
import java.util.Map.Entry;

import javax.xml.parsers.ParserConfigurationException;
import javax.xml.parsers.SAXParser;
import javax.xml.parsers.SAXParserFactory;
import javax.xml.transform.OutputKeys;
import javax.xml.transform.Result;
import javax.xml.transform.Transformer;
import javax.xml.transform.TransformerConfigurationException;
import javax.xml.transform.sax.SAXTransformerFactory;
import javax.xml.transform.sax.TransformerHandler;
import javax.xml.transform.stream.StreamResult;

import org.dom4j.Attribute;
import org.dom4j.Document;
import org.dom4j.DocumentException;
import org.dom4j.Element;
import org.dom4j.Namespace;
import org.dom4j.Node;
import org.dom4j.io.SAXReader;
import org.dom4j.io.XMLWriter;
import org.xml.sax.SAXException;
import org.xml.sax.helpers.AttributesImpl;
import org.xml.sax.helpers.DefaultHandler;

import com.orangelabs.rcs.utils.logger.Logger;

/**
 * A XML tool which is used to create a XML or parse a XML.
 */
public class XMLUtils {
	/**
	 * Class name tag.
	 */
	private static final String TAG = "XMLUtils";
	
	/**
     * The logger
     */
	private static Logger sLogger = Logger.getLogger(TAG);

	/**
	 * Identify the generated xml encoding.
	 */
	private static final String ENCODING_UTF8 = "UTF-8";

	/**
	 * Identify the generated xml whether need indent.
	 */

	private static final String INDENT_YES = "yes";

	/**
	 * It is used to store a XML element information.
	 */
	public static class ElementInfo {
		/**
		 * Class name tag.
		 */
		private static final String TAG = "ElementInfo";

		/**
		 * Namespace URI.
		 */
		protected String mUri;
		/**
		 * The qualified name (with prefix), or the empty string if qualified
		 * names are not available.
		 */
		protected String mQName;
		/**
		 * The local name (without prefix), or the empty string if Namespace
		 * processing is not being performed.
		 */
		protected String mLocalName;

		/**
		 * Element attribute.
		 */
		protected Map<String, String> mAttributeMap;

		/**
		 * Element namespace declaration.
		 */
		protected Map<String, String> mNamespaceUriMap;

		/**
		 * Element child elements.
		 */
		protected List<ElementInfo> mChildElements;
		/**
		 * Element content text.
		 */
		protected String mContentText;

		/**
		 * Element comment.
		 */
		protected String mComment;

		/**
		 * Constructor.
		 */
		public ElementInfo() {
			mAttributeMap = new HashMap<String, String>();
			mChildElements = new ArrayList<ElementInfo>();
		}

		/**
		 * Constructor.
		 */
		public ElementInfo(String localName) {
			setLocalName(localName);
			mAttributeMap = new HashMap<String, String>();
			mChildElements = new ArrayList<ElementInfo>();
		}

		/**
		 * Set the namespace Uri.
		 * 
		 * @param uri
		 */
		public void setUri(String uri) {
			if (null != uri) {
				this.mUri = uri;
				sLogger.debug("setUri: uri = " + uri);
			} else {
				sLogger.debug("setUri: uri is null.");
			}
		}

		/**
		 * Get the namespace Uri.
		 * 
		 * @return uri
		 */
		public String getUri() {
			sLogger.debug("getUri");
			return mUri;
		}

		/**
		 * Set namespace uri map.
		 * 
		 * @param namespaceUriMap
		 */
		public void setNamespaceUriMap(Map<String, String> namespaceUriMap) {
			if (null != namespaceUriMap) {
				sLogger.debug("setNamespaceUriMap: namespaceUriMap size = "
						+ namespaceUriMap.size());
				mNamespaceUriMap = namespaceUriMap;
			} else {
				sLogger.debug("setNamespaceUriMap: namespaceUriMap is null.");
			}
		}

		/**
		 * Get namespace uri map.
		 * 
		 * @return namespaceUriMap
		 */
		public Map<String, String> getNamespaceUriMap() {
			sLogger.debug("getNamespaceUriMap");
			return mNamespaceUriMap;
		}

		/**
		 * Set the qualified name.
		 * 
		 * @param qName
		 */
		public void setQName(String qName) {
			if (null != qName) {
				sLogger.debug("setQName: qName = " + qName);
				this.mQName = qName;
			} else {
				sLogger.debug("setQName is null.");
			}
		}

		/**
		 * Get the qualified name.
		 * 
		 * @return qName
		 */
		public String getQName() {
			sLogger.debug("getQName");
			return mQName;
		}

		/**
		 * Set the local name of the element.
		 * 
		 * @param localName
		 */
		public void setLocalName(String localName) {
			if (null != localName) {
				sLogger.debug("setLocalName: localName = " + localName);

				this.mLocalName = localName;
				if (null == mQName) {
					mQName = localName;
				}
			} else {
				sLogger.debug("setLocalName: localName is null.");
			}
		}

		/**
		 * Get the local name of the element.
		 * 
		 * @return local name
		 */
		public String getLocalName() {
			sLogger.debug("getLocalName");
			return mLocalName;
		}

		/**
		 * Add a Key-Value attribute.
		 * 
		 * @param attrName
		 * @param attrValue
		 */
		public void addAttribute(String attrName, String attrValue) {
			if (null != attrName && null != attrValue) {
				sLogger.debug("addAttribute: attrName = " + attrName
						+ ", attrValue = " + attrValue);
				mAttributeMap.put(attrName, attrValue);
			} else {
				sLogger.debug("addAttribute: attrName or attrValue is null or empty.");
			}
		}

		/**
		 * Get the attribute value from its name.
		 * 
		 * @param attrName
		 * @return attrValue
		 */
		public String getAttribute(String attrName) {
			String attrValue = null;
			if (null == attrName) {
				sLogger.debug("getAttribute: attrName is null or empty.");
			} else {
				sLogger.debug("getAttribute: attrName = " + attrName);
				attrValue = mAttributeMap.get(attrName);
			}

			return attrValue;
		}

		/**
		 * Set Key-Value attribute map.
		 * 
		 * @param attrbuteMap
		 */
		public void setAttributeMap(Map<String, String> attrbuteMap) {
			if (null != attrbuteMap) {
				sLogger.debug("setAttributeMap: attrbuteMap size = "
						+ attrbuteMap.size());
				mAttributeMap = attrbuteMap;
			} else {
				sLogger.debug("setAttributeMap: attrbuteMap is null.");
			}
		}

		/**
		 * Get the Key-Value attribute map.
		 * 
		 * @param attrbuteMap
		 */
		public Map<String, String> getAttributeMap() {
			sLogger.debug("getAttributeMap");
			return mAttributeMap;
		}

		/**
		 * Add a child element for the element.
		 * 
		 * @param attrName
		 * @param attrValue
		 */
		public void addChildElement(ElementInfo element) {
			if (null != element) {
				sLogger.debug("addChildElement: element = " + element);
				mChildElements.add(element);
			} else {
				sLogger.debug("addChildElement: element is null.");
			}
		}

		/**
		 * Set some child elements for the element.
		 * 
		 * @param childElements
		 *            A list contains these child elements.
		 */
		public void setChildElements(List<ElementInfo> childElements) {
			if (null != childElements) {
				sLogger.debug("setChildElements: childElements size = "
						+ childElements.size());
				mChildElements = childElements;
			} else {
				sLogger.debug("setChildElements: childElements is null.");
			}

		}

		/**
		 * Get the child elements of the element.
		 * 
		 * @return childElements A list contains these child elements.
		 */
		public List<ElementInfo> getChildElements() {
			sLogger.debug("getChildElements");

			return mChildElements;
		}

		/**
		 * Set text content for the element.
		 * 
		 * @param text
		 *            Content text.
		 */
		public void setContentText(String text) {
			if (null != text) {
				sLogger.debug("setContentText: text = " + text);
				mContentText = text;
			} else {
				sLogger.debug("setContentText: text is null.");
			}
		}

		/**
		 * Get text content of the element.
		 * 
		 * @return Content text
		 */
		public String getContentText() {
			sLogger.debug("getContentText");
			return mContentText;
		}

		/**
		 * Set comment for the element.
		 * 
		 * @param comment
		 *            comment string.
		 */
		public void setComment(String comment) {
			if (null != comment) {
				sLogger.debug("setComment: comment = " + comment);
				mComment = comment;
			} else {
				sLogger.debug("setComment: comment is null.");
			}
		}

		/**
		 * Set comment for the element.
		 * 
		 * @return comment comment string.
		 */
		public String getComment() {
			sLogger.debug("getComment");
			return mComment;
		}

	}

	/**
	 * Create XML with SAX.
	 * 
	 * @param xmlOutputStream
	 *            The XML result.
	 * @param element
	 *            The root element to be used to create XML, which may have
	 *            child element.
	 */
	public static void saxCreateXml(OutputStream xmlOutputStream,
			ElementInfo element) {
		if (null == xmlOutputStream || null == element) {
			sLogger.debug("saxCreateXml: xmlOutputStream or element is null.");
		} else {
			sLogger.debug("saxCreateXml: the child elemnts size of the root element =  "
					+ element.getChildElements().size());
		}
		try {
			// SAX transformer factory
			SAXTransformerFactory sff = (SAXTransformerFactory) SAXTransformerFactory
					.newInstance();
			// Transformer handler
			TransformerHandler transformerHandler = sff.newTransformerHandler();
			// Transformer
			Transformer transformer = transformerHandler.getTransformer();
			// Encoding
			transformer.setOutputProperty(OutputKeys.ENCODING, ENCODING_UTF8);
			// Indent
			transformer.setOutputProperty(OutputKeys.INDENT, INDENT_YES);

			// Set the output for XML result.
			Result resultXml = new StreamResult(xmlOutputStream);
			transformerHandler.setResult(resultXml);

			transformerHandler.startDocument();
			AttributesImpl attributesImpl = new AttributesImpl();
			saxConvertToXML(element, transformerHandler, attributesImpl);
			transformerHandler.endDocument();
		} catch (TransformerConfigurationException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Convert the element to XML format with SAX.
	 * 
	 * @param element
	 *            The element will be converted.
	 * @param transformerHandler
	 *            It is used to create a XML element.
	 * @param attributesImpl
	 *            It is used to add attribute for the element.
	 * 
	 */
	private static void saxConvertToXML(ElementInfo element,
			TransformerHandler transformerHandler, AttributesImpl attributesImpl) {
		sLogger.debug("convertToXML");
		if (null == transformerHandler || null == attributesImpl) {
			sLogger.debug("convertToXML: parameter transformerHandler or attributesImpl is null.");
			return;
		}

		try {
			String comment = element.getComment();
			if (null != comment) {
				transformerHandler.comment(comment.toCharArray(), 0,
						comment.length());
			}

			Map<String, String> attributeMap = element.getAttributeMap();

			if (null != attributeMap) {
				attributesImpl.clear();
				Set<Entry<String, String>> entrySet = attributeMap.entrySet();
				String attrLocalName;
				String attrValue;
				for (Entry<String, String> entry : entrySet) {
					attrLocalName = entry.getKey();
					attrValue = entry.getValue();
					if (null != attrLocalName && !"".equals(attrLocalName)
							&& null != attrValue && !"".equals(attrValue)) {
						attributesImpl.addAttribute(element.getUri(),
								entry.getKey(), entry.getKey(), "",
								entry.getValue());
					}
				}
			}

			String localName = element.getLocalName();
			String qName = element.getQName();
			if (null != localName) {
				transformerHandler.startElement(element.getUri(), localName,
						element.getQName(), attributesImpl);
				String mCharacters = element.getContentText();
				if (null != element) {
					transformerHandler.characters(mCharacters.toCharArray(), 0,
							mCharacters.length());
				}

				List<ElementInfo> childElements = element.getChildElements();
				if (null != childElements) {
					for (ElementInfo childElement : childElements) {
						saxConvertToXML(childElement, transformerHandler,
								attributesImpl);
					}
				}

				transformerHandler.endElement(element.getUri(), localName,
						qName);
			}

		} catch (SAXException e) {
			e.printStackTrace();
		}

	}

	/**
	 * Parse XML with SAX.
	 * 
	 * @param xmlInputStream
	 *            The XML input stream that will be parsed.
	 * @param defaultHandler
	 *            A DefaultHandler object to handle parsing callback event.
	 */
	public static void saxParseXML(InputStream xmlInputStream,
			DefaultHandler defaultHandler) {
		sLogger.debug("saxParseXML");
		if (null == xmlInputStream || null == defaultHandler) {
			sLogger.debug("saxParseXML: Parameter xmlInputStream or defaultHandler is null.");
			return;
		}

		SAXParserFactory factory = SAXParserFactory.newInstance();
		SAXParser parser;
		try {
			parser = factory.newSAXParser();
			parser.parse(xmlInputStream, defaultHandler);
		} catch (ParserConfigurationException e) {
			e.printStackTrace();
		} catch (SAXException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Parse XML with Dom4j.
	 * 
	 * @param xmlInputStream
	 *            The XML input stream that will be parsed.
	 * 
	 */
	public static ElementInfo dom4jParseXML(InputStream xmlInputStream) {
		sLogger.debug("dom4jParseXML");

		ElementInfo rootElementInfo = null;

		if (null == xmlInputStream) {
			sLogger.debug("dom4jParseXML: Parameter xmlInputStream is null.");
			return null;
		}

		Document document = dom4jReadXMLToDocument(xmlInputStream, new SAXReader());
		Element rootElement = document.getRootElement();

		rootElementInfo = dom4jParseXML(rootElement);

		return rootElementInfo;
	}
	
	/**
	 * Read data from the inputstream and create a dom4j 'Document' object.
	 * @param xmlInputStream
	 * @param saxReader
	 * @return a dom4j 'Document' object
	 */
	public static Document dom4jReadXMLToDocument(InputStream xmlInputStream, SAXReader saxReader) {
		Document targetDocument = null;
		if (null == xmlInputStream || null == saxReader) {
			sLogger.debug("dom4jReadXMLToDocument: xmlInputStream or saxReader is null.");
			return targetDocument;
		}
		
		try {
			targetDocument = saxReader.read(xmlInputStream);
		} catch (DocumentException e) {
			e.printStackTrace();
		}
		
		return targetDocument;
	}
	
	/**
	 * Write the target XML document to the output stream.
	 * 
	 * @param xmlOutputStream
	 */
	public static void dom4jWriteDocumentToXML(OutputStream xmlOutputStream, Document sourceDocument) {
		if (null == xmlOutputStream || null == sourceDocument) {
			sLogger.debug("dom4jWriteDocumentToXML: xmlOutputStream or sourceDocument is null.");
			return;
		}

		try {
			XMLWriter xmlWriter = new XMLWriter(xmlOutputStream);
			xmlWriter.write(sourceDocument);
			xmlWriter.close();
		} catch (UnsupportedEncodingException e) {
			e.printStackTrace();
		} catch (IOException e) {
			e.printStackTrace();
		}
	}

	/**
	 * Parse XML with Dom4j.
	 * 
	 * @param xmlInputStream
	 *            The XML input stream that will be parsed.
	 * 
	 */
	private static ElementInfo dom4jParseXML(Element element) {
		sLogger.debug("dom4jParseXML");

		if (null == element) {
			sLogger.debug("dom4jParseXML: Parameter element is null.");
			return null;
		}

		ElementInfo elementInfo = new ElementInfo();
		setElementInfoWithDom4jElement(elementInfo, element);

		Iterator<Element> iterator = element.elementIterator();
		Element childElement;
		ElementInfo childElementInfo;

		while (iterator.hasNext()) {
			childElementInfo = new ElementInfo();
			childElement = iterator.next();
			childElementInfo = dom4jParseXML(childElement);
			elementInfo.addChildElement(childElementInfo);
		}

		return elementInfo;
	}

	/**
	 * 
	 * @param targetElementInfo
	 * @param dom4jElement
	 */
	private static void setElementInfoWithDom4jElement(
			ElementInfo targetElementInfo, Element dom4jElement) {
		sLogger.debug("setElementInfoWithDom4jElement");

		if (null == targetElementInfo || null == dom4jElement) {
			sLogger.debug("dom4jParseXML: Parameter targetElementInfo or dom4jElement is null.");
			return;
		}

		targetElementInfo.setLocalName(dom4jElement.getName());
		targetElementInfo.setQName(dom4jElement.getQualifiedName());
		targetElementInfo.setUri(dom4jElement.getNamespaceURI());
		targetElementInfo.setContentText(dom4jElement.getTextTrim());
		
		// set attribute
		List<Attribute> attributeList = dom4jElement.attributes();
		Map<String, String> attrbuteMap = new HashMap<String, String>();

		for (Attribute attribute : attributeList) {
			attrbuteMap.put(attribute.getName(), attribute.getValue());
		}
		
		targetElementInfo.setAttributeMap(attrbuteMap);

		// set namespace
		Map<String, String> namespaceMap = new HashMap<String, String>();
		
		// firstly, set the root element itself namespace.
		namespaceMap.put(dom4jElement.getNamespacePrefix(), dom4jElement.getNamespaceURI());
		
		// secondly, set additional namespace.
		List<Namespace> namespaceList = dom4jElement.additionalNamespaces();

		for (Namespace namespace : namespaceList) {
			namespaceMap.put(namespace.getPrefix(), namespace.getURI());
		}
		
		targetElementInfo.setNamespaceUriMap(namespaceMap);
	}
	
}

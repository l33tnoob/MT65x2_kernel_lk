package com.mtk.sanitytest.smstest;

import java.io.File;

import javax.xml.parsers.DocumentBuilder;
import javax.xml.parsers.DocumentBuilderFactory;

import org.w3c.dom.Document;
import org.w3c.dom.Element;
import org.w3c.dom.NamedNodeMap;
import org.w3c.dom.Node;
import org.w3c.dom.NodeList;

import android.util.Log;

public class xmlParser {
	private static final String tag = "Parser";
	// private DocumentBuilderFactory mDocBuilderFactory;
	// private DocumentBuilder mDocBuilder;
	private Document mDoc;
	private Element mRoot;
	int i;

	public xmlParser(String path) {
		int j =i;
		try {
			DocumentBuilderFactory mDocBuilderFactory = DocumentBuilderFactory
					.newInstance();
			mDocBuilderFactory.setValidating(false);
			mDocBuilderFactory.setNamespaceAware(true);
			DocumentBuilder mDocBuilder = mDocBuilderFactory
					.newDocumentBuilder();
			mDoc = mDocBuilder.parse(new File(path));
			mRoot = mDoc.getDocumentElement();
		} catch (Exception e) {
			e.printStackTrace();
		}
	}

	public Node getRootNode() {
		return mRoot;
	}

	public Node getNodeByName(Node ParentNode, String name) {
		if (ParentNode == null || name == null) {
			Log.d(tag, "Invalid input parameter");
			return null;
		}

		Node node = ParentNode.getFirstChild();
		while (node != null) {
			if (name.equals(node.getNodeName())) {
				return node;
			}
			node = node.getNextSibling();
		}
		return null;
	}

	public Node getNodeByIndex(Node ParentNode, int nIndex) {
		NodeList nodeList = ParentNode.getChildNodes();
		int nTemp = nodeList.getLength();
		if (nIndex < 0 || nIndex > nTemp || nTemp <= 0) {
			Log.d(tag, "Invalid input parameter");
			return null;
		}

		return nodeList.item(nIndex);
	}

	public String getNodeName(Node node) {
		if (null == node) {
			Log.d(tag, "Invalid input parameter");
			return null;
		}
		return node.getNodeName();
	}

	public String getNodeValue(Node node) {
		if (null == node) {
			Log.d(tag, "Invalid input parameter");
			return null;
		}

		NodeList list = node.getChildNodes();
		for (int i = 0; i < list.getLength(); i++) {
			Node subnode = list.item(i);
			if (subnode.getNodeType() == Node.TEXT_NODE
					&& !subnode.getNodeValue().equals("#text")) {
				return subnode.getNodeValue();
			}
		}
		return null;
	}

	public String getAttrValue(Node node, String AttrName) {
		if (node == null || AttrName == null) {
			Log.d(tag, "Invalid input parameter");
			return null;
		}

		NamedNodeMap attributes = node.getAttributes();
		if (attributes == null)
			return null;

		return attributes.getNamedItem(AttrName).getNodeValue();
	}

	public int getLegth(Node node) {
		if (null == node) {
			Log.d(tag, "Invalid input parameter");
			return 0;
		}

		NodeList nodeList = node.getChildNodes();
		return nodeList.getLength();
	}
}

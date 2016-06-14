#include <libxml/xmlreader.h>
#include <string>
#include <iostream>
#include <vector>
#include <functional>

using XMLString = xmlChar*;
using NodeVisitor = std::function<void(xmlTextReaderPtr, std::vector<XMLString>)>;

const NodeVisitor identicalNodeVisitor = [](
    xmlTextReaderPtr,
    std::vector<XMLString>) {
};

struct XMLVisitor {
  NodeVisitor readNoneNode;
  NodeVisitor readElementNode;
  NodeVisitor readAttributeNode;
  NodeVisitor readTextNode;
  NodeVisitor readCDataNode;
  NodeVisitor readEntityReferenceNode;
  NodeVisitor readEntityNode;
  NodeVisitor readProcessingInstructionNode;
  NodeVisitor readCommentNode;
  NodeVisitor readDocumentNode;
  NodeVisitor readDocumentTypeNode;
  NodeVisitor readDocumentFragmentNode;
  NodeVisitor readNotationNode;
  NodeVisitor readWhiteSpaceNode;
  NodeVisitor readSignificantWhitespaceNode;
  NodeVisitor readEndElementNode;
  NodeVisitor readEndEntityNode;
  NodeVisitor readXmlDeclarationNode;
  NodeVisitor readDefaultNode;
};

const XMLVisitor identicalXMLVisitor = {
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor,
  identicalNodeVisitor
};

void ReadXML(xmlTextReaderPtr reader, XMLVisitor visitor) {
  std::vector<XMLString> path;
  while (xmlTextReaderRead(reader) == 1) {
    xmlReaderTypes nodeType = static_cast<xmlReaderTypes>(xmlTextReaderNodeType(reader));
    XMLString name = xmlTextReaderName(reader);
    switch (nodeType) {
      case XML_READER_TYPE_NONE:
        visitor.readNoneNode(reader, path);
        break;
      case XML_READER_TYPE_ELEMENT:
        path.push_back(name);
        visitor.readElementNode(reader, path);
        break;
      case XML_READER_TYPE_ATTRIBUTE:
        visitor.readAttributeNode(reader, path);
        break;
      case XML_READER_TYPE_TEXT:
        visitor.readTextNode(reader, path);
        break;
      case XML_READER_TYPE_CDATA:
        visitor.readCDataNode(reader, path);
        break;
      case XML_READER_TYPE_ENTITY_REFERENCE:
        visitor.readEntityReferenceNode(reader, path);
        break;
      case XML_READER_TYPE_ENTITY:
        visitor.readEntityNode(reader, path);
        break;
      case XML_READER_TYPE_PROCESSING_INSTRUCTION:
        visitor.readProcessingInstructionNode(reader, path);
        break;
      case XML_READER_TYPE_COMMENT:
        visitor.readCommentNode(reader, path);
        break;
      case XML_READER_TYPE_DOCUMENT:
        visitor.readDocumentNode(reader, path);
        break;
      case XML_READER_TYPE_DOCUMENT_TYPE:
        visitor.readDocumentTypeNode(reader, path);
        break;
      case XML_READER_TYPE_DOCUMENT_FRAGMENT:
        visitor.readDocumentFragmentNode(reader, path);
        break;
      case XML_READER_TYPE_NOTATION:
        visitor.readNotationNode(reader, path);
        break;
      case XML_READER_TYPE_WHITESPACE:
        visitor.readWhiteSpaceNode(reader, path);
        break;
      case XML_READER_TYPE_SIGNIFICANT_WHITESPACE:
        visitor.readSignificantWhitespaceNode(reader, path);
        break;
      case XML_READER_TYPE_END_ELEMENT:
        visitor.readEndElementNode(reader, path);
        path.pop_back();
        break;
      case XML_READER_TYPE_END_ENTITY:
        visitor.readEndEntityNode(reader, path);
        break;
      case XML_READER_TYPE_XML_DECLARATION:
        visitor.readXmlDeclarationNode(reader, path);
        break;
    }
  }
}

void processNode(xmlTextReaderPtr reader);

int main(int argc, char **argv) {
  std::string filename(argv[1]);
  xmlTextReaderPtr reader = xmlNewTextReaderFilename(filename.c_str());
  processNode(reader);
  xmlFreeTextReader(reader);
  return 0;
}

std::vector<xmlChar*> path;

void processNode(xmlTextReaderPtr reader) {
  XMLVisitor visitor = identicalXMLVisitor;
  visitor.readTextNode = [](xmlTextReaderPtr reader, std::vector<XMLString> path) {
    xmlChar* value = xmlTextReaderValue(reader);
    if (!value) {
      value = xmlStrdup(BAD_CAST "---");
    }
    for (int i = 1, limit = path.size(); i < limit; ++i) {
      std::cout << '.';
    }
    std::cout << value << std::endl;
  };
  ReadXML(reader, visitor);
}

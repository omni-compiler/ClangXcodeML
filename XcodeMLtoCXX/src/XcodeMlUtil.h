#ifndef XCODEMLUTIL_H
#define XCODEMLUTIL_H

namespace XcodeMl {
class Function;
class UnqualId;
}

class SourceInfo;

std::shared_ptr<XcodeMl::UnqualId> getUnqualIdFromNameNode(xmlNodePtr idNode);

std::shared_ptr<XcodeMl::UnqualId> getUnqualIdFromIdNode(
    xmlNodePtr nameNode, xmlXPathContextPtr ctxt);

XcodeMl::Name getQualifiedName(xmlNodePtr node, SourceInfo &);

std::string getType(xmlNodePtr node);

void xcodeMlPwd(xmlNodePtr, std::ostream &);

struct XcodeMlPwdType {
  xmlNodePtr node;
};

XcodeMlPwdType getXcodeMlPath(xmlNodePtr);

std::vector<XcodeMl::CodeFragment> getParamNames(
    xmlNodePtr fnNode, const SourceInfo &src);

XcodeMl::CodeFragment makeFunctionDeclHead(XcodeMl::Function *func,
    const XcodeMl::Name &name,
    const std::vector<XcodeMl::CodeFragment> &paramNames,
    const SourceInfo &src,
    bool emitNameSpec = false);

XcodeMl::CodeFragment makeFunctionDeclHead(xmlNodePtr node,
    const std::vector<XcodeMl::CodeFragment> paramNames,
    const SourceInfo &src,
    bool emitNameSpec = false);

XcodeMl::CodeFragment wrapWithLangLink(const XcodeMl::CodeFragment &content,
    xmlNodePtr node,
    const SourceInfo &src);

std::ostream &operator<<(std::ostream &, const XcodeMlPwdType &);

#endif /* !XCODEMLUTIL_H */

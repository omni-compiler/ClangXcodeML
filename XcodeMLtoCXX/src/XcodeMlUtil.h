#ifndef XCODEMLUTIL_H
#define XCODEMLUTIL_H

namespace XcodeMl {
class UnqualId;
}

class SourceInfo;

std::shared_ptr<XcodeMl::UnqualId> getUnqualIdFromNameNode(xmlNodePtr idNode);

std::shared_ptr<XcodeMl::UnqualId> getUnqualIdFromIdNode(
    xmlNodePtr nameNode, xmlXPathContextPtr ctxt);

std::shared_ptr<XcodeMl::Nns> getNns(
    const XcodeMl::NnsMap &nnsTable, xmlNodePtr nameNode);

XcodeMl::Name getQualifiedNameFromNameNode(
    xmlNodePtr nameNode, const SourceInfo &);

#endif /* !XCODEMLUTIL_H */

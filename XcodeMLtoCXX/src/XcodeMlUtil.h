#ifndef XCODEMLUTIL_H
#define XCODEMLUTIL_H

namespace XcodeMl {
class UnqualId;
}

std::shared_ptr<XcodeMl::UnqualId> getUnqualIdFromNameNode(xmlNodePtr idNode);

std::shared_ptr<XcodeMl::UnqualId> getUnqualIdFromIdNode(
    xmlNodePtr nameNode, xmlXPathContextPtr ctxt);

#endif /* !XCODEMLUTIL_H */

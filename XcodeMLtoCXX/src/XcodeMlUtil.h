#ifndef XCODEMLUTIL_H
#define XCODEMLUTIL_H

namespace XcodeMl {
class UnqualId;
}

std::shared_ptr<XcodeMl::UnqualId> getNameFromNameNode(xmlNodePtr idNode);

std::shared_ptr<XcodeMl::UnqualId> getNameFromIdNode(
    xmlNodePtr nameNode, xmlXPathContextPtr ctxt);

#endif /* !XCODEMLUTIL_H */

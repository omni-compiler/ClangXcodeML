#ifndef NNSANALYZER_H
#define NNSANALYZER_H

class SourceInfo;

XcodeMl::NnsMap analyzeNnsTable(xmlNodePtr, xmlXPathContextPtr);

XcodeMl::NnsMap expandNnsMap(const XcodeMl::NnsMap &table,
    xmlNodePtr nnsTableNode,
    xmlXPathContextPtr ctxt);

#endif /* !NNSANALYZER_H */

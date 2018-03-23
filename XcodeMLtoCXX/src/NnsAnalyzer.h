#ifndef NNSANALYZER_H
#define NNSANALYZER_H

class SourceInfo;

XcodeMl::NnsTable analyzeNnsTable(xmlNodePtr, xmlXPathContextPtr);

XcodeMl::NnsTable expandNnsTable(const XcodeMl::NnsTable &table,
    xmlNodePtr nnsTableNode,
    xmlXPathContextPtr ctxt);

#endif /* !NNSANALYZER_H */

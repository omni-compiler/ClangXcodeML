#ifndef TYPEANALYZER_H
#define TYPEANALYZER_H

XcodeMl::TypeTable parseTypeTable(
    xmlNodePtr, xmlXPathContextPtr, std::stringstream &);

XcodeMl::TypeTable expandTypeTable(const XcodeMl::TypeTable &env,
    xmlNodePtr typeTable,
    xmlXPathContextPtr ctxt);

#endif /* !TYPEANALYZER_H */

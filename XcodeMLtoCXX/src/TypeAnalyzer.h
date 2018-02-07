#ifndef TYPEANALYZER_H
#define TYPEANALYZER_H

XcodeMl::Environment parseTypeTable(
    xmlNodePtr, xmlXPathContextPtr, std::stringstream &);

XcodeMl::Environment expandEnvironment(const XcodeMl::Environment &env,
    xmlNodePtr typeTable,
    xmlXPathContextPtr ctxt);

#endif /* !TYPEANALYZER_H */

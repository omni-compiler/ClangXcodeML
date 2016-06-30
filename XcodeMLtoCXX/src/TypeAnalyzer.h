#ifndef TYPEANALYZER_H
#define TYPEANALYZER_H

using TypeAnalyzer = XMLWalker<XcodeMl::TypeMap&>;
XcodeMl::TypeMap parseTypeTable(xmlDocPtr doc);

#endif /* !TYPEANALYZER_H */

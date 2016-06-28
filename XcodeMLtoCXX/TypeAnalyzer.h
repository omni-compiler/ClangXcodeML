#ifndef TYPEANALYZER_H
#define TYPEANALYZER_H

using TypeAnalyzer = Reality<XcodeMl::TypeMap&>;
XcodeMl::TypeMap parseTypeTable(xmlDocPtr doc);

#endif /* !TYPEANALYZER_H */

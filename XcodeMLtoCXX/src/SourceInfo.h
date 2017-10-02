#ifndef SOURCEINFO_H
#define SOURCEINFO_H

/*!
 * \brief A pack of necessary information for generating
 * C++ source code.
 */
class SourceInfo {
public:
  xmlXPathContextPtr ctxt;
  XcodeMl::Environment typeTable;
  XcodeMl::NnsMap nnsTable;
};

#endif /* !SOURCEINFO_H */

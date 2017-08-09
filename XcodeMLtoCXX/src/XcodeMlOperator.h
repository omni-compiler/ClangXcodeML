#ifndef XCODEMLOPERATOR_H
#define XCODEMLOPERATOR_H

namespace XcodeMl {

llvm::Optional<std::string> OperatorNameToSpelling(const std::string&);

XcodeMl::CodeFragment makeOpNode(xmlNodePtr);

} // namespace XcodeMl

#endif /* !XCODEMLOPERATOR_H */

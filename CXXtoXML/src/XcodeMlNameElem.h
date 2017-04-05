#ifndef XCODEMLNAMEELEM_H
#define XCODEMLNAMEELEM_H

xmlNodePtr makeNameNode(TypeTableInfo&, const NamedDecl*);
xmlNodePtr makeNameNodeForCXXMethodDecl(TypeTableInfo&, const CXXMethodDecl*);
xmlNodePtr makeIdNodeForCXXMethodDecl(TypeTableInfo&, const CXXMethodDecl*);

#endif /*! XCODEMLNAMEELEM_H */

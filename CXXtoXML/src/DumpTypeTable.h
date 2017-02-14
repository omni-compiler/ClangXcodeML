class TypeTableDumpASTConsumer : public clang::ASTConsumer {
public:
  explicit TypeTableDumpASTConsumer(xmlNodePtr N) : dummyRoot(N) {};
  virtual void HandleTranslationUnit(clang::ASTContext &) override;
private:
  xmlNodePtr dummyRoot;
};

class TypeTableDumpAction : public clang::ASTFrontendAction {
public:
  bool BeginSourceFileAction(clang::CompilerInstance&, llvm::StringRef) override;

  virtual std::unique_ptr<clang::ASTConsumer>
  CreateASTConsumer(clang::CompilerInstance &, llvm::StringRef) override;

  void EndSourceFileAction() override;
private:
  xmlNodePtr dummyRoot;
};

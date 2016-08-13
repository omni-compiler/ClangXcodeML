namespace XcodeMl {
  struct Environment {
    std::map<DataTypeIdent, TypeRef> typemap;
  };

  TypeRef get(const Environment& env, const DataTypeIdent& type);
}

class A {};
class B {};

class D : A {};
class PD1 : public A {};
class PD2 : private A {};
class PD3 : protected A {};
class MD1 : A, B {};
class MD2 : public PD1, private PD2, protected PD3 {};
class VD1 : virtual A {};
class VD2 : virtual public A {};
class VD3 : virtual private A {};
class VD4 : virtual protected A {};

class CD : VD1, public VD2, virtual protected D, MD1 {};

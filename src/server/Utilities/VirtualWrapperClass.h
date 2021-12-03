#ifndef _VIRTUALWRAPPERCLASS_H
#define _VIRTUALWRAPPERCLASS_H
//virtual_pointer_wrapper_interface_only(Interface): that is that this class wraps a pointer only
//Derived classes must use virtual_pointer_wrapper_class_only(Derived)
//and must only have one foreign pointer to wrap, NEVER_OURS
//example:
//  virtual_pointer_wrapper_interface_only(IXmlXPathFunctionContext) {};
//
//  virtual_pointer_wrapper_class_only(LibXmlXPathFunctionContext), public IXmlXPathFunctionContext {
//    xmlXPathParserContext *m_pXCTxt; //singular pointer allowed
//    <functions...>
//  };
//  on instanciation, the base VirtualPointerWrapperClass() will check the validity of the wrapper

#include <exception>
using namespace std;

#define virtual_pointer_wrapper_interface_only(Interface) class Interface: \
  virtual private VirtualPointerWrapperClassChecker

#define virtual_pointer_wrapper_class_only(Class) class Class: \
  private VirtualPointerWrapperClass<Class>, \
  private VirtualPointerWrapperClassConfirmation

#define _VTABLE_SIZE_BYTES 4
#define _SINGULAR_POINTER_SIZE_BYTES 4
#define _VIRTUALPOINTERWRAPPERCLASS_SIZE _VTABLE_SIZE_BYTES + _SINGULAR_POINTER_SIZE_BYTES

//--------------------------------------- exceptions
class VirtualPointerWrapperClassHasMembers: public exception {
  const size_t m_uSize;
public:
  VirtualPointerWrapperClassHasMembers(const size_t uSize): m_uSize(uSize) {}
  const char *what() const throw() {return "VirtualPointerWrapperClassHasMembers: it is only allowed one pointer to wrap and should not conditionally destroy it.";}
};

//--------------------------------------- virtual check system
class VirtualPointerWrapperClassChecker {
  //TODO: IFDEBUG(virtual void check(VirtualPointerWrapperClassChecker *p) = 0;)
};
class VirtualPointerWrapperClassConfirmation {
  IFDEBUG(void check(VirtualPointerWrapperClassChecker *p) {})
};

//--------------------------------------- actual assert size checking class
template<class Item> class VirtualPointerWrapperClass {
protected:
  IFDEBUG(
    VirtualPointerWrapperClass() {if (sizeof(Item) != _VIRTUALPOINTERWRAPPERCLASS_SIZE) throw VirtualPointerWrapperClassHasMembers(this, sizeof(Item));}
  )
  virtual ~VirtualPointerWrapperClass() {} //force virtuality (4 byte vtable)
};

#endif

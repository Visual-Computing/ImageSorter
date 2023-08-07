#if !defined(PX_REF_H)
#define PX_REF_H
#include "pxdefs.h"
namespace isortisearch{
/*!
some helper template functions

ref() returns the underlying value (read/write access), wether a value, a ptr to a value or a refPtr<value> is passed
*/
template <typename T> T& ref(T& t) {return t;}
template <typename T> T& ref(T*& t) {return *t;}
template <typename T> T& ref(refPtr<T>& t) {return *t;}

/*!
some helper template functions

const_ref() returns the underlying value (read access), wether a value, a ptr to a value or a refPtr<value> is passed
*/
template <typename T> const T& const_ref(T& t) {return t;}
template <typename T> const T& const_ref(T*& t) {return *t;}
template <typename T> const T& const_ref(refPtr<T>& t) {return *t;}
}
#endif
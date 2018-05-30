#ifndef __BOXEDPTR_H__
#define __BOXEDPTR_H__

class BoxedPtrBase {
public:        
    BoxedPtrBase() : _retainCount(0) {}
    virtual ~BoxedPtrBase() {}
 
    void retain() { 
        _retainCount++; 
    }
    void release() {        
        _retainCount--;
        if (_retainCount == 0) 
            delete this;
    }
    int retainCount() {return this->_retainCount;}
private:
    int _retainCount;
};

template <typename T> 
class BoxedPtr {    
public: 
    explicit BoxedPtr() : ptr(0) {};
    BoxedPtr(T *p) : ptr(p) { 
        if (ptr) 
            ptr->retain(); 
    }
    BoxedPtr(const BoxedPtr &p) : ptr(p.ptr) { 
        if (ptr) 
            ptr->retain(); 
    }
    BoxedPtr(BoxedPtr &&p) : ptr(p.ptr) { p.ptr = 0; }
   
    template <class X>
    BoxedPtr(BoxedPtr<X> &&p) : ptr(p.get()) {
        p.ptr = 0;
    }
 
    template <class X>
    BoxedPtr(const BoxedPtr<X> &p) : ptr(p.get()) {
        if (ptr)
            ptr->retain();
    }

   ~BoxedPtr() { if (ptr) ptr->release(); }
 
    BoxedPtr &operator=(BoxedPtr S) {
        swap(S);
        return *this;
    }
    BoxedPtr &operator=(T* p) {
        if (ptr)
            ptr->release();
        ptr = p;
        if (ptr)
            ptr->retain();
        return *this;
    }
    T &operator*() const { return *ptr; }
    T *operator->() const { return ptr; }
    T *get() const { return ptr; }
    explicit operator bool() const { return ptr!=0; }
 
    void swap(BoxedPtr &other) {
        T *tmp = other.ptr;
        other.ptr = ptr;
        ptr = tmp;
    }
private:
    T *ptr;
 };
 
template <class T, class U>
 inline bool operator==(const BoxedPtr<T> &A,
                        const BoxedPtr<U> &B) {
   return A.get() == B.get();
 }
 
 template <class T, class U>
 inline bool operator!=(const BoxedPtr<T> &A,
                        const BoxedPtr<U> &B) {
   return A.get() != B.get();
 }
 
 template <class T, class U>
 inline bool operator==(const BoxedPtr<T> &A, U *B) {
   return A.get() == B;
 }
 
 template <class T, class U>
 inline bool operator!=(const BoxedPtr<T> &A, U *B) {
   return A.get() != B;
 }
 
 template <class T, class U>
 inline bool operator==(T *A, const BoxedPtr<U> &B) {
   return A == B.get();
 }
 
 template <class T, class U>
 inline bool operator!=(T *A, const BoxedPtr<U> &B) {
   return A != B.get();
 }
 
 template <class T>
 bool operator==(std::nullptr_t A, const BoxedPtr<T> &B) {
   return !B;
 }
 
 template <class T>
 bool operator==(const BoxedPtr<T> &A, std::nullptr_t B) {
   return B == A;
 }
 
 template <class T>
 bool operator!=(std::nullptr_t A, const BoxedPtr<T> &B) {
   return !(A == B);
 }
 
 template <class T>
 bool operator!=(const BoxedPtr<T> &A, std::nullptr_t B) {
   return !(A == B);
 }

#endif
#ifndef VFS_REFCOUNTED_H
#define VFS_REFCOUNTED_H

#include "VFSInternal.h"
#include <algorithm>

VFS_NAMESPACE_START


template <typename T> class RefcountedT
{
public:
    // Static methods for refcounting, could overload with atomics if there is the need
    inline static void s_incRef(T& ref)
    {
        return ++ref;
    }
    inline static bool s_decRef(T& ref) // returns true if refcount is zero afterwards
    {
        return (--ref) == 0;
    }
    inline static bool s_setRef(T& ref, int val)
    {
        ref = val;
    }
    inline static int s_getRef(T& ref)
    {
        return ref;
    }

    RefcountedT()
    {
        s_setRef(_refcount, 0);
    }

    virtual ~RefcountedT()
    {
        int val = s_getRef(_refcount);
        assert(val == 0, "Object was deleted with refcount ", val);
    }

    inline void incref()
    {
        s_incRef(refcount);
    }
    inline void decref()
    {
        if (s_decRef(&_refcount))
        {
            // if the refcount is now zero, it will stay zero forever as nobody has a reference anymore
            delete this;
        }
    }

private:
    T _refcount;
};

// This is the typedef used for VFSBase
typedef RefcountedT<int> Refcounted;


template<typename T> class CountedPtr
{
public:
    inline ~CountedPtr()
    {
        if(_p)
            _p->decref();
    }
    inline CountedPtr() : _p(nullptr)
    {}
    inline CountedPtr(T* p) : _p(p)
    {
        if(p)
            p->incref();
    }
    inline CountedPtr(const CountedPtr& ref) : _p(ref._p)
    {
        if (_p)
            _p->incref();
    }

#ifdef HAVE_CXX_11
    // C++11 move constructor
    CountedPtr(CountedPtr&& ref) : CountedPtr() // initialize via default constructor
    {
        CountedPtr::swap(*this, ref);
    }
#endif

    // intentionally not a reference -- see http://stackoverflow.com/questions/3279543/what-is-the-copy-and-swap-idiom
    CountedPtr& operator=(CountedPtr ref)
    {
        CountedPtr::swap(*this, ref);
        return *this;
    }

    const T* operator->() const  { return _p; }
          T* operator->()        { return _p; }

    bool operator!() const { return !_p; }

    // Safe for use in if statements
    operator bool() const  { return _p != nullptr; }

    // if you use these, make sure you also keep a counted reference to the object!
    inline operator       T* ()       { return _p; }
    inline operator const T* () const { return _p; }

    bool operator<(const CountedPtr& ref) const { return _p < ref._p; }
    bool operator<=(const CountedPtr& ref) const { return _p <= ref._p; }
    bool operator==(const CountedPtr& ref) const { return _p == ref._p; }
    bool operator!=(const CountedPtr& ref) const { return _p != ref._p; }
    bool operator>=(const CountedPtr& ref) const { return _p >= ref._p; }
    bool operator>(const CountedPtr& ref) const { return _p > ref._p; }

    inline static void swap(CountedPtr& a, CountedPtr& b)
    {
        std::swap(a._p, b._p);
    }

private:

    T *_p;
};

VFS_NAMESPACE_END

#endif

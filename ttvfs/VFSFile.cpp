// VFSFile.cpp - basic file interface + classes
// For conditions of distribution and use, see copyright notice in VFS.h

#include "VFSInternal.h"
#include "VFSFile.h"
#include "VFSTools.h"

VFS_NAMESPACE_START

VFSFile::VFSFile(const char *name)
: _buf(NULL), _delfunc(NULL)
{
    _setName(name);
}

VFSFile::~VFSFile()
{
    close();
    dropBuf(true);
}

void VFSFile::delBuf(const void *mem) const
{
    if(mem)
    {
        if(_delfunc)
            _delfunc(_buf);
        else
            delete [] (char*)_buf;
    }
}

const void *VFSFile::getBuf(allocator_func alloc /* = NULL */, delete_func del /* = NULL */)
{
    assert(!alloc == !del); // either both or none may be defined. Checked extra early to prevent possible errors later.

    VFS_GUARD_OPT(this);
    if(_buf)
        return _buf;

    bool op = isopen();

    if(!op && !open()) // open with default params if not open
        return NULL;

    unsigned int s = (unsigned int)size();
    _buf = alloc ? alloc(s + 4) : (void*)(new char[s + 4]); // a bit extra padding
    if(!_buf)
        return NULL;

    _delfunc = del;

    vfspos offs;
    if(op)
    {
        vfspos oldpos = getpos();
        seek(0);
        offs = read(_buf, s);
        seek(oldpos);
    }
    else
    {
        offs = read(_buf, s);
        close();
    }
    memset((char*)_buf + offs, 0, 4);

    return _buf;
}

void VFSFile::dropBuf(bool del)
{
    VFS_GUARD_OPT(this);
    if(del)
        delBuf(_buf);
    _buf = NULL;
}

VFSFileReal::VFSFileReal(const char *name /* = NULL */)
: VFSFile(name), _fh(NULL), _size(npos), _buf(NULL)
{
}

VFSFileReal::~VFSFileReal()
{
}

bool VFSFileReal::open(const char *mode /* = NULL */)
{
    VFS_GUARD_OPT(this);

    if(isopen())
        close();

    dropBuf(true);

    _fh = fopen(fullname(), mode ? mode : "rb");
    if(!_fh)
        return false;

    fseek((FILE*)_fh, 0, SEEK_END);
    _size = getpos();
    fseek((FILE*)_fh, 0, SEEK_SET);

    return true;
}

bool VFSFileReal::isopen(void) const
{
    VFS_GUARD_OPT(this);
    return !!_fh;
}

bool VFSFileReal::iseof(void) const
{
    VFS_GUARD_OPT(this);
    return !_fh || feof((FILE*)_fh);
}

bool VFSFileReal::close(void)
{
    VFS_GUARD_OPT(this);
    if(_fh)
    {
        fclose((FILE*)_fh);
        _fh = NULL;
    }
    return true;    
}

bool VFSFileReal::seek(vfspos pos)
{
    VFS_GUARD_OPT(this);
    if(!_fh)
        return false;
    fseek((FILE*)_fh, pos, SEEK_SET);
    return true;
}

bool VFSFileReal::seekRel(vfspos offs)
{
    VFS_GUARD_OPT(this);
    if(!_fh)
        return false;
    fseek((FILE*)_fh, offs, SEEK_CUR);
    return true;
}

bool VFSFileReal::flush(void)
{
    VFS_GUARD_OPT(this);
    if(!_fh)
        return false;
    fflush((FILE*)_fh);
    return true;
}

vfspos VFSFileReal::getpos(void) const
{
    VFS_GUARD_OPT(this);
    if(!_fh)
        return npos;
    return ftell((FILE*)_fh);
}

unsigned int VFSFileReal::read(void *dst, unsigned int bytes)
{
    VFS_GUARD_OPT(this);
    if(!_fh)
        return npos;
    return fread(dst, 1, bytes, (FILE*)_fh);
}

unsigned int VFSFileReal::write(const void *src, unsigned int bytes)
{
    VFS_GUARD_OPT(this);
    if(!_fh)
        return npos;
    return fwrite(src, 1, bytes, (FILE*)_fh);
}

vfspos VFSFileReal::size(void)
{
    VFS_GUARD_OPT(this);
    if(_size != npos)
        return _size;
    open();
    close();
    // now size is known.
    return _size;
}

// ------------- VFSFileMem -----------------------

VFSFileMem::VFSFileMem(const char *name, void *buf, unsigned int size, Mode mode /* = COPY */,
                       allocator_func alloc /* = NULL */, delete_func delfunc /* = NULL */)
: VFSFile(name), _pos(0), _size(size), _mybuf(mode == TAKE_OVER || mode == COPY)
{
    if(mode == COPY)
    {
        assert(!alloc == !delfunc);
        _buf = alloc ? alloc(size+1) : (void*)(new char[size+1]);
        memcpy(_buf, buf, size);
        ((char*)_buf)[size] = 0;
    }
    else
    {
        _buf = buf;
    }
    _delfunc = delfunc;
}

VFSFileMem::~VFSFileMem()
{
    if(_mybuf)
        VFSFile::dropBuf(true);
}

unsigned int VFSFileMem::read(void *dst, unsigned int bytes)
{
    VFS_GUARD_OPT(this);
    if(iseof())
        return 0;
    unsigned int rem = std::min<unsigned int>((unsigned int)(_size - _pos), bytes);

    memcpy(dst, (char*)_buf + _pos, rem);
    return rem;
}

unsigned int VFSFileMem::write(const void *src, unsigned int bytes)
{
    VFS_GUARD_OPT(this);
    if(iseof())
        return 0;
    unsigned int rem = std::min<unsigned int>((unsigned int)(_size - _pos), bytes);

    memcpy((char*)_buf + _pos, src, rem);
    return rem;
}

VFS_NAMESPACE_END

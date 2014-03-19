// VFSFile.cpp - basic file interface + classes
// For conditions of distribution and use, see copyright notice in VFS.h

#include "VFSInternal.h"
#include "VFSFile.h"
#include "VFSTools.h"
#include "VFSFileFuncs.h"

#include <cstdio>

VFS_NAMESPACE_START

VFSFile::VFSFile(const char *name)
{
    _setName(name);
}

VFSFile::~VFSFile()
{
    close();
}

VFSFileReal::VFSFileReal(const char *name /* = NULL */)
: VFSFile(name), _fh(NULL), _buf(NULL)
{
}

VFSFileReal::~VFSFileReal()
{
}

bool VFSFileReal::open(const char *mode /* = NULL */)
{
    if(isopen())
        close();

    _fh = real_fopen(fullname(), mode ? mode : "rb");

    return !!_fh;
}

bool VFSFileReal::isopen(void) const
{
    return !!_fh;
}

bool VFSFileReal::iseof(void) const
{
    return !_fh || real_feof((FILE*)_fh);
}

bool VFSFileReal::close(void)
{
    if(_fh)
    {
        real_fclose((FILE*)_fh);
        _fh = NULL;
    }
    return true;
}

bool VFSFileReal::seek(vfspos pos)
{
    if(!_fh)
        return false;
    return real_fseek((FILE*)_fh, pos, SEEK_SET) == 0;
}

bool VFSFileReal::seekRel(vfspos offs)
{
    if(!_fh)
        return false;
    return real_fseek((FILE*)_fh, offs, SEEK_CUR) == 0;
}

bool VFSFileReal::flush(void)
{
    if(!_fh)
        return false;
    return real_fflush((FILE*)_fh) == 0;
}

vfspos VFSFileReal::getpos(void) const
{
    if(!_fh)
        return npos;
    return real_ftell((FILE*)_fh);
}

unsigned int VFSFileReal::read(void *dst, unsigned int bytes)
{
    if(!_fh)
        return npos;
    return real_fread(dst, 1, bytes, (FILE*)_fh);
}

unsigned int VFSFileReal::write(const void *src, unsigned int bytes)
{
    if(!_fh)
        return npos;
    return real_fwrite(src, 1, bytes, (FILE*)_fh);
}

vfspos VFSFileReal::size(void)
{
    return GetFileSize(fullname());
}

// ------------- VFSFileMem -----------------------

VFSFileMem::VFSFileMem(const char *name, void *buf, unsigned int size, delete_func delfunc /* = NULL */)
: VFSFile(name), _pos(0), _size(size), _buf(buf), _delfunc(delfunc)
{
}

VFSFileMem::~VFSFileMem()
{
    if(_delfunc)
        _delfunc(_buf);
}

unsigned int VFSFileMem::read(void *dst, unsigned int bytes)
{
    if(iseof())
        return 0;
    unsigned int rem = std::min<unsigned int>((unsigned int)(_size - _pos), bytes);

    memcpy(dst, (char*)_buf + _pos, rem);
    return rem;
}

unsigned int VFSFileMem::write(const void *src, unsigned int bytes)
{
    if(iseof())
        return 0;
    unsigned int rem = std::min<unsigned int>((unsigned int)(_size - _pos), bytes);

    memcpy((char*)_buf + _pos, src, rem);
    return rem;
}

VFS_NAMESPACE_END

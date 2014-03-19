// VFSFile.cpp - basic file interface + classes
// For conditions of distribution and use, see copyright notice in VFS.h

#include "VFSInternal.h"
#include "VFSFile.h"
#include "VFSTools.h"
#include "VFSFileFuncs.h"

#include <cstdio>

VFS_NAMESPACE_START

File::File(const char *name)
{
    _setName(name);
}

File::~File()
{
    close();
}

DiskFile::DiskFile(const char *name /* = NULL */)
: File(name), _fh(NULL), _buf(NULL)
{
}

DiskFile::~DiskFile()
{
}

bool DiskFile::open(const char *mode /* = NULL */)
{
    if(isopen())
        close();

    _fh = real_fopen(fullname(), mode ? mode : "rb");

    return !!_fh;
}

bool DiskFile::isopen(void) const
{
    return !!_fh;
}

bool DiskFile::iseof(void) const
{
    return !_fh || real_feof((FILE*)_fh);
}

bool DiskFile::close(void)
{
    if(_fh)
    {
        real_fclose((FILE*)_fh);
        _fh = NULL;
    }
    return true;
}

bool DiskFile::seek(vfspos pos)
{
    if(!_fh)
        return false;
    return real_fseek((FILE*)_fh, pos, SEEK_SET) == 0;
}

bool DiskFile::seekRel(vfspos offs)
{
    if(!_fh)
        return false;
    return real_fseek((FILE*)_fh, offs, SEEK_CUR) == 0;
}

bool DiskFile::flush(void)
{
    if(!_fh)
        return false;
    return real_fflush((FILE*)_fh) == 0;
}

vfspos DiskFile::getpos(void) const
{
    if(!_fh)
        return npos;
    return real_ftell((FILE*)_fh);
}

unsigned int DiskFile::read(void *dst, unsigned int bytes)
{
    if(!_fh)
        return npos;
    return real_fread(dst, 1, bytes, (FILE*)_fh);
}

unsigned int DiskFile::write(const void *src, unsigned int bytes)
{
    if(!_fh)
        return npos;
    return real_fwrite(src, 1, bytes, (FILE*)_fh);
}

vfspos DiskFile::size(void)
{
    return GetFileSize(fullname());
}

// ------------- VFSFileMem -----------------------

VFSFileMem::VFSFileMem(const char *name, void *buf, unsigned int size, delete_func delfunc /* = NULL */)
: File(name), _pos(0), _size(size), _buf(buf), _delfunc(delfunc)
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

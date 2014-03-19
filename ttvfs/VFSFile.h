// VFSFile.h - basic file interface + classes
// For conditions of distribution and use, see copyright notice in VFS.h

#ifndef VFSFILE_H
#define VFSFILE_H

#include "VFSBase.h"
#include <string>

VFS_NAMESPACE_START


/** -- VFSFile basic interface --
  * All functions that return bool should return true on success and false on failure.
  * If an operation is not necessary or irrelevant (for example, files in memory can't be closed),
  *    it is useful to return true anyways, because this operation did not fail, technically.
  *    (Common sense here!)
  * An int/vfspos value of 0 indicates failure, except the size/seek/getpos functions, where npos means failure.
  * Only the functions required or applicable need to be implemented, for unsupported operations
  *    the default implementation should be sufficient.
  **/
class VFSFile : public VFSBase
{
public:

    /** The ctor is expected to set both name() and fullname();
        The name must remain static throughout the object's lifetime. */
    VFSFile(const char *fn);

    virtual ~VFSFile();

    /** Open a file.
        Mode can be "r", "w", "rb", "rb", and possibly other things that fopen supports.
        It is the subclass's choice to support other modes. Default is "rb".
        Closes and reopens if already open (even in the same mode). */
    virtual bool open(const char *mode = NULL) { return false; }

    virtual bool isopen(void) const { return false; }
    virtual bool iseof(void) const { return true; }
    virtual bool close(void) { return true; }
    virtual bool seek(vfspos pos) { return false; }

    /** Seek relative to current position. Negative numbers will seek backwards.
        (In most cases, the default implementation does not have to be changed) */
    virtual bool seekRel(vfspos offs) { VFS_GUARD_OPT(this); return seek(getpos() + offs); }

    virtual bool flush(void) { return true; }

    /** Current offset in file. Return npos if NA. */
    virtual vfspos getpos(void) const { return npos; }

    virtual unsigned int read(void *dst, unsigned int bytes) { return 0; }
    virtual unsigned int write(const void *src, unsigned int bytes) { return 0; }

    /** Return file size. If NA, return npos. If size is not yet known,
        open() and close() may be called (with default args) to find out the size.
        The file is supposed to be in its old state when the function returns,
        that is in the same open state and seek position. */
    virtual vfspos size(void) { return npos; }

    /** Basic RTTI, for debugging purposes */
    virtual const char *getType(void) const { return "virtual"; }
};

class VFSFileReal : public VFSFile
{
public:
    VFSFileReal(const char *name);
    virtual ~VFSFileReal();
    virtual bool open(const char *mode = NULL);
    virtual bool isopen(void) const;
    virtual bool iseof(void) const;
    virtual bool close(void);
    virtual bool seek(vfspos pos);
    virtual bool seekRel(vfspos offs);
    virtual bool flush(void);
    virtual vfspos getpos(void) const;
    virtual unsigned int read(void *dst, unsigned int bytes);
    virtual unsigned int write(const void *src, unsigned int bytes);
    virtual vfspos size(void);
    virtual const char *getType(void) const { return "disk"; }

    inline void *getFP() { return _fh; }

protected:
    
    void *_fh; // FILE*
    void *_buf;
};

class VFSFileMem : public VFSFile
{
public:
    /* Creates a virtual file from a memory buffer. The buffer is passed as-is,
       so for text files you should make sure it ends with a \0 character.
       A deletor function can be passed optionally, that the buffer will be passed to
       when the memory file is destroyed. */
    VFSFileMem(const char *name, void *buf, unsigned int size, delete_func delfunc = NULL);
    virtual ~VFSFileMem();
    virtual bool open(const char *mode = NULL) { return true; }
    virtual bool isopen(void) const { return true; } // always open
    virtual bool iseof(void) const { return _pos >= _size; }
    virtual bool close(void) { return true; } // cant close, but not a problem
    virtual bool seek(vfspos pos) { _pos = pos; return true; }
    virtual bool seekRel(vfspos offs) { _pos += offs; return true; }
    virtual bool flush(void) { return false; } // can't flush, if a successful file write is expected, this IS a problem.
    virtual vfspos getpos(void) const { return _pos; }
    virtual unsigned int read(void *dst, unsigned int bytes);
    virtual unsigned int write(const void *src, unsigned int bytes);
    virtual vfspos size(void) { return _size; }
    virtual const char *getType(void) const { return "mem"; }

protected:

    void *_buf;
    vfspos _pos;
    vfspos _size;
    delete_func _delfunc;
};

VFS_NAMESPACE_END

#endif

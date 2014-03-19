// VFSDir.h - basic directory interface + classes
// For conditions of distribution and use, see copyright notice in VFS.h

#ifndef VFSDIR_H
#define VFSDIR_H

#include "VFSBase.h"
#include <map>
#include <cstring>

#ifdef VFS_USE_HASHMAP
#  include "VFSHashmap.h"
#  include "VFSTools.h"
#endif

VFS_NAMESPACE_START


#ifdef VFS_IGNORE_CASE
#  ifdef _MSC_VER
#    pragma warning(push)
#    pragma warning(disable: 4996)
#  endif

struct ci_less
{
    inline bool operator() (const char *a, const char *b) const
    {
        return VFS_STRICMP(a, b) < 0;
    }
};
typedef ci_less map_compare;

inline int casecmp(const char *a, const char *b)
{
    return VFS_STRICMP(a, b);
}

#  ifdef _MSC_VER
#    pragma warning(pop)
#  endif

#else // VFS_IGNORE_CASE

struct cs_less
{
    inline bool operator() (const char *a, const char *b) const
    {
        return strcmp(a, b) < 0;
    }
};
typedef cs_less map_cmp;

inline int casecmp(const char *a, const char *b)
{
    return strcmp(a, b);
}


#endif // VFS_IGNORE_CASE



class DirBase;
class File;

typedef void (*FileEnumCallback)(File *vf, void *user);
typedef void (*DirEnumCallback)(DirBase *vd, void *user);

typedef std::map<const char *, CountedPtr<Dir>, map_cmp> Dirs;
typedef std::map<const char *, CountedPtr<File>, map_cmp> Files;


class DirBase : public VFSBase
{
public:
    DirBase(const char *fullpath);
    virtual ~DirBase();

    /** Returns a file from this dir's file map.
    Expects the actual file name without path - does NOT descend. */
    virtual File *getFileByName(const char *fn) const = 0;

    /** Enumerate directory with given path. Keeps previously loaded entries.
    Returns the number of files found. */
    virtual unsigned int load(bool recursive) = 0;

    /** Creates a new virtual directory of an internally specified type. */
    virtual Dir *createNew(const char *dir) const = 0;






    /** Returns a file for this dir's subtree. Descends if necessary.
    Returns NULL if the file is not found. */
    File *getFile(const char *fn);

    /** Returns a subdir, descends if necessary. If forceCreate is true,
    create directory tree if it does not exist, and return the originally requested
    subdir. Otherwise return NULL if not found. */
    Dir *getDir(const char *subdir, bool forceCreate = false);

};

class Dir : public DirBase
{
public:

    // Avoid using std::string as key.
    // The file names are known to remain constant during each object's lifetime,
    // so just keep the pointers and use an appropriate comparator function.

    Dir(const char *fullpath);
    virtual ~Dir();

    File *Dir::getFileByName(const char *fn) const;

    /** Can be overloaded if necessary. Called by VFSHelper::ClearGarbage() */
    virtual void clearGarbage() {}

    /** Can be overloaded to close resources this dir keeps open */
    virtual bool close() { return true; }

    /** Iterate over all files or directories, calling a callback function,
        optionally with additional userdata. If safe is true, iterate over a copy.
        This is useful if the callback function modifies the tree, e.g.
        adds or removes files. */
    void forEachFile(FileEnumCallback f, void *user = NULL, bool safe = false);
    void forEachDir(DirEnumCallback f, void *user = NULL, bool safe = false);


    /** Adds a file directly to this directory, allows any name.
    If another file with this name already exists, optionally drop the old one out.
    Returns whether the file was actually added. */
    bool add(File *f, bool overwrite);

    /** Like add(), but if the file name contains a path, descend the tree to the target dir.
        Not-existing subdirs are created on the way. */
    bool addRecursive(File *f, bool overwrite);

protected:

    Files _files;
    Dirs _subdirs;
};

typedef Dir::Files::iterator FileIter;
typedef Dir::Dirs::iterator DirIter;

class DiskDir : public Dir
{
public:
    DiskDir(const char *dir);
    virtual ~DiskDir() {};
    virtual unsigned int load(bool recursive);
    virtual Dir *createNew(const char *dir) const;
    virtual const char *getType(void) const { return "DiskDir"; }
};

VFS_NAMESPACE_END

#endif

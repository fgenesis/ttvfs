// VFSDir.h - basic directory interface + classes
// For conditions of distribution and use, see copyright notice in VFS.h

#ifndef VFSDIR_H
#define VFSDIR_H

#include "VFSBase.h"
#include <map>

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

struct ci_equal
{
    inline bool operator() (const char *a, const char *b) const
    {
        return !VFS_STRICMP(a, b);
    }
};

inline int casecmp(const char *a, const char *b)
{
    return VFS_STRICMP(a, b);
}

#  ifdef _MSC_VER
#    pragma warning(pop)
#  endif

#else // VFS_IGNORE_CASE

inline int casecmp(const char *a, const char *b)
{
    return strcmp(a, b);
}

#endif // VFS_IGNORE_CASE


#ifdef VFS_USE_HASHMAP

struct hashmap_eq
{
    inline bool operator() (const char *a, const char *b, size_t h, const VFSBase *itm) const
    {
        // quick check - instead of just comparing the strings,
        // check the hashes first. If they don't match there is no
        // need to check the strings at all.
        return itm->hash() == h && !casecmp(a, b);
    }
};

struct charptr_hash
{
    inline size_t operator()(const char *s)
    {
        // case sensitive or in-sensitive, depending on config
        return STRINGHASH(s);
    }
};

#endif // VFS_USE_HASHMAP


class VFSDir;
class VFSFile;

class VFSDir : public VFSBase
{
public:

    // Avoid using std::string as key.
    // The file names are known to remain constant during each object's lifetime,
    // so just keep the pointers and use an appropriate comparator function.
#ifdef VFS_USE_HASHMAP
        // VFS_IGNORE_CASE already handled in hash generation
        typedef HashMap<const char *, VFSDir*, charptr_hash, hashmap_eq> Dirs;
        typedef HashMap<const char *, VFSFile*, charptr_hash, hashmap_eq> Files;
#else
#  ifdef VFS_IGNORE_CASE
        typedef std::map<const char *, VFSDir*, ci_less> Dirs;
        typedef std::map<const char *, VFSFile*, ci_less> Files;
#  else
        typedef std::map<const char *, VFSDir*, strcmp> Dirs;
        typedef std::map<const char *, VFSFile*, strcmp> Files;
#  endif
#endif

    VFSDir(const char *fullpath);
    virtual ~VFSDir();

    /* Enumerate directory with given path. If dir is NULL, simply reload directory.
       If dir is not NULL, load files of that directory instead. Clears previously
       loaded entries. */
    virtual unsigned int load(const char *dir = NULL);
    virtual VFSFile *getFile(const char *fn);
    virtual VFSDir *getDir(const char *subdir, bool forceCreate = false);
    virtual VFSDir *createNew(const char *dir) const;
    virtual const char *getType(void) const { return "VFSDir"; }

    bool insert(VFSDir *subdir, bool overwrite = true);
    bool merge(VFSDir *dir, bool overwrite = true);
    bool add(VFSFile *f, bool overwrite = true); // add file directly in this dir
    bool addRecursive(VFSFile *f, bool overwrite = true); // traverse subdir tree to find correct subdir; create if not existing

    // iterators are NOT thread-safe! If you need to iterate over things in a multithreaded environment,
    // do the locking yourself! (see below)
    inline Files::iterator       fileIter()          { return _files.begin(); }
    inline Files::iterator       fileIterEnd()       { return _files.end(); }
    inline Dirs::iterator        dirIter()           { return _subdirs.begin(); }
    inline Dirs::iterator        dirIterEnd()        { return _subdirs.end(); }

    // std::map<const char*,X> or ttvfs::HashMap<const char*, X> stores for files and subdirs
    Files _files;
    Dirs _subdirs;
};

typedef VFSDir::Files::iterator FileIter;
typedef VFSDir::Dirs::iterator DirIter;

class VFSDirReal : public VFSDir
{
public:
    VFSDirReal(const char *dir);
    virtual ~VFSDirReal() {};
    virtual unsigned int load(const char *dir = NULL);
    virtual VFSDir *createNew(const char *dir) const;
    virtual const char *getType(void) const { return "VFSDirReal"; }
};

VFS_NAMESPACE_END

#endif

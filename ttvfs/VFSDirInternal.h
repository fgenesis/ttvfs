#ifndef VFS_DIR_INTERNAL_H
#define VFS_DIR_INTERNAL_H

#include "VFSDir.h"
#include <vector>

VFS_NAMESPACE_START


class VFSHelper;

// Internal class, not to be used outside

class InternalDir : public DirBase
{
    friend class VFSHelper;

public:
    // virtual overrides (final)
    const char *getType() const { return "InternalDir"; }
    void forEachFile(FileEnumCallback f, void *user = NULL, bool safe = false);
    void load();
    File *getFileByName(const char *fn) const;

protected:
    // virtual overrides(final)
    InternalDir *createNew(const char *dir) const;

    typedef std::vector<CountedPtr<DirBase> > MountedDirs;
    MountedDirs _mountedDirs;

private:
    InternalDir(const char *);
    virtual ~InternalDir();
    bool insert(Dir *subdir, bool overwrite);
    bool merge(Dir *dir, bool overwrite);

    void _clearDirs();
    void _clearAllMountsRec();
    void _remount();
    void _addMountDir(DirBase *d);
};


VFS_NAMESPACE_END

#endif

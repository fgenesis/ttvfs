#ifndef VFS_DIR_INTERNAL_H
#define VFS_DIR_INTERNAL_H

#include "VFSDir.h"
#include <vector>

VFS_NAMESPACE_START


class VFSHelper;
class DirView;

// Internal class, not to be used outside

class InternalDir : public DirBase
{
    friend class VFSHelper;

public:

    bool fillView(const char *path, DirView& view);

    // virtual overrides (final)
    const char *getType() const { return "InternalDir"; }
    void forEachFile(FileEnumCallback f, void *user = NULL, bool safe = false);
    void forEachDir(DirEnumCallback f, void *user = NULL, bool safe = false);
    File *getFileByName(const char *fn) const;

protected:

    // virtual overrides(final)
    InternalDir *createNew(const char *dir) const;
    bool _addToView(char *path, DirView& view);

private:

    InternalDir(const char *);
    virtual ~InternalDir();

    typedef std::vector<CountedPtr<DirBase> > MountedDirs;
    MountedDirs _mountedDirs;

    void _clearDirs();
    void _clearMounts();
    void _addMountDir(CountedPtr<DirBase> d);
    void _removeMountDir(DirBase *d);

    bool s_fillView(DirBase *cur, char *path, DirView& view) const;

};


VFS_NAMESPACE_END

#endif

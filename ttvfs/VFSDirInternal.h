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

    enum MountType
    {
        MOUNT_IMPLIED, // mount in subdir by the other two
        MOUNT_MOUNTED, // mounted by user
        MOUNT_FIXED, // can't be changed
    };

    // virtual overrides (final)
    const char *getType() const { return "InternalDir"; }
    void forEachFile(FileEnumCallback f, void *user = NULL, bool safe = false);
    void load();
    File *getFileByName(const char *fn) const;

protected:

    struct MountEntry
    {
        MountEntry() {}
        MountEntry(DirBase *d, MountType t) : dir(d), type(t) {}
        CountedPtr<DirBase> dir;
        MountType type;
    };

    // virtual overrides(final)
    InternalDir *createNew(const char *dir) const;

    typedef std::vector<MountEntry> MountedDirs;
    MountedDirs _mountedDirs;

private:
    InternalDir(const char *);
    virtual ~InternalDir();
    bool insert(Dir *subdir, bool overwrite);
    bool merge(Dir *dir, bool overwrite);

    void _clearDirs();
    void _clearMountsRec(MountType level); // clear all mounts weaker or equal to level
    void _addMountDir(DirBase *d, MountType ty);

    static void s_addSubMount(DirBase *d, void *user);

    class MountCheck
    {
    public:
        MountCheck(MountType level) : _level(level) {}
        inline bool operator() (MountEntry& e) const
        {
            return e.type <= _level;
        }
    private:
        MountType _level;
    };
};


VFS_NAMESPACE_END

#endif

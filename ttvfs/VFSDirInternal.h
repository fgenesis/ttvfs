#ifndef VFS_DIR_INTERNAL_H
#define VFS_DIR_INTERNAL_H

class VFSHelper;

// Internal class, not to be used outside

class InternalDir : public DirBase
{
    friend class VFSHelper;

public:
    // virtual overrides (final)
    const char *getType() const { return "InternalDir"; }

protected:
    // virtual overrides(final)
    InternalDir *createNew(const char *dir) const;
    unsigned int load(bool recursive);
    File *getFileByName(const char *fn) const;


    std::vector<CountedPtr<Dir> > _mountedDirs;

private:
    bool insert(Dir *subdir, bool overwrite);
    bool merge(Dir *dir, bool overwrite);

    void _clearAllMountsRec();
    void _remount();
    void _addMountDir(DirBase *d);
};

#endif

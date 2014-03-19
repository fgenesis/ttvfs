#ifndef VFS_DIR_INTERNAL_H
#define VFS_DIR_INTERNAL_H


// Internal class, not to be used outside

class InternalDir : public DirBase
{
public:
    // virtual overrides (final)
    InternalDir *createNew(const char *dir) const;
    unsigned int load(bool recursive);
    const char *getType() const { return "InternalDir"; }
    File *getFileByName(const char *fn) const;


    bool insert(Dir *subdir, bool overwrite);
    bool merge(Dir *dir, bool overwrite);

    void _remount();

protected:
    std::vector<CountedPtr<Dir> > _mountedDirs;
};

#endif

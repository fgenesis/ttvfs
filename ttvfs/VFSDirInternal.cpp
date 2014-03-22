#include "VFSDirInternal.h"
#include "VFSInternal.h"
#include "VFSFile.h"
#include <algorithm>

VFS_NAMESPACE_START


// Internal class, not to be used outside

InternalDir::InternalDir(const char *fullpath)
: DirBase(fullpath)
{
}

InternalDir::~InternalDir()
{
}

void InternalDir::_clearDirs()
{
    _subdirs.clear();
}

InternalDir *InternalDir::createNew(const char *dir) const
{
    return new InternalDir(dir);
}

void InternalDir::s_addSubMount(DirBase *d, void *user)
{
    InternalDir *this_ = (InternalDir*)user;
    InternalDir *sub = safecastNonNull<InternalDir*>(this_->getDir(d->name(), true));
    sub->_addMountDir(d, MOUNT_IMPLIED);
}

void InternalDir::_addMountDir(DirBase *d, MountType type)
{
    for(MountedDirs::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
        if(it->dir.content() == d)
        {
            // make permanent mount if it's already a temporary
            it->type = std::max(type, it->type);
            return;
        }
    _mountedDirs.push_back(MountEntry(d, type));
    d->forEachDir(s_addSubMount, this);
}

void InternalDir::load()
{
    for(MountedDirs::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
        it->dir->load();
}

void InternalDir::_clearMountsRec(MountType level)
{
    for(MountedDirs::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
        safecastNonNull<InternalDir*>(it->dir.content())->_clearMountsRec(std::min(it->type, level)); // min(): don't clear user mounts from implied mounts, for example
    MountedDirs::iterator it = std::remove_if(_mountedDirs.begin(), _mountedDirs.end(), MountCheck(level));
    _mountedDirs.resize(std::distance(it, _mountedDirs.end()));
}


File *InternalDir::getFileByName(const char *fn) const
{
    if(_mountedDirs.size())
        for(MountedDirs::const_reverse_iterator it = _mountedDirs.rbegin(); it != _mountedDirs.rend(); ++it)
            if(File *f = it->dir->getFileByName(fn))
                return f;
    return NULL;
}

static void _addFileCallback(File *f, void *p)
{
    ((Files*)p)->insert(std::make_pair(f->name(), f)); // only inserts if not exist
}

void InternalDir::forEachFile(FileEnumCallback f, void *user /* = NULL */, bool /*ignored*/)
{
    Files flist; // TODO: optimize allocation
    for(MountedDirs::reverse_iterator it = _mountedDirs.rbegin(); it != _mountedDirs.rend(); ++it)
        it->dir->forEachFile(_addFileCallback, &flist);

    for(Files::iterator it = flist.begin(); it != flist.end(); ++it)
        f(it->second, user);
}




VFS_NAMESPACE_END


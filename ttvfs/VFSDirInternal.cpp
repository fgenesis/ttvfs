#include "VFSDirInternal.h"
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

void InternalDir::_addMountDir(DirBase *d)
{
    for(MountedDirs::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
        if(it->content() == d)
            return;
    _mountedDirs.push_back(d);
}

void InternalDir::load()
{
    for(MountedDirs::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
        (*it)->load();
}

void InternalDir::_clearAllMountsRec()
{
    for(MountedDirs::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
        safecastNonNull<InternalDir*>(it->content())->_clearAllMountsRec();
    _mountedDirs.clear();
}

void InternalDir::_remount()
{
    for(MountedDirs::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
    {
        InternalDir *sub = safecastNonNull<InternalDir*>(getDir((*it)->name(), true));
        sub->_addMountDir(*it);
    }
}


File *InternalDir::getFileByName(const char *fn) const
{
    if(_mountedDirs.size())
        for(MountedDirs::const_reverse_iterator it = _mountedDirs.rbegin(); it != _mountedDirs.rend(); ++it)
            if(File *f = (*it)->getFileByName(fn))
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
        (*it)->forEachFile(_addFileCallback, &flist);

    for(Files::iterator it = flist.begin(); it != flist.end(); ++it)
        f(it->second, user);
}




VFS_NAMESPACE_END


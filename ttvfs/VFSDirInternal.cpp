#include "VFSDir.h"


// Internal class, not to be used outside

Dir *InternalDir::createNew(const char *dir) const
{
    return new InternalDir(dir);
}

void InternalDir::_addMountDir(DirBase *d)
{
    if(std::find(_mountedDirs.begin(), _mountedDirs.end()) == _mountedDirs.end())
        _mountedDirs.push_back(d);
}

unsigned int InternalDir::load(bool recursive)
{
    for(std::vector<Dir>::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
        (*it)->load(recursive);
}

void InternalDir::_clearAllMountsRec()
{
    for(std::vector<Dir>::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
        safecastNonNull<InternalDir*>(*it)->_clearAllMountsRec();
    _mountedDirs.clear();
}

void InternalDir::_remount()
{
    for(std::vector<Dir>::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
    {
        InternalDir *sub = safecastNonNull<InternalDir*>(getDir(it->first, true));
        sub->_addMountDir(it->second);
    }
}


File *Dir::getFileByName(const char *fn) const
{
    if(_mountedDirs.size())
        for(std::vector<Dir>::reverse_iterator it = _mountedDirs.rbegin(); it != _mountedDirs.rend(); ++it)
            if(File *f = (*it)->getFileByName(fn))
                return f;
    return NULL;
}



VFS_NAMESPACE_END


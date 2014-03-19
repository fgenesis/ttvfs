#include "VFSDir.h"


// Internal class, not to be used outside

Dir *InternalDir::createNew(const char *dir) const
{
    return new InternalDir(dir);
}

unsigned int InternalDir::load(bool recursive)
{
    for(std::vector<Dir>::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
        (*it)->load(recursive);
    _remount();
}

void InternalDir::_remount()
{
    for(std::vector<Dir>::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
    {
        InternalDir *sub = createNew(it->first);
}


File *Dir::getFileByName(const char *fn) const
{
    if(_mountedDirs.size())
        for(std::vector<Dir>::reverse_iterator it = _mountedDirs.rbegin(); it != _mountedDirs.rend(); ++it)
            if(File *f = (*it)->getFileByName(fn))
                return f;
    return NULL;
}




bool InternalDir::add(File *f, bool overwrite, EntryFlags flag)
{
    if(!f)
        return false;

    VFS_GUARD_OPT(this);

    Files::iterator it = _files.find(f->name());

    if(it != _files.end())
    {
        if(overwrite)
        {
            File *oldf = it->second.ptr;
            if(oldf == f)
                return false;

            _files.erase(it);
            oldf->ref--;
        }
        else
            return false;
    }

    f->ref++;
    _files[f->name()] = MapEntry<File>(f, flag);
    return true;
}

bool InternalDir::addRecursive(File *f, bool overwrite, EntryFlags flag)
{
    if(!f)
        return false;

    // figure out directory from full file name
    Dir *vdir;
    size_t prefixLen = f->fullnameLen() - f->nameLen();
    if(prefixLen)
    {
        char *dirname = (char*)VFS_STACK_ALLOC(prefixLen);
        --prefixLen; // -1 to strip the trailing '/'. That's the position where to put the terminating null byte.
        memcpy(dirname, f->fullname(), prefixLen); // copy trailing null byte
        dirname[prefixLen] = 0;
        vdir = getDir(dirname, true);
        VFS_STACK_FREE(dirname);
    }
    else
        vdir = this;

    return vdir->add(f, true, flag);
}

bool InternalDir::merge(Dir *dir, bool overwrite, EntryFlags flag)
{
    if(!dir)
        return false;
    if(dir == this)
        return true; // nothing to do then

    bool result = false;

    for(Files::iterator it = dir->_files.begin(); it != dir->_files.end(); ++it)
        result = add(it->second.ptr, overwrite, flag) || result;

    for(Dirs::iterator it = dir->_subdirs.begin(); it != dir->_subdirs.end(); ++it)
        result = insert(it->second.ptr, overwrite, flag) || result;
    return result;
}

bool InternalDir::insert(Dir *subdir, bool overwrite, EntryFlags flag)
{
    if(!subdir)
        return false;

    VFS_GUARD_OPT(this);

    // With load() cleaning up the tree, it is ok to add subsequent VFSDirs directly.
    // This will be very useful at some point when files can be mounted into a directory
    // belonging to an archive, and this way adding files to the archive.
    Dirs::iterator it = _subdirs.find(subdir->name());
    if(it == _subdirs.end())
    {
        subdir->ref++;
        _subdirs[subdir->name()] = MapEntry<Dir>(subdir, flag);
        return true;
    }
    else
    {
        it->second.ptr->merge(subdir, overwrite, flag);
        return false;
    }
}

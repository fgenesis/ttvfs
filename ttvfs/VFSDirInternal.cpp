#include "VFSDirInternal.h"
#include "VFSDirView.h"
#include "VFSInternal.h"
#include "VFSFile.h"
#include "VFSTools.h"
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

void InternalDir::_clearMounts()
{
    _mountedDirs.clear();
}

InternalDir *InternalDir::createNew(const char *dir) const
{
    return new InternalDir(dir);
}


void InternalDir::_addMountDir(CountedPtr<DirBase> d)
{
    // move to end of vector if already mounted
    for(MountedDirs::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
        if(*it == d)
        {
            _mountedDirs.erase(it);
            break; 
        }

    _mountedDirs.push_back(d);
}

void InternalDir::_removeMountDir(DirBase *d)
{
    for(MountedDirs::iterator it = _mountedDirs.begin(); it != _mountedDirs.end(); ++it)
        if(it->content() == d)
        {
            _mountedDirs.erase(it);
            return; // pointers are unique
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

static void _iterDirs(Dirs &m, DirEnumCallback f, void *user)
{
    for(Dirs::iterator it = m.begin(); it != m.end(); ++it)
        f(it->second.content(), user);
}

void InternalDir::forEachDir(DirEnumCallback f, void *user /* = NULL */, bool safe /* = false */)
{
    for(MountedDirs::reverse_iterator it = _mountedDirs.rbegin(); it != _mountedDirs.rend(); ++it)
        (*it)->forEachDir(f, user, safe);
}


bool InternalDir::fillView(const char *path, DirView& view)
{
    SkipSelfPath(path);
    view.init(path);
    char *pathcopy = (char*)VFS_STACK_ALLOC(strlen(path) + 1);
    bool added = s_fillView(this, pathcopy, view);
    VFS_STACK_FREE(pathcopy);
    return added;
}

// assumes that path is writable
bool InternalDir::s_fillView(DirBase *cur, char *path, DirView& view) const
{
    char *slashpos = (char *)strchr(path, '/');
    bool added = false;

    if(slashpos)
    {
        char *subpath = slashpos+1;
        SkipSelfPath(subpath);

        *slashpos = 0;
        cur->_addToView(subpath, view);
        *slashpos = '/';
    }
    else
    {
        char nul = 0;
        cur->_addToView(&nul, view);
    }

    return added;
}

bool InternalDir::_addToView(char *path, DirView& view)
{
    bool added = false;

    if(*path)
    {
        for(MountedDirs::reverse_iterator it = _mountedDirs.rbegin(); it != _mountedDirs.rend(); ++it)
            if(!VFS_STRICMP((*it)->name(), path))
                added = s_fillView(it->content(), path, view) || added;

        if(InternalDir *subdir = safecast<InternalDir*>(getDirByName(path)))
            added = s_fillView(subdir, path, view) || added;
    }
    else
    {
        for(MountedDirs::reverse_iterator it = _mountedDirs.rbegin(); it != _mountedDirs.rend(); ++it)
        {
            added = true;
            view.add(it->content());
        }
    }

    return added;
}




VFS_NAMESPACE_END


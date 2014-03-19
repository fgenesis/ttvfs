// Dir.cpp - basic directory interface + classes
// For conditions of distribution and use, see copyright notice in VFS.h

#include <set>

#include "VFSInternal.h"
#include "VFSTools.h"
#include "VFSFile.h"
#include "VFSDir.h"

VFS_NAMESPACE_START

DirBase::DirBase(const char *fullpath)
{
    _setName(fullpath);
}

DirBase::~DirBase()
{
}


File *DirBase::getFile(const char *fn)
{
    while(fn[0] == '.' && fn[1] == '/') // skip ./
        fn += 2;

    char *slashpos = (char *)strchr(fn, '/');

    // if there is a '/' in the string, descend into subdir and continue there
    if(slashpos)
    {
        // "" (the empty directory name) is allowed, so we can't return 'this' when hitting an empty string the first time.
        // This whole mess is required for absolute unix-style paths ("/home/foo/..."),
        // which, to integrate into the tree properly, sit in the root directory's ""-subdir.
        // FIXME: Bad paths (double-slashes and the like) need to be normalized elsewhere, currently!

        size_t len = strlen(fn);
        char *dup = (char*)VFS_STACK_ALLOC(len + 1);
        memcpy(dup, fn, len + 1); // also copy the null-byte
        slashpos = dup + (slashpos - fn); // use direct offset, not to have to recount again the first time
        Dir *subdir = this;
        const char *ptr = dup;
        Dirs::iterator it;

        goto pos_known;
        do
        {
            ptr = slashpos + 1;
            while(ptr[0] == '.' && ptr[1] == '/') // skip ./
                ptr += 2;
            slashpos = (char *)strchr(ptr, '/');
            if(!slashpos)
                break;

        pos_known:
            *slashpos = 0;
            it = subdir->_subdirs.find(ptr);
            if(it != subdir->_subdirs.end())
                subdir = it->second.ptr; // found it
            else
                subdir = NULL; // bail out
        }
        while(subdir);
        VFS_STACK_FREE(dup);
        if(!subdir)
            return NULL;
        // restore pointer into original string, by offset
        ptr = fn + (ptr - dup);
        return subdir->getFileByName(ptr);
    }

    // no subdir? file must be in this dir now.
    return getFileByName(fn);
}


Dir *DirBase::getDir(const char *subdir, bool forceCreate /* = false */)
{
    if(!subdir[0] || (subdir[0] == '.' && (!subdir[1] || subdir[1] == '/'))) // empty string or "." or "./" ? use this.
        return this;

    Dir *ret = NULL;
    char *slashpos = (char *)strchr(subdir, '/');

    // if there is a '/' in the string, descend into subdir and continue there
    if(slashpos)
    {
        // from a/b/c, cut out the a, without the trailing '/'.
        const char *sub = slashpos + 1;
        size_t copysize = slashpos - subdir;
        char * const t = (char*)VFS_STACK_ALLOC(copysize + 1);
        memcpy(t, subdir, copysize);
        t[copysize] = 0;
        Dirs::iterator it = _subdirs.find(t);
        if(it != _subdirs.end())
        {
            ret = it->second.ptr->getDir(sub, forceCreate); // descend into subdirs
        }
        else if(forceCreate)
        {
            // -> newname = fullname() + '/' + t
            size_t fullLen = fullnameLen();
            Dir *ins;
            if(fullLen)
            {
                char * const newname = (char*)VFS_STACK_ALLOC(fullLen + copysize + 2);
                char *ptr = newname;
                memcpy(ptr, fullname(), fullLen);
                ptr += fullLen;
                *ptr++ = '/';
                memcpy(ptr, t, copysize);
                ptr[copysize] = 0;
                ins = createNew(newname);
                VFS_STACK_FREE(newname);
            }
            else
                ins = createNew(t);

            _subdirs[ins->name()] = ins;
            ret = ins->getDir(sub, true); // create remaining structure
        }
    }
    else
    {
        Dirs::iterator it = _subdirs.find(subdir);
        if(it != _subdirs.end())
            ret = it->second.ptr;
        else if(forceCreate)
        {
            size_t fullLen = fullnameLen();
            if(fullLen)
            {
                // -> newname = fullname() + '/' + subdir
                size_t subdirLen = strlen(subdir);
                char * const newname = (char*)VFS_STACK_ALLOC(fullLen + subdirLen + 2);
                char *ptr = newname;
                memcpy(ptr, fullname(), fullLen);
                ptr += fullLen;
                *ptr++ = '/';
                memcpy(ptr, subdir, subdirLen);
                ptr[subdirLen] = 0;

                ret = createNew(newname);
                VFS_STACK_FREE(newname);
            }
            else
            {
                ret = createNew(subdir);
            }

            _subdirs[ret->name()] = ret;
        }
    }

    return ret;
}


Dir::Dir(const char *fullpath)
: DirBase(fullpath)
{
}

Dir::~Dir()
{
}

File *Dir::getFileByName(const char *fn) const
{
    Files::iterator it = _files.find(fn);
    return it == _files.end() ? NULL : it->second;
}

template<typename T> static void iterIncref(T *b, void*) { ++(b->ref); }
template<typename T> static void iterDecref(T *b, void*) { --(b->ref); }

static void _iterDirs(Dir::Dirs &m, DirEnumCallback f, void *user)
{
    for(DirIter it = m.begin(); it != m.end(); ++it)
        f(it->second.ptr, user);
}

void Dir::forEachDir(DirEnumCallback f, void *user /* = NULL */, bool safe /* = false */)
{
    if(safe)
    {
        Dirs cp = _subdirs;
        _iterDirs(cp, iterIncref<Dir>, NULL);
        _iterDirs(cp, f, user);
        _iterDirs(cp, iterDecref<Dir>, NULL);
    }
    else
        _iterDirs(_subdirs, f, user);
}

static void _iterFiles(Dir::Files &m, FileEnumCallback f, void *user)
{
    for(FileIter it = m.begin(); it != m.end(); ++it)
        f(it->second.ptr, user);
}

void Dir::forEachFile(FileEnumCallback f, void *user /* = NULL */, bool safe /* = false */)
{
    if(safe)
    {
        Files cp = _files;
        _iterFiles(cp, iterIncref<File>, NULL);
        _iterFiles(cp, f, user);
        _iterFiles(cp, iterDecref<File>, NULL);
    }
    else
        _iterFiles(_files, f, user);
}


// ----- DiskDir start here -----


DiskDir::DiskDir(const char *dir) : Dir(dir)
{
}

Dir *DiskDir::createNew(const char *dir) const
{
    return new DiskDir(dir);
}

unsigned int DiskDir::load(bool recursive)
{
    Files remainF;
    Dirs remainD;

    remainF.swap(_files);
    remainD.swap(_subdirs);

    // _files, _subdirs now empty

    StringList li;
    GetFileList(fullname(), li);
    for(StringList::iterator it = li.begin(); it != li.end(); ++it)
    {
        // file was already present, move over and erase
        FileIter fi = remainF.find(it->c_str());
        if(fi != remainF.end())
        {
            _files[fi->first] = fi->second;
            remainF.erase(fi);
            continue;
        }

        // TODO: use stack alloc
        std::string tmp = fullname();
        tmp += '/';
        tmp += *it;
        DiskFile *f = new DiskFile(tmp.c_str());
        _files[f->name()] = f;
    }
    unsigned int sum = li.size();

    li.clear();
    GetDirList(fullname(), li, 0);
    for(std::deque<std::string>::iterator it = li.begin(); it != li.end(); ++it)
    {
        // subdir was already present, move over and erase
        DirIter fi = remainD.find(it->c_str());
        if(fi != remainD.end())
        {
            if(recursive)
                sum += fi->second.ptr->load(true);
            ++sum;

            _subdirs[fi->first] = fi->second;
            remainD.erase(fi);
            continue;
        }

        // TODO: use stack alloc
        std::string full(fullname());
        full += '/';
        full += *it; // GetDirList() always returns relative paths

        Dir *d = createNew(full.c_str());
        if(recursive)
            sum += d->load(true);
        ++sum;
        _subdirs[d->name()] = d;
    }

    // clean up & remove no longer existing files & dirs,
    // and move over entries mounted here.
    for(FileIter it = remainF.begin(); it != remainF.end(); ++it)
        if(it->second.isMounted())
            _files[it->first] = it->second;
        else
            it->second.ptr->ref--;
    for(DirIter it = remainD.begin(); it != remainD.end(); ++it)
        if(it->second.isMounted())
            _subdirs[it->first] = it->second;
        else
            it->second.ptr->ref--;

    return sum;
}

VFS_NAMESPACE_END

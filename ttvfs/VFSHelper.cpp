// VFSHelper.cpp - glues it all together and makes use simple
// For conditions of distribution and use, see copyright notice in VFS.h

#include <iostream> // for debug only, see EOF

#include "VFSInternal.h"
#include "VFSHelper.h"
#include "VFSTools.h"

#include "VFSDir.h"
#include "VFSFile.h"
#include "VFSLoader.h"
#include "VFSArchiveLoader.h"

#ifdef _DEBUG
#  include <cassert>
#  define DEBUG_ASSERT(x) assert(x)
#else
#  define DEBUG_ASSERT(x)
#endif


VFS_NAMESPACE_START

// predecl is in VFS.h
bool _checkCompatInternal(_AbiCheck *used)
{
    if(sizeof(_AbiCheck) != used->structSize)
        return false;

    _AbiCheck here;
    memset(&here, 0, sizeof(here));
    here.structSize = sizeof(here);
    here.vfsposSize = sizeof(vfspos);

#ifdef VFS_LARGEFILE_SUPPORT
    here.largefile = 1;
#endif

#ifdef VFS_IGNORE_CASE
    here.nocase = 1;
#endif

    return !memcmp(&here, used, sizeof(here));
}

VFSHelper::VFSHelper()
: filesysRoot(NULL), merged(NULL)
{
}

VFSHelper::~VFSHelper()
{
    Clear();
}

void VFSHelper::Clear(void)
{
    _cleanup();

    if(filesysRoot)
    {
        filesysRoot->ref--; // this should always delete it...
        filesysRoot = NULL; // ...but it may be referenced elsewhere, just in case
    }

    _ClearMountPoints();

    for(LoaderArray::iterator it = loaders.begin(); it != loaders.end(); ++it)
        delete *it;
    loaders.clear();

    for(DirArray::iterator it = trees.begin(); it != trees.end(); ++it)
        it->dir->ref--;
    trees.clear();

    for(ArchiveLoaderArray::iterator it = archLdrs.begin(); it != archLdrs.end(); ++it)
        delete *it;
    archLdrs.clear();
}

void VFSHelper::_ClearMountPoints(void)
{
    for(VFSMountList::iterator it = vlist.begin(); it != vlist.end(); ++it)
        it->vdir->ref--;
    vlist.clear();
}

void VFSHelper::_cleanup(void)
{
    if(merged)
    {
        merged->ref--;
        merged = NULL;
    }
}

bool VFSHelper::LoadFileSysRoot(bool recursive)
{
    if(filesysRoot)
        return !!filesysRoot->load(recursive);

    filesysRoot = new DiskDir(".");
    if(!filesysRoot->load(recursive))
    {
        filesysRoot->ref--;
        filesysRoot = NULL;
        return false;
    }

    loaders.push_back(new VFSLoaderDisk);

    BaseTreeEntry bt;
    bt.source = "";
    bt.dir = filesysRoot;
    trees.push_back(bt);
    filesysRoot->ref++;

    AddVFSDir(filesysRoot, "");

    return true;
}

// TODO: deprecate this
void VFSHelper::Prepare(bool clear /* = true */)
{
    Reload(false, clear, false); // HACK

    //for(DirArray::iterator it = trees.begin(); it != trees.end(); ++it)
    //    merged->getDir((*it)->fullname(), true)->merge(*it);
}

void VFSHelper::Reload(bool fromDisk /* = false */, bool clear /* = false */, bool clearMountPoints /* = false */)
{
    if(clearMountPoints)
        _ClearMountPoints();
    if(fromDisk && filesysRoot)
        LoadFileSysRoot(true);
    if(clear)
        _cleanup();
    if(!merged && trees.size())
    {
        for(DirArray::iterator it = trees.begin(); it != trees.end(); ++it)
            it->dir->clearMounted();

        // FIXME: not sure if really correct
        merged = trees[0].dir;
        merged->ref++;

        // FIXME: this is too hogging
        //merged->load(true);
    }
    if(merged)
    {
        for(VFSMountList::iterator it = vlist.begin(); it != vlist.end(); ++it)
        {
            //printf("VFS: mount {%s} [%s] -> [%s] (overwrite: %d)\n", it->vdir->getType(), it->vdir->fullname(), it->mountPoint.c_str(), it->overwrite);
            GetDir(it->mountPoint.c_str(), true)->merge(it->vdir, it->overwrite, Dir::MOUNTED);
        }
    }
}

bool VFSHelper::Mount(const char *src, const char *dest, bool overwrite /* = true*/)
{
    return AddVFSDir(GetDir(src, false), dest, overwrite);
}

bool VFSHelper::AddVFSDir(Dir *dir, const char *subdir /* = NULL */, bool overwrite /* = true */)
{
    if(!dir)
        return false;
    if(!subdir)
        subdir = dir->fullname();

    VDirEntry ve(dir, subdir, overwrite);
    _StoreMountPoint(ve);

    Dir *sd = GetDir(subdir, true);
    if(!sd) // may be NULL if Prepare() was not called before
        return false;
    sd->merge(dir, overwrite, Dir::MOUNTED); // merge into specified subdir. will be (virtually) created if not existing

    return true;
}

bool VFSHelper::Unmount(const char *src, const char *dest)
{
    Dir *vd = GetDir(src, false);
    if(!vd)
        return false;

    VDirEntry ve(vd, dest, true); // last is dummy
    if(!_RemoveMountPoint(ve))
        return false;

    // FIXME: this could be done more efficiently by just reloading parts of the tree that were involved.
    Reload(false, true, false);
    //vd->load(true);
    //vd->load(false);
    /*Dir *dstdir = GetDir(dest, false);
    if(dstdir)
        dstdir->load(false);*/
    return true;
}

void VFSHelper::_StoreMountPoint(const VDirEntry& ve)
{
    // increase ref already before it will be added
    ve.vdir->ref++;

    // scan through and ensure only one mount point with the same data is present.
    // if present, remove and re-add, this ensures the mount point is at the end of the list
    for(VFSMountList::iterator it = vlist.begin(); it != vlist.end(); )
    {
        const VDirEntry& oe = *it;
        if (ve.mountPoint == oe.mountPoint
            && (ve.vdir == oe.vdir || !casecmp(ve.vdir->fullname(), oe.vdir->fullname()))
            && (ve.overwrite || !oe.overwrite) ) // overwrite definitely, or if other does not overwrite
        {
            it->vdir->ref--;
            vlist.erase(it++); // do not break; just in case there are more (fixme?)
        }
        else
            ++it;
    }

    vlist.push_back(ve);
}

bool VFSHelper::_RemoveMountPoint(const VDirEntry& ve)
{
    for(VFSMountList::iterator it = vlist.begin(); it != vlist.end(); ++it)
    {
        const VDirEntry& oe = *it;
        if(ve.mountPoint == oe.mountPoint
            && (ve.vdir == oe.vdir || !casecmp(ve.vdir->fullname(), oe.vdir->fullname())) )
        {
            it->vdir->ref--;
            vlist.erase(it);
            return true;
        }
    }
    return false;
}

bool VFSHelper::MountExternalPath(const char *path, const char *where /* = "" */, bool loadRec /* = false */, bool overwrite /* = true */)
{
    DiskDir *vfs = new DiskDir(path);
    if(vfs->load(loadRec))
        AddVFSDir(vfs, where, overwrite);
    return !!--(vfs->ref); // 0 if deleted
}

void VFSHelper::AddLoader(VFSLoader *ldr)
{
    loaders.push_back(ldr);
}

void VFSHelper::AddArchiveLoader(VFSArchiveLoader *ldr)
{
    archLdrs.push_back(ldr);
}

Dir *VFSHelper::AddArchive(const char *arch, bool asSubdir /* = true */, const char *subdir /* = NULL */, void *opaque /* = NULL */)
{
    File *af = GetFile(arch);
    if(!af)
        return NULL;

    Dir *ad = NULL;
    VFSLoader *fileLdr = NULL;
    for(ArchiveLoaderArray::iterator it = archLdrs.begin(); it != archLdrs.end(); ++it)
        if((ad = (*it)->Load(af, &fileLdr, opaque)))
            break;
    if(!ad)
        return NULL;

    if(fileLdr)
        loaders.push_back(fileLdr);

    BaseTreeEntry bt;
    bt.source = arch;
    bt.dir = ad;
    trees.push_back(bt);

    AddVFSDir(ad, subdir, true);

    return ad;
}

inline static File *VFSHelper_GetFileByLoader(VFSLoader *ldr, const char *fn, const char *unmangled, Dir *root)
{
    if(!ldr)
        return NULL;
    File *vf = ldr->Load(fn, unmangled);
    if(vf)
    {
        root->addRecursive(vf, true, Dir::NONE);
        --(vf->ref);
    }
    return vf;
}

File *VFSHelper::GetFile(const char *fn)
{
    if(!merged) // Prepare() called?
        return NULL;

    const char *unmangled = fn;
    std::string fixed = fn; // TODO: get rid of allocation here
    FixPath(fixed);
    fn = fixed.c_str();

    File *vf = NULL;

    vf = merged->getFile(fn);

    // nothing found? maybe a loader has something.
    // if so, add the newly created File to the tree.
    if(!vf)
        for(LoaderArray::iterator it = loaders.begin(); it != loaders.end(); ++it)
            if((vf = VFSHelper_GetFileByLoader(*it, fn, unmangled, GetDirRoot())))
                break;

    //printf("VFS: GetFile '%s' -> '%s' (%s:%p)\n", fn, vf ? vf->fullname() : "NULL", vf ? vf->getType() : "?", vf);

    return vf;
}

inline static Dir *VFSHelper_GetDirByLoader(VFSLoader *ldr, const char *fn, const char *unmangled, Dir *root)
{
    if(!ldr)
        return NULL;
    Dir *vd = ldr->LoadDir(fn, unmangled);
    if(vd)
    {
        std::string parentname = fn;
        StripLastPath(parentname);

        Dir *parent = parentname.empty() ? root : root->getDir(parentname.c_str(), true);
        parent->insert(vd, true, Dir::NONE);
        --(vd->ref); // should delete it

        vd = root->getDir(fn); // can't return vd directly because it is cloned on insert+merge, and already deleted here
    }
    return vd;
}

Dir *VFSHelper::GetDir(const char* dn, bool create /* = false */)
{
    const char *unmangled = dn;
    std::string fixed = dn; // TODO: get rid of alloc
    FixPath(fixed);
    dn = fixed.c_str();

    if(!merged)
        return NULL;
    if(!*dn)
        return merged;
    Dir *vd = merged->getDir(dn);

    if(!vd && create)
    {
        if(!vd)
            for(LoaderArray::iterator it = loaders.begin(); it != loaders.end(); ++it)
                if((vd = VFSHelper_GetDirByLoader(*it, dn, unmangled, GetDirRoot())))
                    break;

        if(!vd)
            vd = merged->getDir(dn, true);
    }

    //printf("VFS: GetDir '%s' -> '%s' (%s:%p)\n", dn, vd ? vd->fullname() : "NULL", vd ? vd->getType() : "?", vd);

    return vd;
}

Dir *VFSHelper::GetDirRoot(void)
{
    return merged;
}

Dir *VFSHelper::GetBaseTree(const char *path)
{
    for(DirArray::iterator it = trees.begin(); it != trees.end(); ++it)
        if(!casecmp(it->source.c_str(), path))
            return it->dir;
    return NULL;
}

Dir *VFSHelper::GetMountPoint(const char *path)
{
    for(VFSMountList::iterator it = vlist.begin(); it != vlist.end(); ++it)
        if(!casecmp(it->mountPoint.c_str(), path))
            return it->vdir;
    return NULL;
}


void VFSHelper::ClearGarbage(void)
{
    for(DirArray::iterator it = trees.begin(); it != trees.end(); ++it)
        it->dir->clearGarbage();
}



// DEBUG STUFF


struct _DbgParams
{
    _DbgParams(std::ostream& os_, Dir *parent_, const std::string& sp_)
        : os(os_), parent(parent_), sp(sp_) {}

    std::ostream& os;
    Dir *parent;
    const std::string& sp;
};

static void _DumpFile(File *vf, void *user)
{
    _DbgParams& p = *((_DbgParams*)user);

    p.os << p.sp << "f|" << vf->name() << " [" << vf->getType() << ", ref " << vf->ref.count() << ", 0x" << vf << "]";

    if(strncmp(p.parent->fullname(), vf->fullname(), p.parent->fullnameLen()))
        p.os << " <-- {" << vf->fullname() << "} ***********";

    p.os << std::endl;
}

static void _DumpTreeRecursive(Dir *vd, void *user)
{
    _DbgParams& p = *((_DbgParams*)user);

    std::string sub = p.sp + "  ";

    p.os << p.sp << "d|" << vd->name() << " [" << vd->getType() << ", ref " << vd->ref.count() << ", 0x" << vd << "]";

    if(p.parent && strncmp(p.parent->fullname(), vd->fullname(), strlen(p.parent->fullname())))
        p.os << " <-- {" << vd->fullname() << "} ***********";
    p.os << std::endl;

    _DbgParams recP(p.os, vd, sub);

    vd->forEachDir(_DumpTreeRecursive, &recP);

    vd->forEachFile(_DumpFile, &recP);

}

void VFSHelper::debugDumpTree(std::ostream& os, Dir *start /* = NULL */)
{
    _DbgParams recP(os, NULL, "");
    Dir *d = start ? start : GetDirRoot();
    _DumpTreeRecursive(d, &recP);
}


VFS_NAMESPACE_END

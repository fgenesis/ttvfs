
/* ttvfs example #6 - System specific directories and mount points */

#include <VFS.h>
#include <iostream>

static void FileCallback(ttvfs::VFSFile *vf, void * /*unused*/)
{
    std::cout << vf->name() << " --> " << vf->fullname() << std::endl;
}


int main(int argc, char *argv[])
{
    ttvfs::VFSHelper vfs;

    vfs.LoadFileSysRoot(false);
    vfs.Prepare();

    std::cout << "Possible app dir: " << ttvfs::GetAppDir("example6") << std::endl;

    // Make the user's home directory accessible as "./user"
    vfs.MountExternalPath(ttvfs::GetUserDir().c_str(), "user");

    // Should be there now
    ttvfs::VFSDir *vd = vfs.GetDir("user");
    if(!vd)
    {
        std::cout << "ERROR" << std::endl;
        return 1;
    }

    // Where is it?
    // Because vd->fullname() returns "user" only, we have to find out the original mount point
    // and use that instead -- MountExternalPath() "hides" the original directory and just merges its contents!
    ttvfs::VFSDir *mp = vfs.GetMountPoint("user");
    if(mp)
        std::cout << "User directory: " << mp->fullname() << std::endl;
    else
        std::cout << "ERROR: Mount point not found (internal error!)" << std::endl;

    // List all files
    vd->forEachFile(FileCallback);

    // Some more testing.
    vfs.debugDumpTree(std::cout);


    return 0;
}


/* ttvfs example #2 - init, mounting and basic tree operations */

#include <iostream>
#include <VFS.h>

ttvfs::VFSHelper vfs;

static void PrintFile(const char *fn)
{
    ttvfs::VFSFile *vf = vfs.GetFile(fn);
    std::cout << "Open file " << fn << ": ";
    if(vf)
    {
        vf->open("r"); // force text mode
        std::cout << (const char*)vf->getBuf() << std::endl;
        vf->dropBuf(true); // no longer needed
        vf->close();
    }
    else
    {
        std::cout << "FILE OPEN ERROR!" << std::endl;
    }
}

int main(int argc, char *argv[])
{
    // this should be checked, especially in larger projects
    if(!ttvfs::checkCompat())
    {
        std::cout << "HUH? ttvfs was compiled with different options than this file!" << std::endl;
        return 1;
    }

    vfs.LoadFileSysRoot();
    vfs.Prepare();
    
    PrintFile("myfile.txt"); // this is the default file
    PrintFile("patches/myfile.txt"); // this is the patch file

    std::cout << "-- Mounting 'patches' -> ''" << std::endl;

    // merge "patches" into root dir
    //vfs.AddPath("patches"); // this works, but recreates parts of the tree
                              // that are already existing - possibly error prone!

    vfs.Mount("patches", ""); // <-- this is the better way.
    // all files and subdirs that were in "patches" are now mirrored in "" as well.

    PrintFile("myfile.txt"); // Access the file as before -> it got replaced.

    std::cout << "-- Before mounting 'more/even_more/deep' -> 'far' (should error)" << std::endl;

    PrintFile("far/file.txt"); // not found!

    // remount a directory under a different name
    vfs.Mount("more/even_more/deep", "far");

    std::cout << "-- After mounting 'more/even_more/deep' -> 'far'" << std::endl;

    PrintFile("far/file.txt"); // ... and access this file normally

    // mount an external directory (this could be ~/.MyApp or anything)
    vfs.MountExternalPath("../ttvfs", "ext");

    ttvfs::VFSDir *ext = vfs.GetDir("ext");
    if(ext)
    {
        //VFS_GUARD(ext); // in case this would be called from multiple threads, lock this directory.

        std::cout << "Listing files in 'ext' subdir ..." << std::endl;

        for(ttvfs::FileIter it = ext->fileIter(); it != ext->fileIterEnd(); ++it)
            std::cout << it->second->name() << " --> " << it->second->fullname() << std::endl;
    }
    
    return 0;
}

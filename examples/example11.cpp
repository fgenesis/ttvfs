
/* ttvfs example #1 - Embedding a zip file in the code */

#include <stdio.h>
#include <ttvfs.h>
#include <ttvfs_zip.h>

#include "res.h"

int main(int argc, char *argv[])
{
    ttvfs::Root vfs;
    // Note that we're not adding a disk loader here.
    vfs.AddArchiveLoader(new ttvfs::VFSZipArchiveLoader);

    // By default, the buffer is left untouched when the MemFile is deleted.
    // Use new because refcounting will delete the file when vfs is destroyed.
    // Use a CountedPtr so that we don't have to worry about cleaning up if something fails.
    // The file name is arbitrarily chosen, but the subdir representing the zip file
    // will have this name.
    ttvfs::CountedPtr<ttvfs::MemFile> mf = new ttvfs::MemFile("memdata.zip",
        ResourceData, ResourceSize);

    // Make all files from the archive available in the root.
    // The MemFile itself can NOT be accessed from the vfs root.
    if(!vfs.AddArchive(mf, ""))
    {
        puts("ERROR adding embedded archive");
        return 1;
    }

    char buf[513];
    size_t bytes = 0;
    ttvfs::File *vf = vfs.GetFile("a.txt");
    if(!vf || !vf->open("r") || !(bytes = vf->read(buf, 512)) )
    {
        puts("ERROR reading from embedded a.txt");
        return 2;
    }
    buf[bytes] = 0;
    puts(buf);

    vf = vfs.GetFile("b/c.txt");
    if(!vf || !vf->open("r") || !(bytes = vf->read(buf, 512)) )
    {
        puts("ERROR reading from embedded a.txt");
        return 2;
    }
    buf[bytes] = 0;
    puts(buf);

    return 0;
}


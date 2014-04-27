
/* ttvfs example #5 - Mounting  */

#include <iostream>
#include <stdio.h>
#include <ttvfs.h>
#include <ttvfs_zip.h>

static void dirCallback(ttvfs::DirBase *vd, void * /*unused*/)
{
    std::cout << "D : " << vd->name() << " --> " << vd->fullname() << std::endl;
}
static void fileCallback(ttvfs::File *vf, void * /*unused*/)
{
    std::cout << " F: " << vf->name() << " --> " << vf->fullname() << std::endl;
}

static void showDir(ttvfs::Root& vfs, const char *path)
{
    ttvfs::DirView view;
    vfs.FillDirView(path, view);
    view.forEachDir(dirCallback);
    view.forEachFile(fileCallback);
}

int main(int argc, char *argv[])
{
    ttvfs::Root vfs;
    vfs.AddLoader(new ttvfs::DiskLoader);
    vfs.AddArchiveLoader(new ttvfs::VFSZipArchiveLoader);

    // Merge "patches" dir into root dir, virtually overwriting files
    vfs.Mount("patches", "");

    // First, load patches/test.zip
    // (remember, patches/test.zip "overwrites" ./test.zip)
    vfs.AddArchive("test.zip");

    // Then, make its contents available in the root dir
    vfs.Mount("test.zip", "");

    // Expected: myfile.txt inside patches/test.zip.
    ttvfs::File *vf = vfs.GetFile("myfile.txt");
    if(!vf || !vf->open("r"))
    {
        puts("ERROR 1\n");
        return 1; 
    }
    char buf[513];
    size_t bytes = vf->read(buf, 512);
    buf[bytes] = 0;
    puts(buf);


    // Some more fun.
    // Mount a path from inside the zip archive to the root directory.
    // Note that its contents was already mounted to the root dir,
    // so it will get the mounted subdir from there.
    vfs.Mount("pets", "");

    // Let's see where all the files now in the root dir come from.
    showDir(vfs, "");
    puts("-------");

    // Expected: WOOF WOOF!
    vf = vfs.GetFile("dog.txt");
    if(!vf || !vf->open("r"))
    {
        puts("ERROR 2\n");
        return 1;
    }
    bytes = vf->read(buf, 512);
    buf[bytes] = 0;
    puts(buf);

    return 0;
}

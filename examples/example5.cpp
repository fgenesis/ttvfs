
/* ttvfs example #5 - Mounting  */

#include <cstdio>
#include <ttvfs.h>
#include <VFSZipArchiveLoader.h>

int main(int argc, char *argv[])
{
    ttvfs::VFSHelper vfs;

    vfs.AddLoader(new ttvfs::DiskLoader);

    // Make the VFS able to load Zip files
    vfs.AddArchiveLoader(new ttvfs::VFSZipArchiveLoader);

    // Merge "patches" dir into root dir, virtually overwriting files
    vfs.Mount("patches", "");

    // This time, do not mount as a subdir, but instead, unpack to the containing directory.
    // Note: The archive added here is the one really in the "patches" subdir!
    // Because it knows its own path (which is "patches/test.zip") and this is different from where we
    // actually want to mount it, we have to explicitly specify the path, which is the root dir ("").
    // (Otherwise, the zip archive would provide its contents in "patches/*")
    vfs.AddArchive("test.zip");
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
    // Note that the zip was already mounted to the root dir,
    // so it will get the mounted subdir from there.
    vfs.Mount("pets", "");

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

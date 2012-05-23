
/* ttvfs example #5 - Mounting  */

#include <cstdio>
#include <VFS.h>
#include <VFSZipArchiveLoader.h>

int main(int argc, char *argv[])
{
    ttvfs::VFSHelper vfs;

    // Make the VFS able to load Zip files
    vfs.AddArchiveLoader(new ttvfs::VFSZipArchiveLoader);

    // Load all files from current directory.
    // Here, it is again required to load everything recursively so that the mounting will work.
    vfs.LoadFileSysRoot(true);

    // Make the VFS usable
    vfs.Prepare();

    // Merge "patches" dir into root dir, virtually overwriting files
    vfs.Mount("patches", "");

    // This time, do not mount as a subdir, but instead, unpack to the containing directory.
    // Note: The archive added here is the one really in the "patches" subdir!
    // Because it knows its own path (which is "patches/test.zip") and this is different from where we
    // actually want to mount it, we have to explicitly specify the path, which is the root dir ("").
    // (Otherwise, the zip archive would provide its contents in "patches/*")
    vfs.AddArchive("test.zip", false, "");

    // Expected: myfile.txt inside patches/test.zip.
    ttvfs::VFSFile *vf = vfs.GetFile("myfile.txt");
    if(!vf)
    {
        puts("ERROR 1\n");
        return 1; 
    }
    puts((const char*)vf->getBuf());



    // Some more fun.
    // Mount a path from inside the zip archive to the root directory.
    // Note that the zip was already virtually unpacked to the root dir,
    // so this step is really trivial.
    vfs.Mount("pets", "");

    // Expected: WOOF WOOF!
    vf = vfs.GetFile("dog.txt");
    if(!vf)
    {
        puts("ERROR 2\n");
        return 1; 
    }
    puts((const char*)vf->getBuf());

    return 0;
}

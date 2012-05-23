
/* ttvfs example #4 - Example #1 re-done, now reading from a zip file. */

#include <cstdio>
#include <VFS.h>
#include <VFSZipArchiveLoader.h>

int main(int argc, char *argv[])
{
    ttvfs::VFSHelper vfs;

    // Make the VFS able to load Zip files
    vfs.AddArchiveLoader(new ttvfs::VFSZipArchiveLoader);

    // Load all files from current directory, subdirs not required in this case
    vfs.LoadFileSysRoot(false);

    // Make the VFS usable
    vfs.Prepare();

    // Mount an archive as a folder in the directory it resides in.
    // (Which is the default setting, and in this case, creates a virtual folder in the root dir.)
    vfs.AddArchive("test.zip");

    // Access the file as usual
    ttvfs::VFSFile *vf = vfs.GetFile("test.zip/zipped.txt");
    if(!vf)
    {
        puts("ERROR\n"); // failed to find file
        return 1; 
    }

    puts((const char*)vf->getBuf()); // dump to console

    return 0;
}

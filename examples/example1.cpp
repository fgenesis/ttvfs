
/* ttvfs example #1 - the most simple way to get started */

#include <cstdio>
#include <VFS.h>

int main(int argc, char *argv[])
{
    ttvfs::VFSHelper vfs;

    // Load all files from all subdirs, recursively
    vfs.LoadFileSysRoot();

    // Make the VFS usable
    vfs.Prepare();

    ttvfs::VFSFile *vf = vfs.GetFile("myfile.txt");
    if(!vf)
    {
        puts("ERROR\n"); // failed to find file
        return 1; 
    }

    puts((const char*)vf->getBuf()); // dump to console

    return 0;
}

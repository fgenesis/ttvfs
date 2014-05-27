
/* ttvfs example #8 - ttvfs classes are refcounted */

#include <ttvfs.h>
#include <cstdio>

// This example creates a VFS root to obtain a single file,
// then deletes the root before using the file.
// The file stays valid until the CountedPtr is destroyed.
// You can use CountedPtr::content() to obtain a raw pointer,
// but usually conversions are done automatically.
// Bottom line: If you store ttvfs objects in data structures
// and then modify the file tree, you might end up with dangling pointers.
// So use CountedPtr whenever you are going to keep hold of a file for a longer time.
// (Try returning ttvfs::File* instead of a CountedPtr -- it will crash!)
ttvfs::CountedPtr<ttvfs::File> openOneFile(const char *name, const char *mode)
{
    ttvfs::Root vfs;
    vfs.AddLoader(new ttvfs::DiskLoader);
    ttvfs::CountedPtr<ttvfs::File> vf = vfs.GetFile(name);
    if(!vf->open(mode))
        return NULL; // can simply return, the CountedPtr will clean up
    return vf; // <- pointer stays valid even after vfs is destroyed
}

int main(int argc, char *argv[])
{
    ttvfs::CountedPtr<ttvfs::File> vf = openOneFile("myfile.txt", "r");

    char buf[513];
    size_t bytes = vf->read(buf, 512);
    buf[bytes] = 0;
    puts(buf);

    vf->close();

    return 0;
}


/* ttvfs example #1 - the most simple way to get started */

#include <ttvfs.h>
#include <cstdio>

int main(int argc, char *argv[])
{
    ttvfs::Root vfs;
    vfs.AddLoader(new ttvfs::DiskLoader);

    ttvfs::File *vf = vfs.GetFile("myfile.txt");
    if(!vf)
    {
        printf("ERROR: myfile.txt does not exist\n");
        return 1;
    }
    if(!vf->open("r"))
    {
        printf("ERROR: Failed to open myfile.txt for reading\n");
        return 2;
    }

    char buf[513];
    size_t bytes = vf->read(buf, 512);
    buf[bytes] = 0;

    puts(buf);

    vf->close();

    return 0;
}

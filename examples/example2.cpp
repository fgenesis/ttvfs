
/* ttvfs example #2 - proper init, and listing files */

#include <iostream>
#include <ttvfs.h>

ttvfs::Root vfs;


static void fileCallback(ttvfs::File *vf, void *user)
{
    std::cout << "File: " << vf->name() << " --> " << vf->fullname() <<  " [" << vf->size() << " bytes]" << std::endl;

    // Known to be an int ptr.
    unsigned int *c = (unsigned int*)user;
    ++(*c);
}

int main(int argc, char *argv[])
{
    // this should be checked, especially in larger projects or when included as a dynamic library6
    if(!ttvfs::checkCompat())
    {
        std::cout << "HUH? ttvfs was compiled with different options than this file!" << std::endl;
        return 1;
    }

    vfs.AddLoader(new ttvfs::DiskLoader);

    // List files in working directory
    ttvfs::DirView view;
    if(vfs.FillDirView("", view))
    {
        std::cout << "Listing files in working dir ..." << std::endl;

        unsigned int c = 0;
        view.forEachFile(fileCallback, &c);

        std::cout << c << " files in total!" << std::endl;
    }
    else
        std::cout << "Invalid DirView: NO FILES?!" << std::endl;

    return 0;
}

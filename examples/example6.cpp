
/* ttvfs example #6 - System specific directories and mount points */

#include <ttvfs.h>
#include <iostream>

static void FileCallback(ttvfs::File *vf, void * /*unused*/)
{
    std::cout << vf->name() << " --> " << vf->fullname() << std::endl;
}


int main(int argc, char *argv[])
{
    ttvfs::Root vfs;

    vfs.AddLoader(new ttvfs::DiskLoader);

    // System-dependent suggested per-user program directory. On linux, this is the homedir,
    // on windows, this is most likely somewhere in %APPDATA%.
    std::cout << "Possible app dir: " << ttvfs::GetAppDir("example6") << std::endl << std::endl;


    // Make the user's home directory accessible as "./user"
    vfs.Mount(ttvfs::GetUserDir().c_str(), "user");

    // Should be there now
    ttvfs::DirView view;
    if(!vfs.FillDirView("user", view))
    {
        std::cout << "ERROR" << std::endl;
        return 1;
    }

    // List all files
    view.forEachFile(FileCallback);


    std::cout << std::endl << "--------------------------------" << std::endl;

    // Mount a relative path into an existing path on disk
    vfs.Mount("../ttvfs", "src");
    if(!vfs.FillDirView("src", view))
    {
        std::cout << "ERROR" << std::endl;
        return 2;
    }
    view.forEachFile(FileCallback);



    return 0;
}

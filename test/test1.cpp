

#include <ttvfs.h>
#include <cstdio>

template <typename T> static void assume(const T& what, const char *err)
{
    if(!what)
    {
        puts("##### FAILED #####");
        puts(err);
        exit(2);
    }
}

static void testmount_shared(ttvfs::Root& vfs)
{
    ttvfs::File *vf = vfs.GetFile("data/file.txt");
    assume(vf, "File not found");
    assume(vf->open("r"), "Failed to open");
    char buf[32];
    assume(vf->read(&buf, sizeof(buf)) == 1, "Failed to read");
    assume(buf[0] == 'B', "Mounted incorrectly - wrong file");
}


static bool testmount1()
{
    puts("- testmount1...");
    ttvfs::Root vfs;
    vfs.AddLoader(new ttvfs::DiskLoader);
    vfs.Mount("a/data", "data");
    vfs.Mount("b/data", "data");
    vfs.Mount("c/data", "data");
    testmount_shared(vfs);
    return true;
}

static bool testmount2()
{
    puts("- testmount2...");
    ttvfs::Root vfs;
    vfs.AddLoader(new ttvfs::DiskLoader);
    vfs.Mount("a", "");
    vfs.Mount("b", "");
    vfs.Mount("c", "");
    testmount_shared(vfs);
    return true;
}


int main(int argc, char *argv[])
{
    if (testmount1()
     && testmount2()
    ){
        puts("Tests passed!");
        return 0;
    }

    puts("FAIL?!");
    return 1;
}

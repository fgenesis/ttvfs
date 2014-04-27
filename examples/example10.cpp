
/* ttvfs example #10 - Embedding files & directories in the code */

#include <stdio.h>
#include <ttvfs.h>

static char memFile1[] = "File #1";
static char memFile2[] = { 1, 2, 3 };
static char memFile3[] = "Subdir file";
static char memFile4[] = "Hi, this is the in-memory myfile.txt";

static ttvfs::Root vfs;

void printFile(const char *fn)
{
    printf("=== Reading file '%s' ===\n", fn);
    char buf[513];
    size_t bytes = 0;
    ttvfs::File *vf = vfs.GetFile(fn);
    if(!vf || !vf->open("r") || !(bytes = vf->read(buf, 512)) )
    {
        printf(" *** ERROR reading file '%s' ***\n", fn);
        exit(1);
    }
    buf[bytes] = 0;
    printf("  Size: %u\n  Fullname: %s [%s]\n  Content: [%s]\n", (unsigned int)bytes, vf->fullname(), vf->getType(), buf);
}

int main(int argc, char *argv[])
{
    // Note that we're not adding any loaders for now.

    // Create a virtual in-memory directory and make it available in the root as "memory" subdir.
    ttvfs::MemDir *md = new ttvfs::MemDir("memory");
    vfs.AddVFSDir(md);

    // Populate with files. Paths in the file name are handled automatically,
    // and the files are added into the corresponding subdirs.
    md->add(new ttvfs::MemFile("memFile1",     memFile1, sizeof(memFile1)));
    md->add(new ttvfs::MemFile("memFile2",     memFile2, sizeof(memFile2)));
    md->add(new ttvfs::MemFile("sub/memFile3", memFile3, sizeof(memFile3))); // <- goes into subdir

    // Access as usual...
    printFile("memory/memFile1");
    printFile("memory/memFile2");
    printFile("memory/sub/memFile3");

    // Mounting works as usual, too...
    // Make "memory" available in the root
    vfs.Mount("memory", "");
    printFile("sub/memFile3");

    // Add another in-memory file.
    // Remember that "memory" is mounted in the root, so this file is immediately
    // available in the root, too.
    md->add(new ttvfs::MemFile("myfile.txt", memFile4, sizeof(memFile4)));
    printFile("myfile.txt");

    // Now comes the trick: Add the DiskLoader AFTER setting up the in-memory file system.
    // The loader will be mounted inside the root, AFTER "memory".
    // It will look for files in the reverse added order (most recently mounted first).
    // So disk comes first, then "memory".
    // Which means: If myfile.txt exists on the disk it will read that,
    // if it does not exist, it will use the in-memory file from above.
    vfs.AddLoader(new ttvfs::DiskLoader);
    printFile("myfile.txt");

    return 0;
}


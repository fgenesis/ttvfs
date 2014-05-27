
#include <ttvfs.h>
#include <cstdio>
#include <ctime>

ttvfs::Root vfs;

static bool lookupVFS(const char *fn, unsigned int times)
{
    ttvfs::File *vf = vfs.GetFile(fn);
    if(!vf)
        return false;
    for(unsigned int i = 0; i < times; ++i)
        vf = vfs.GetFile(fn);
    return true;
}

static bool openVFS(const char *fn, unsigned int times)
{
    ttvfs::File *vf = vfs.GetFile(fn);
    if(!vf)
        return false;
    for(unsigned int i = 0; i < times; ++i)
    {
        vf = vfs.GetFile(fn);
        vf->open("rb");
        vf->close();
    }
    return true;
}

static bool openfopen(const char *fn, unsigned int times)
{
    FILE *fh = fopen(fn, "rb");
    if(!fh)
        return false;
    for(unsigned int i = 0; i < times; ++i)
    {
        fh = fopen(fn, "rb");
        fclose(fh);
    }
    return true;
}

typedef bool (*bench_func)(const char *, unsigned int);

static void doRun(const char *fn, unsigned int times, bench_func f)
{
    clock_t ci = clock();
    if(!f(fn, times))
    {
        puts("FAIL!");
        return;
    }
    clock_t ce = clock();
    clock_t diff = ce - ci;
    printf("Time: %f ms\n", (diff * 1000.0f) / CLOCKS_PER_SEC);
}

int main(int argc, char *argv[])
{
    if(argc < 2 || !*argv[1])
    {
        puts("Specify a file name for repeated lookup!");
        return 1;
    }

    vfs.AddLoader(new ttvfs::DiskLoader);

    const char *fn =  argv[1];
    const int times = 100000;

    puts("Lookup only...");
    doRun(fn, times, lookupVFS);

    puts("VFS open...");
    doRun(fn, times, openVFS);

    puts("Plain fopen()...");
    doRun(fn, times, openfopen);

    return 0;
}

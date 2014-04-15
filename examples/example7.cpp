
/* ttvfs example #7 - Using the C-like override API.
   See comments in cfileapi/ttvfs_stdio.h for setup information. */


// ### CONFIG PART ###
// Change this to 0 to use the normal C stdio functions / C++ fstreams.
// This way, it is easy to test if the VFS and stdio produce identical results.
#define VFS_ENABLE_C_API 1



#include <ttvfs_stdio.h>
#include <ttvfs.h> // this does not need to be included if the file calls only vfopen() and friends

int main(int argc, char *argv[])
{
#if VFS_USING_C_API // <- This will be defined if VFS_ENABLE_C_API is on and the ttvfs overrides are in use.
    ttvfs::Root vfs;
    vfs.AddLoader(new ttvfs::DiskLoader);

    ttvfs_setroot(&vfs); // The C-like API supports only one (global) root object.
                         // Must be set before calling any of the functions,
                         // and must stay alive and unchanged while any VFILEs are opened.
    puts("Using VFS");
#else
    puts("Not using VFS");
#endif

    VFILE *fh = vfopen("myfile.txt", "r");
    if(!fh)
    {
        puts("ERROR: File not found: myfile.txt");
        return 1;
    }

    char buf[513];
    size_t bytes = vfread(buf, 1, 512, fh);
    buf[bytes] = 0;
    puts(buf);

    vfclose(fh);

    //----------------------------------

    InStream strm("mydata.txt");
    if(strm)
    {
        // The below is mostly copied from example #3
        unsigned int i, line = 0;
        float f;
        std::string s;
        while(true)
        {
            ++line;
            strm >> s >> i >> f; // we may hit EOF, causing any of the reads to fail
            if(!strm) // after reading, check if line was read in completely
                break;
            std::cout << "Line " << line << ": String '" << s << "', int " << i <<  ", float " << f << std::endl;
        }
        strm.close();
    }

    return 0;
}

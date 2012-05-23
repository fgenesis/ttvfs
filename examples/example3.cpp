
/* ttvfs example #3 - a quick std::ifstream replacement for simple text file reading */

#include <iostream>
#include <fstream>
#include <sstream>
#include <VFS.h>

ttvfs::VFSHelper vfs;


/* This class is a minimal adapter to support STL-like read-only file streams for VFS files,
 * using std::istringstream. Beware, it reads a whole file at once, use this only
 * if your text files are reasonably small!
*/
class VFSTextStdStreamIn : public std::istringstream
{
public:
    VFSTextStdStreamIn(const char *fn, bool dropOther = true);
    inline void close() {} // no-op
};

VFSTextStdStreamIn::VFSTextStdStreamIn(const char *fn, bool dropOther /* = true */)
: std::istringstream()
{
    ttvfs::VFSFile *vf = vfs.GetFile(fn);
    if(vf)
    {
        vf->open("r"); // force text mode reading
        str((char*)vf->getBuf()); // stringstream will always make a copy of the input buffer
        vf->close();

        // we may want to keep the memory if it is used elsewhere later,
        // to avoid re-reading from disk or other sources.
        // here, this is not required, so we delete it.
        if(dropOther) 
            vf->dropBuf(true);
    }
    else
    {
        // this will also cause std::istringstream's operator void*() to return NULL.
        this->setstate(std::ios_base::failbit); 
    }
}

template <typename T> void ReadAndPrint(const char *fn)
{
    unsigned int i, line = 0;
    float f;
    std::string s;

    T strm(fn);

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


int main(int argc, char *argv[])
{
    // this should be checked, especially in larger projects
    if(!ttvfs::checkCompat())
    {
        std::cout << "HUH? ttvfs was compiled with different options than this file!" << std::endl;
        return 1;
    }

    // first read, with standard std::ifstream
    std::cout << "-- Using std::ifstream --" << std::endl;
    ReadAndPrint<std::ifstream>("mydata.txt");

    // the obligatory VFS initialization
    vfs.LoadFileSysRoot(true);
    vfs.Prepare();

    // second read, using the VFS. Same code.
    // This is especially powerful if the files you are reading lie inside a container file, for example.
    std::cout << "-- Using VFSTextStdStreamIn --" << std::endl;
    ReadAndPrint<VFSTextStdStreamIn>("mydata.txt");

    return 0;
}

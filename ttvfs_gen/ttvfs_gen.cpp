#include <iostream>
#include <iomanip>
#include <fstream>
#include <algorithm>
#include <cstdlib>
#include <cstring>

#include <VFSTools.h>
#include <miniz.h>

ttvfs::StringList GetRecursiveFileList(const std::string& dirPath)
{
    ttvfs::StringList dirs;
    ttvfs::StringList allFiles;

    (void)ttvfs::GetDirList(dirPath.c_str(), dirs, -1);
    (void)ttvfs::GetFileList(dirPath.c_str(), allFiles);

    for(std::deque<std::string>::const_iterator it = dirs.begin();
        it != dirs.end(); ++it)
    {
        std::string externalDirPath = dirPath + std::string("/") + (*it);
        std::string internalDirPath = *it;

        ttvfs::FixPath(externalDirPath);

        ttvfs::StringList files;

        (void)ttvfs::GetFileList(externalDirPath.c_str(), files);

        for(std::deque<std::string>::const_iterator it2 = files.begin();
            it2 != files.end(); ++it2)
        {
            std::string internalPath = internalDirPath +
                std::string("/") + *it2;

            ttvfs::FixPath(internalPath);

            allFiles.push_back(internalPath);
        }
    }

    return allFiles;
}

int main(int argc, char *argv[])
{
    if(6 != argc)
    {
        std::cerr << "USAGE: " << argv[0] << " ARRAY_NAME SIZE_NAME "
            "DIR SOURCE HEADER" << std::endl;

        return EXIT_FAILURE;
    }

    std::string arrayName = argv[1];
    std::string sizeName = argv[2];
    std::string dirPath = argv[3];
    std::string sourcePath = argv[4];
    std::string headerPath = argv[5];
    std::string arrayNameUpper = arrayName;

    std::transform(arrayNameUpper.begin(), arrayNameUpper.end(),
        arrayNameUpper.begin(), ::toupper);

    ttvfs::StringList files = GetRecursiveFileList(dirPath);

    mz_zip_archive zip;
    memset(&zip, 0, sizeof(zip));

    if(MZ_FALSE == mz_zip_writer_init_heap(&zip, 0, 128 * 1024))
    {
        std::cerr << "mz_zip_writer_init_heap failed" << std::endl;

        return EXIT_FAILURE;
    }

    for(std::deque<std::string>::const_iterator it = files.begin();
        it != files.end(); ++it)
    {
        std::string externalPath = dirPath + std::string("/") + (*it);
        std::string internalPath = *it;

        ttvfs::FixPath(externalPath);

        if(MZ_FALSE == mz_zip_writer_add_file(&zip, internalPath.c_str(),
            externalPath.c_str(), NULL, 0, MZ_DEFAULT_COMPRESSION))
        {
            std::cerr << "mz_zip_writer_add_file failed on file "
                << internalPath << std::endl;

            return EXIT_FAILURE;
        }
    }

    unsigned char *pBuf;
    size_t size;

    if(MZ_FALSE == mz_zip_writer_finalize_heap_archive(&zip,
        (void**)&pBuf, &size))
    {
        std::cerr << "mz_zip_writer_finalize_heap_archive failed" << std::endl;

        return EXIT_FAILURE;
    }

    if(MZ_FALSE == mz_zip_writer_end(&zip))
    {
        std::cerr << "mz_zip_writer_end failed" << std::endl;

        return EXIT_FAILURE;
    }

    std::ofstream header;
    header.open(headerPath.c_str(), std::ofstream::out);

    if(!header.good())
    {
        std::cerr << "Failed to open header file " << headerPath << std::endl;

        return EXIT_FAILURE;
    }

    header << "/* THIS FILE IS GENERATED. DO NOT EDIT. "
        "DO NOT ADD TO CODE REPOSITORY. */" << std::endl;
    header << std::endl;
    header << "#ifndef " << arrayNameUpper << "_H" << std::endl;
    header << "#define " << arrayNameUpper << "_H" << std::endl;
    header << std::endl;
    header << "#include <stdio.h>" << std::endl;
    header << std::endl;
    header << "#ifdef __cplusplus" << std::endl;
    header << "extern \"C\" {" << std::endl;
    header << "#endif" << std::endl;
    header << std::endl;
    header << "/** Embedded resource zip file. */" << std::endl;
    header << "extern unsigned char " << arrayName << "[" << size
        << "];" << std::endl;
    header << std::endl;
    header << "/** Embedded resource zip file size. */" << std::endl;
    header << "extern size_t " << sizeName << ";" << std::endl;
    header << std::endl;
    header << "#ifdef __cplusplus" << std::endl;
    header << "}" << std::endl;
    header << "#endif" << std::endl;
    header << std::endl;
    header << "#endif /* " << arrayNameUpper << "_H */" << std::endl;

    std::ofstream source;
    source.open(sourcePath.c_str(), std::ofstream::out);

    if(!source.good())
    {
        std::cerr << "Failed to open header file " << sourcePath << std::endl;

        return EXIT_FAILURE;
    }

    source << "/* THIS FILE IS GENERATED. DO NOT EDIT. "
        "DO NOT ADD TO CODE REPOSITORY. */" << std::endl;
    source << std::endl;
    source << "#include <stdio.h>" << std::endl;
    source << std::endl;
    source << "unsigned char " << arrayName << "[" << size
        << "] = {" << std::endl;

    for(size_t i = 0; i < size; ++i)
    {
        if(0 == (i % 8))
        {
            source << "    ";
        }

        source << "0x" << std::hex << std::setw(2) << std::setfill('0')
            << ((unsigned int)pBuf[i]) << ",";

        if(i != (size - 1))
        {
            source << " ";
        }

        if(7 == (i % 8))
        {
            source << std::endl;
        }
    }

    if(0 != (size % 8))
    {
        source << std::endl;
    }

    source << "};" << std::endl;
    source << std::endl;
    source << "size_t " << sizeName << " = " << std::dec
        << size << ";" << std::endl;

    return EXIT_SUCCESS;
}

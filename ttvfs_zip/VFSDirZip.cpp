#include "VFSFileZip.h"
#include "VFSDirZip.h"
#include "VFSTools.h"

#include "VFSInternal.h"

#include "miniz.h"

VFS_NAMESPACE_START



inline std::string _joinPath(std::string base, const char *sub)
{
    if(!*sub)
        return base;
    if(*sub != '/')
        base += '/';
    return base + sub;
}

ZipDir::ZipDir(ZipArchiveRef *handle, const char *subpath)
: Dir(_joinPath(handle->fullname(), subpath).c_str(), NULL)
, _archiveHandle(handle)
{
}

ZipDir::~ZipDir()
{
    close();
}


void ZipDir::close(void)
{
    _archiveHandle->close();
}

DirBase *ZipDir::createNew(const char *dir) const
{
    const ZipArchiveRef *czref = _archiveHandle;
    ZipArchiveRef *zref = const_cast<ZipArchiveRef*>(czref);
    // FIXME: verify ctor is correct
    return new ZipDir(zref, dir);
}

#define MZ ((mz_zip_archive*)_archiveHandle->mz)

void ZipDir::load()
{
    close();

    unsigned int files = mz_zip_reader_get_num_files(MZ);

    mz_zip_archive_file_stat fs;
    for (unsigned int i = 0; i < files; ++i)
    {
        if(mz_zip_reader_is_file_encrypted(MZ, i))
            continue;
        if(!mz_zip_reader_file_stat(MZ, i, &fs))
            continue;
        if(mz_zip_reader_is_file_a_directory(MZ, i))
        {
            getDir(fs.m_filename, true);
            continue;
        }
        if(getFile(fs.m_filename))
            continue;

        ZipFile *vf = new ZipFile(fs.m_filename, _archiveHandle, (vfspos)fs.m_uncomp_size, fs.m_file_index); // TODO: stat
        addRecursive(vf); // FIXME: correct?
    }
}



VFS_NAMESPACE_END
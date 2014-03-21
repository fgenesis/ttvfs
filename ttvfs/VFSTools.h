// VFSTools.h - useful functions and misc stuff
// For conditions of distribution and use, see copyright notice in VFS.h

// Not all of these functions are used by ttvfs, but are added for user convenience.
// Everyone needs some path/file mangling functions at some point.

#ifndef VFS_TOOLS_H
#define VFS_TOOLS_H

#include <cstdlib>
#include <deque>
#include <string>

#include "VFSDefines.h"

VFS_NAMESPACE_START

typedef std::deque<std::string> StringList;

// these return false if the queried dir does not exist
bool GetFileList(const char *, StringList& files);
bool GetDirList(const char *, StringList& dirs, int depth = 0); // recursion depth: 0 = subdirs of current, 1 = subdirs one level down, ...,  -1 = deep recursion

bool FileExists(const char *);
bool IsDirectory(const char *);
bool CreateDir(const char*);
bool CreateDirRec(const char*);
vfspos GetFileSize(const char*);
void FixSlashes(std::string& s);
void FixPath(std::string& s);
const char *GetBaseNameFromPath(const char *str);
void MakeSlashTerminated(std::string& s);
void StripFileExtension(std::string& s);
void StripLastPath(std::string& s);
bool WildcardMatch(const char *str, const char *pattern);
size_t strnNLcpy(char *dst, const char *src, unsigned int n = -1);

template <class T> void StrSplit(const std::string &src, const std::string &sep, T& container, bool keepEmpty = false)
{
    std::string s;
    for (std::string::const_iterator i = src.begin(); i != src.end(); i++)
    {
        if (sep.find(*i) != std::string::npos)
        {
            if (keepEmpty || s.length())
                container.push_back(s);
            s = "";
        }
        else
        {
            s += *i;
        }
    }
    if (keepEmpty || s.length())
        container.push_back(s);
}

VFS_NAMESPACE_END

#endif


/* Test for directory listing correctness */

#include <VFS.h>
#include <VFSTools.h>
#include <cstdio>

int main(int argc, char *argv[])
{
	const char *dir = argc > 1 ? argv[1] : "";
	int depth = argc > 2 ? atoi(argv[2]) : 0;
	ttvfs::StringList sl;
	ttvfs::GetDirList(dir, sl, depth);

	printf("== [ \"%s\" -> %d entries ]==\n", dir, sl.size());

	for(ttvfs::StringList::iterator it = sl.begin(); it != sl.end(); ++it)
	{
		puts(it->c_str());
	}

	return 0;
}

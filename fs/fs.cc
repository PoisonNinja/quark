#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/path.h>

namespace Filesystem
{
void init()
{
    Filesystem::InodeCache::init();
    Filesystem::Path::resolve("/alpha/beta/gamma", 0, 0);
}
}

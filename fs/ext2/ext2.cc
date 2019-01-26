#include <fs/ext2/ext2.h>
#include <kernel/init.h>

namespace filesystem
{
namespace ext2
{
driver::driver()
{
}

driver::~driver()
{
}

bool driver::mount(superblock* sb)
{
    return false;
}

uint32_t driver::flags()
{
    return 0;
}

namespace
{
ext2::driver ext2_driver;
int init()
{
    filesystem::drivers::add("ext2", &ext2_driver);
    return 0;
}
FS_INITCALL(init);
} // namespace
} // namespace ext2
} // namespace filesystem
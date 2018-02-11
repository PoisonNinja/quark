#include <fs/fs.h>
#include <fs/inode.h>
#include <fs/path.h>
#include <kernel.h>
#include <lib/list.h>
#include <lib/string.h>

namespace Filesystem
{
static List<Driver, &Driver::node> filesystems;

status_t register_driver(Driver& driver)
{
    filesystems.push_front(driver);
    return SUCCESS;
}

status_t unregister_driver(Driver& driver)
{
    Log::printk(
        Log::WARNING,
        "unregister_driver called for %s but function not implemented\n",
        driver.name);
    return FAILURE;
}

Driver* get_driver(const char* name)
{
    for (auto& i : filesystems) {
        if (!String::strcmp(name, i.name)) {
            return &i;
        }
    }
    return nullptr;
}

void init()
{
    Filesystem::InodeCache::init();
    Filesystem::Path::resolve("/alpha/beta/gamma", 0, 0);
}
}

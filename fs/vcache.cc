#include <fs/vcache.h>
#include <fs/vnode.h>
#include <kernel.h>
#include <lib/murmur.h>
#include <lib/unordered_map.h>

namespace filesystem
{
namespace
{
class key
{
public:
    key(ino_t i, dev_t d)
        : ino(i)
        , dev(d){};
    key(const key& other) = default;
    key(key&& other)      = default;
    key& operator=(const key& other) = default;
    key& operator=(key&& other) = default;

    bool operator==(const key& k) const
    {
        return (ino == k.ino) && (dev == k.dev);
    }
    bool operator!=(const key& k) const
    {
        return !(*this == k);
    }

private:
    ino_t ino;
    dev_t dev;
};

struct hash {
    unsigned long operator()(const key& key)
    {
        return libcxx::murmur::hash(&key, sizeof(key));
    }
};

constexpr size_t vcache_size = 1024;

libcxx::unordered_map<key, libcxx::intrusive_ptr<vnode>, vcache_size, hash>
    vcache_hash;
} // namespace

namespace vcache
{
bool add(ino_t ino, dev_t dev, libcxx::intrusive_ptr<vnode> vnode)
{
    libcxx::intrusive_ptr<filesystem::vnode> dummy(nullptr);
    key key(ino, dev);
    if (vcache_hash.at(key, dummy)) {
        log::printk(log::log_level::WARNING,
                    "Vnode cache already has this vnode, discarding\n");
        return false;
    }
    vcache_hash.insert(key, vnode);
    return true;
}

libcxx::intrusive_ptr<vnode> get(ino_t ino, dev_t dev)
{
    libcxx::intrusive_ptr<vnode> dummy = libcxx::intrusive_ptr<vnode>(nullptr);
    key key(ino, dev);
    if (!vcache_hash.at(key, dummy)) {
        return libcxx::intrusive_ptr<vnode>(nullptr);
    }
    return dummy;
}
} // namespace vcache
} // namespace filesystem

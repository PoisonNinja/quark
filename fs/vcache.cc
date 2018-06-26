#include <fs/vcache.h>
#include <kernel.h>
#include <lib/hashmap.h>
#include <lib/murmur.h>

namespace Filesystem
{
namespace
{
constexpr size_t vcache_size = 1024;

class Key
{
public:
    Key(ino_t i, dev_t d) : ino(i), dev(d){};
    bool operator==(const Key& k) const
    {
        return (ino == k.ino) && (dev == k.dev);
    }
    bool operator!=(const Key& k) const
    {
        return !(*this == k);
    }

private:
    ino_t ino;
    dev_t dev;
};

struct Hash {
    unsigned long operator()(const Key& key)
    {
        return Murmur::hash(&key, sizeof(key)) % vcache_size;
    }
};

Hashmap<Key, Ref<Vnode>, vcache_size, Hash> vcache_hash;
}  // namespace

namespace VCache
{
bool add(ino_t ino, dev_t dev, Ref<Vnode> vnode)
{
    Ref<Vnode> dummy(nullptr);
    Key key(ino, dev);
    if (vcache_hash.get(key, dummy)) {
        Log::printk(Log::WARNING,
                    "Vnode cache already has this vnode, discarding\n");
        return false;
    }
    vcache_hash.put(key, vnode);
    return true;
}

Ref<Vnode> get(ino_t ino, dev_t dev)
{
    Ref<Vnode> dummy = Ref<Vnode>(nullptr);
    Key key(ino, dev);
    vcache_hash.get(key, dummy);
    return dummy;
}
}  // namespace VCache
}  // namespace Filesystem
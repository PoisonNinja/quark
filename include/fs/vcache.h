#pragma once

#include <fs/vnode.h>

namespace Filesystem
{
namespace VCache
{
bool add(ino_t ino, dev_t dev, Ref<Vnode> vnode);
Ref<Vnode> get(ino_t ino, dev_t dev);
}  // namespace VCache
}  // namespace Filesystem
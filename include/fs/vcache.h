#pragma once

#include <fs/vnode.h>

namespace filesystem
{
namespace VCache
{
bool add(ino_t ino, dev_t dev, libcxx::intrusive_ptr<Vnode> vnode);
libcxx::intrusive_ptr<Vnode> get(ino_t ino, dev_t dev);
} // namespace VCache
} // namespace filesystem

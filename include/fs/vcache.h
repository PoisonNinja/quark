#pragma once

#include <fs/vnode.h>

namespace filesystem
{
namespace vcache
{
bool add(ino_t ino, dev_t dev, libcxx::intrusive_ptr<vnode> vnode);
libcxx::intrusive_ptr<vnode> get(ino_t ino, dev_t dev);
} // namespace vcache
} // namespace filesystem

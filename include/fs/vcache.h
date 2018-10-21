#pragma once

#include <fs/vnode.h>
#include <memory>

namespace Filesystem
{
namespace VCache
{
bool add(ino_t ino, dev_t dev, std::shared_ptr<Vnode> vnode);
std::shared_ptr<Vnode> get(ino_t ino, dev_t dev);
} // namespace VCache
} // namespace Filesystem
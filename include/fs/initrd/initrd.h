#pragma once

#include <boot/info.h>
#include <types.h>

namespace Filesystem
{
namespace Initrd
{
void init(struct Boot::info& info);
}
}
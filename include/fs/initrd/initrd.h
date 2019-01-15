#pragma once

#include <boot/info.h>
#include <types.h>

namespace filesystem
{
namespace Initrd
{
void init(struct Boot::info& info);
}
}
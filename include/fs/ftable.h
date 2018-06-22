#pragma once

#include <fs/driver.h>

namespace Filesystem
{
namespace FTable
{
bool add(const char* name, Driver* driver);
Driver* get(const char* name);

}  // namespace FTable
}  // namespace Filesystem
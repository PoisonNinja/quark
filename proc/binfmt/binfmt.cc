#include <proc/binfmt/binfmt.h>

namespace binfmt
{
namespace
{
libcxx::list<binfmt, &binfmt::node> binfmts;
}

void add(binfmt& fmt)
{
    binfmts.push_back(fmt);
}

binfmt* get(libcxx::intrusive_ptr<filesystem::descriptor> file)
{
    for (auto& fmt : binfmts) {
        if (fmt.is_match(file)) {
            return &fmt;
        }
    }
    return nullptr;
}
} // namespace binfmt

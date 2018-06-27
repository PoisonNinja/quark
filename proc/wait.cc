#include <kernel.h>
#include <proc/sched.h>
#include <proc/wait.h>

namespace Scheduler
{
namespace
{
List<Waiter, &Waiter::node> sleep_queue;
}

bool Waiter::check(token_t token)
{
    return token == this->token;
}

bool broadcast(token_t token)
{
    for (auto& sleeper : sleep_queue) {
        if (sleeper.check(token)) {
            Log::printk(Log::WARNING, "Sleeper is ready to be woken, we should "
                                      "probably actually do it\n");
            return true;
        }
    }
    return false;
}

void add(Waiter& waiter)
{
    sleep_queue.push_back(waiter);
}
}  // namespace Scheduler
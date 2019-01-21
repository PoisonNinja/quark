#include <proc/ptable.h>

ptable::ptable()
{
    this->size = 0;
}

process* ptable::get(pid_t pid)
{
    for (auto wrapper : list) {
        if (wrapper.p->pid == pid) {
            return wrapper.p;
        }
    }
    return nullptr;
}

bool ptable::add(process* process)
{
    ptable_wrapper* wrapper = new ptable_wrapper(process);
    this->list.push_back(*wrapper);
    this->size++;
    return true;
}

bool ptable::remove(pid_t pid)
{
    for (auto it = list.begin(); it != list.end(); ++it) {
        auto& value = *it;
        if (value.p->pid == pid) {
            list.erase(it);
            this->size--;
            return true;
        }
    }
    return false;
}

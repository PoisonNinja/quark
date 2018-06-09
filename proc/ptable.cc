#include <proc/ptable.h>

PTable::PTable()
{
    this->size = 0;
}

Process* PTable::get(pid_t pid)
{
    for (auto wrapper : list) {
        if (wrapper.process->pid == pid) {
            return wrapper.process;
        }
    }
    return nullptr;
}

bool PTable::add(Process* process)
{
    PTableWrapper* wrapper = new PTableWrapper(process);
    this->list.push_back(*wrapper);
    this->size++;
    return true;
}

bool PTable::remove(pid_t pid)
{
    for (auto it = list.begin(); it != list.end(); ++it) {
        auto& value = *it;
        if (value.process->pid == pid) {
            list.erase(it);
            this->size--;
            return true;
        }
    }
    return false;
}
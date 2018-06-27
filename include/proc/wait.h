#pragma once

#include <lib/list.h>

class Thread;

namespace Scheduler
{
typedef void* token_t;

class Waiter
{
public:
    Waiter(Thread* t, token_t token) : thread(t), token(token){};
    virtual ~Waiter(){};
    virtual bool check(token_t token);
    Node<Waiter> node;

protected:
    Thread* thread;
    token_t token;
};

bool broadcast(token_t token);
void add(Waiter& waiter);
}  // namespace Scheduler
#include <kernel/console.h>

static struct list_element console_list = LIST_COMPILE_INIT(console_list);

status_t console_register(struct console* console)
{
    if (!console) {
        return FAILURE;
    }
    list_add(&console_list, &console->list);
    return SUCCESS;
}

status_t console_write(const char* buffer, size_t size)
{
    if (!buffer || !size) {
        return FAILURE;
    }

    status_t ret = SUCCESS;

    struct console* current = NULL;
    list_for_each(&console_list, list, current)
    {
        if (current->write) {
            if (current->write(buffer, size) != size) {
                ret = FAILURE;
            }
        }
    }
    return FAILURE;
}

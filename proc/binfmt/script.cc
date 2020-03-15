#include <errno.h>
#include <kernel/init.h>
#include <lib/string.h>
#include <proc/binfmt/binfmt.h>
#include <proc/sched.h>

namespace script
{
class Script : public binfmt::binfmt
{
public:
    virtual const char* name() override;
    virtual bool
    is_match(libcxx::intrusive_ptr<filesystem::descriptor> file) override;
    int load(struct ::binfmt::binprm& prm) override;
};

const char* Script::name()
{
    return "Script Interpreter";
}

bool Script::is_match(libcxx::intrusive_ptr<filesystem::descriptor> file)
{
    uint8_t buffer[2];
    file->pread(buffer, 2, 0);
    if (libcxx::memcmp(buffer, "#!", 2)) {
        return false;
    }
    return true;
}

int Script::load(struct ::binfmt::binprm& prm)
{
    // Capped at 128 chars for now
    // TODO: Move this out?
    uint8_t buffer[129];
    prm.file->pread(buffer, 128, 0);

    // For safety null terminate it
    buffer[128] = '\0';

    char* arg_start = nullptr;
    char* arg_end   = nullptr;

    log::printk(log::log_level::INFO, "Shebang is %s\n", buffer);

    // Bypass the #!
    char* cmd = reinterpret_cast<char*>(&(buffer[2]));

    while (*cmd != '\0' && *cmd != '\n' && *cmd == ' ') {
        cmd++;
    }

    if (*cmd == '\0' || *cmd == '\n') {
        return -EINVAL;
        // Empty shebang line
    }

    char* cmd_start = cmd;

    while (*cmd != '\0' && *cmd != '\n' && *cmd != ' ') {
        cmd++;
    }

    char* cmd_end = cmd;

    if (*cmd_end != '\0' && *cmd_end != '\n') {
        // Argument
        while (*cmd != '\0' && *cmd != '\n' && *cmd == ' ') {
            cmd++;
        }
        arg_start = cmd;

        while (*cmd != '\0' && *cmd != '\n' && *cmd != ' ') {
            cmd++;
        }

        arg_end = cmd;
    }
    *cmd_end = '\0';
    if (arg_end) {
        *arg_end = '\0';
    }

    log::printk(log::log_level::INFO, "script: Command is %s\n", cmd_start);

    if (arg_start) {
        log::printk(log::log_level::INFO, "script: Argument is %s\n",
                    arg_start);
    }

    // Time to construct the new argv
    int new_argc          = prm.argc + ((arg_start) ? 2 : 1);
    const char** new_argv = new const char*[new_argc];
    new_argv[0]           = cmd_start;
    new_argv[1]           = (arg_start) ? arg_start : prm.path;
    new_argv[2]           = (arg_start) ? prm.path : nullptr;
    for (int i = 3; i < new_argc; i++) {
        new_argv[i] = prm.argv[i - 2];
    }

    libcxx::intrusive_ptr<filesystem::descriptor> start(nullptr);
    if (*cmd_start == '/') {
        start = scheduler::get_current_process()->get_root();
    } else {
        start = start = scheduler::get_current_process()->get_cwd();
    }

    auto [err, f] = start->open(cmd_start, 0, 0);
    if (!f) {
        delete[] new_argv;
        return -1;
    }

    int ret = scheduler::get_current_process()->load(
        cmd_start, f, new_argc, new_argv, prm.envc, prm.envp, prm.ctx);

    delete[] new_argv;

    return ret;
}

namespace
{
Script script;

int init()
{
    binfmt::add(script);
    return 0;
}
CORE_INITCALL(init);
} // namespace
} // namespace script

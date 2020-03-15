#include <arch/mm/layout.h>
#include <errno.h>
#include <kernel.h>
#include <kernel/init.h>
#include <lib/string.h>
#include <mm/physical.h>
#include <mm/virtual.h>
#include <proc/binfmt/binfmt.h>
#include <proc/binfmt/elf.h>
#include <proc/process.h>
#include <proc/sched.h>
#include <proc/uthread.h>

extern "C" void signal_return();
void* signal_return_location = reinterpret_cast<void*>(&signal_return);

namespace elf
{
class ELF : public binfmt::binfmt
{
public:
    virtual const char* name() override;
    virtual bool
    is_match(libcxx::intrusive_ptr<filesystem::descriptor> file) override;
    int load(libcxx::intrusive_ptr<filesystem::descriptor> file, int argc,
             const char* argv[], int envc, const char* envp[],
             struct thread_context& ctx) override;
};

const char* ELF::name()
{
    return "ELF";
}

bool ELF::is_match(libcxx::intrusive_ptr<filesystem::descriptor> file)
{
    uint8_t buffer[sizeof(elf_ehdr)];
    file->pread(buffer, sizeof(elf_ehdr), 0);
    elf_ehdr* header = reinterpret_cast<elf_ehdr*>(buffer);
    if (libcxx::memcmp(header->e_ident, ELFMAG, 4)) {
        return false;
    }
    return true;
}

int ELF::load(libcxx::intrusive_ptr<filesystem::descriptor> file, int argc,
              const char* argv[], int envc, const char* envp[],
              struct thread_context& ctx)
{
    process* current_process = scheduler::get_current_process();

    uint8_t buffer[sizeof(elf_ehdr)];
    file->pread(buffer, sizeof(elf_ehdr), 0);
    process* process = scheduler::get_current_process();
    elf_ehdr* header = reinterpret_cast<elf_ehdr*>(buffer);
    if (libcxx::memcmp(header->e_ident, ELFMAG, 4)) {
        log::printk(log::log_level::ERROR,
                    "Binary passed in is not an ELF file!\n");
        return -EINVAL;
    }
    log::printk(log::log_level::DEBUG, "Section header offset: %p\n",
                header->e_shoff);
    log::printk(log::log_level::DEBUG, "Program header offset: %p\n",
                header->e_phoff);

    elf_phdr* tls_phdr = nullptr;

    for (int i = 0; i < header->e_phnum; i++) {
        uint8_t phdr_buffer[sizeof(elf_phdr)];
        file->pread(phdr_buffer, sizeof(elf_phdr),
                    header->e_phoff + (header->e_phentsize * i));
        elf_phdr* phdr = reinterpret_cast<elf_phdr*>(phdr_buffer);
        log::printk(log::log_level::DEBUG, "Header type: %X\n", phdr->p_type);
        if (phdr->p_type == PT_TLS) {
            log::printk(log::log_level::DEBUG, "Found TLS section\n");
            process->set_tls_data(phdr->p_offset, phdr->p_filesz, phdr->p_memsz,
                                  phdr->p_align);
            tls_phdr = phdr;
        }
        if (phdr->p_type == PT_LOAD) {
            log::printk(log::log_level::DEBUG, "Flags:            %X\n",
                        phdr->p_flags);
            log::printk(log::log_level::DEBUG, "Offset:           %p\n",
                        phdr->p_offset);
            log::printk(log::log_level::DEBUG, "Virtual address:  %p\n",
                        phdr->p_vaddr);
            log::printk(log::log_level::DEBUG, "Physical address: %p\n",
                        phdr->p_paddr);
            log::printk(log::log_level::DEBUG, "File size:        %p\n",
                        phdr->p_filesz);
            log::printk(log::log_level::DEBUG, "Memory size:      %p\n",
                        phdr->p_memsz);
            log::printk(log::log_level::DEBUG, "Align:            %p\n",
                        phdr->p_align);
            // Compute the mmap prot from ELF flags
            int prot = 0;
            if (phdr->p_flags & PF_R) {
                prot |= PROT_READ;
            }
            if (phdr->p_flags & PF_W) {
                prot |= PROT_WRITE;
            }
            if (phdr->p_flags & PF_X) {
                prot |= PROT_EXEC;
            }

            // Compute mmap flags
            int flags = MAP_FIXED | MAP_PRIVATE;

            process->mmap(phdr->p_vaddr, phdr->p_memsz, prot, flags, file,
                          phdr->p_offset);
            if (phdr->p_filesz < phdr->p_memsz) {
                // We're supposed to zero out the remaining
                log::printk(log::log_level::DEBUG,
                            "Memory size is larger than file size\n");
                /*
                 * There are two possible scenarios*
                 *
                 * First case, memsz will still fit into the same page as
                 * before, we don't need anything else.
                 *
                 * Second case, memsz will spill over into additional page(s),
                 * we'll simply map another anonymous page beyond.
                 */
                if (memory::virt::align_down(phdr->p_vaddr + phdr->p_memsz) ==
                    memory::virt::align_down(phdr->p_vaddr + phdr->p_filesz)) {
                    libcxx::memset(
                        reinterpret_cast<void*>(phdr->p_vaddr + phdr->p_filesz),
                        0, phdr->p_memsz - phdr->p_filesz);
                } else {
                    addr_t file_end = phdr->p_vaddr + phdr->p_filesz;
                    addr_t mem_end  = phdr->p_vaddr + phdr->p_memsz;
                    addr_t page_end =
                        memory::virt::align_up(phdr->p_vaddr + phdr->p_filesz);
                    libcxx::memset(reinterpret_cast<void*>(file_end), 0,
                                   page_end - file_end);
                    size_t to_map = memory::virt::align_up(mem_end - page_end);
                    process->mmap(page_end, to_map, prot, flags | MAP_ANONYMOUS,
                                  nullptr, 0);
                }
            }
        }
    }
    log::printk(log::log_level::DEBUG, "Entry point: %p\n", header->e_entry);
    // TODO: More sanity checks

    size_t argv_size = sizeof(char*) * (argc + 1); // argv is null terminated
    size_t envp_size = sizeof(char*) * (envc + 1); // envp is null terminated
    for (int i = 0; i < argc; i++) {
        argv_size += libcxx::strlen(argv[i]) + 1;
    };
    for (int i = 0; i < envc; i++) {
        envp_size += libcxx::strlen(envp[i]) + 1;
    }

    size_t tls_raw_size =
        libcxx::round_up(tls_phdr->p_memsz, tls_phdr->p_align);
    size_t tls_size = libcxx::round_up(tls_raw_size + sizeof(struct uthread),
                                       tls_phdr->p_align);

    void* argv_zone =
        current_process->mmap(USER_START, argv_size, PROT_READ | PROT_WRITE,
                              MAP_ANONYMOUS, nullptr, 0);
    if (argv_zone) {
        log::printk(log::log_level::DEBUG, "argv located at %p\n", argv_zone);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate argv\n");
        return false;
    }
    void* envp_zone =
        current_process->mmap(USER_START, envp_size, PROT_READ | PROT_WRITE,
                              MAP_ANONYMOUS, nullptr, 0);
    if (envp_zone) {
        log::printk(log::log_level::DEBUG, "envp located at %p\n", envp_zone);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate envp\n");
        return false;
    }
    void* stack_zone =
        current_process->mmap(USER_START, 0x1000, PROT_READ | PROT_WRITE,
                              MAP_ANONYMOUS | MAP_GROWSDOWN, nullptr, 0);
    if (stack_zone) {
        log::printk(log::log_level::DEBUG, "Stack located at %p\n", stack_zone);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate stack\n");
        return false;
    }
    void* sigreturn_zone = current_process->mmap(
        USER_START, 0x1000, PROT_READ | PROT_WRITE | PROT_EXEC, MAP_ANONYMOUS,
        nullptr, 0);
    if (sigreturn_zone) {
        log::printk(log::log_level::DEBUG, "Sigreturn page located at %p\n",
                    sigreturn_zone);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate sigreturn page\n");
        return false;
    }
    void* tls_zone =
        current_process->mmap(USER_START, tls_size, PROT_READ | PROT_WRITE,
                              MAP_ANONYMOUS, nullptr, 0);
    if (tls_zone) {
        log::printk(log::log_level::DEBUG, "TLS copy located at %p\n",
                    tls_zone);
    } else {
        log::printk(log::log_level::ERROR, "Failed to locate TLS copy\n");
        return false;
    }

    current_process->set_sigreturn(reinterpret_cast<addr_t>(sigreturn_zone));

    // Copy in sigreturn trampoline code
    libcxx::memcpy(reinterpret_cast<void*>(sigreturn_zone),
                   signal_return_location, 0x1000);

    // Make it unwritable
    memory::virt::protect(reinterpret_cast<addr_t>(sigreturn_zone), PAGE_USER);

    // Copy TLS data into thread specific data
    libcxx::memcpy(reinterpret_cast<void*>(tls_zone),
                   reinterpret_cast<void*>(tls_phdr->p_offset),
                   tls_phdr->p_filesz);
    libcxx::memset(reinterpret_cast<void*>(tls_zone + tls_phdr->p_filesz), 0,
                   tls_phdr->p_memsz - tls_phdr->p_filesz);

    struct uthread* uthread =
        reinterpret_cast<struct uthread*>(tls_zone + tls_raw_size);
    uthread->self = uthread;

    char* target =
        reinterpret_cast<char*>(argv_zone + (sizeof(char*) * (argc + 1)));
    char** target_argv = reinterpret_cast<char**>(argv_zone);
    for (int i = 0; i < argc; i++) {
        libcxx::strcpy(target, argv[i]);
        target_argv[i] = target;
        target += libcxx::strlen(argv[i]) + 1;
    }
    target_argv[argc] = 0;
    target = reinterpret_cast<char*>(envp_zone + (sizeof(char*) * (envc + 1)));
    char** target_envp = reinterpret_cast<char**>(envp_zone);
    for (int i = 0; i < envc; i++) {
        libcxx::strcpy(target, envp[i]);
        target_envp[i] = target;
        target += libcxx::strlen(envp[i]) + 1;
    }
    target_envp[envc] = 0;
    libcxx::memset(reinterpret_cast<void*>(stack_zone), 0, 0x1000);

    // TODO: Move to architecture
    libcxx::memset(&ctx, 0, sizeof(ctx));
    ctx.rip = header->e_entry;
    ctx.rdi = argc;
    ctx.rsi = reinterpret_cast<uint64_t>(target_argv);
    ctx.rdx = reinterpret_cast<uint64_t>(target_envp);
    ctx.cs  = 0x20 | 3;
    ctx.ds  = 0x18 | 3;
    ctx.ss  = 0x18 | 3;
    ctx.fs  = reinterpret_cast<uint64_t>(uthread);
    ctx.rsp = ctx.rbp = reinterpret_cast<uint64_t>(stack_zone) + 0x1000;
    ctx.rflags        = 0x200;
    scheduler::get_current_thread()->set_context(ctx);

    return 0;
}

namespace
{
ELF elf;

int init()
{
    binfmt::add(elf);
    return 0;
}
CORE_INITCALL(init);
} // namespace
} // namespace elf

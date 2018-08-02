#include <boot/info.h>
#include <cpu/cpu.h>
#include <cpu/interrupt.h>
#include <fs/fs.h>
#include <fs/initrd/initrd.h>
#include <fs/stat.h>
#include <kernel.h>
#include <kernel/init.h>
#include <kernel/time/time.h>
#include <kernel/version.h>
#include <lib/string.h>
#include <mm/mm.h>
#include <mm/virtual.h>
#include <proc/sched.h>
#include <proc/syscall.h>

#include <kernel/symbol.h>
#include <proc/elf.h>

void h(int a)
{
    Log::printk(Log::LogLevel::INFO, "In there: %d!\n", a);
    for (;;)
        asm("hlt");
}

static void load_module()
{
    void* load_target = nullptr;
    Process* parent = Scheduler::get_current_process();
    Ref<Filesystem::Descriptor> root = parent->get_root();
    Ref<Filesystem::Descriptor> init = root->open("/sbin/test.ko", 0, 0);
    if (!init) {
        Log::printk(Log::LogLevel::ERROR, "Failed to open test.ko\n");
        for (;;)
            CPU::halt();
    }
    struct Filesystem::stat st;
    init->stat(&st);
    Log::printk(Log::LogLevel::DEBUG, "test.ko has size of %zu bytes\n",
                st.st_size);
    uint8_t* init_raw = new uint8_t[st.st_size];
    init->read(init_raw, st.st_size);

    ELF::Elf_Ehdr* header = reinterpret_cast<ELF::Elf_Ehdr*>(init_raw);
    if (String::memcmp(header->e_ident, ELFMAG, 4)) {
        Log::printk(Log::LogLevel::ERROR,
                    "Binary passed in is not an ELF file!\n");
        return;
    }
    ELF::Elf_Shdr* shdr_base = (ELF::Elf_Shdr*)(init_raw + header->e_shoff);
    ELF::Elf_Shdr* shdr_string_table = shdr_base;
    Log::printk(Log::LogLevel::INFO, "Header: %p %X\n", shdr_string_table,
                header->e_shoff);
    shdr_string_table += header->e_shstrndx;
    Log::printk(Log::LogLevel::INFO, "Section string table: %p %X %p\n",
                shdr_string_table, shdr_string_table->sh_type,
                shdr_string_table->sh_offset);
    // ELF::Elf_Shdr*
    char* s_string_table = (char*)(init_raw + shdr_string_table->sh_offset);

    ELF::Elf_Shdr* string_table_header = nullptr;
    char* string_table = nullptr;
    for (uint32_t i = 0; i < header->e_shnum; i++) {
        ELF::Elf_Shdr* shdr = (ELF::Elf_Shdr*)shdr_base + i;
        if (shdr->sh_type == SHT_STRTAB &&
            !String::strcmp(".strtab", s_string_table + shdr->sh_name)) {
            string_table_header = (ELF::Elf_Shdr*)shdr_base + i;
            string_table = (char*)(init_raw + string_table_header->sh_offset);
        }
    }

    Log::printk(Log::LogLevel::INFO, "String table at %p\n", string_table);

    ELF::Elf_Sym* symtab;
    size_t num_syms;
    ELF::Elf_Shdr* shdr_info;
    for (uint32_t i = 0; i < header->e_shnum; i++) {
        ELF::Elf_Shdr* shdr = (ELF::Elf_Shdr*)shdr_base + i;
        if (shdr->sh_type == SHT_SYMTAB) {
            symtab = (ELF::Elf_Sym*)(init_raw + shdr->sh_offset);
            num_syms = shdr->sh_size / shdr->sh_entsize;
        }
    }

    for (uint32_t i = 0; i < header->e_shnum; i++) {
        ELF::Elf_Shdr* shdr = (ELF::Elf_Shdr*)shdr_base + i;
        if (shdr->sh_type == SHT_NOBITS) {
            shdr->sh_addr = (ELF::Elf64_Addr) new uint8_t[shdr->sh_size];
            String::memset((void*)shdr->sh_addr, 0x00, shdr->sh_size);
        } else {
            shdr->sh_addr = (ELF::Elf64_Addr)header + shdr->sh_offset;
        }
    }

    for (uint32_t i = 0; i < header->e_shnum; i++) {
        ELF::Elf_Shdr* shdr = (ELF::Elf_Shdr*)shdr_base + i;
        if (shdr->sh_type == SHT_RELA) {
            for (uint32_t x = 0; x < shdr->sh_size; x += shdr->sh_entsize) {
                ELF::Elf_Rela* rel =
                    (ELF::Elf_Rela*)(init_raw + shdr->sh_offset + x);
                ELF::Elf_Sym* sym = symtab + ELF_R_SYM(rel->r_info);
                Log::printk(Log::LogLevel::INFO, "Name: %s\n",
                            string_table + sym->st_name);
                addr_t symaddr = 0;
                if (sym->st_shndx == 0) {
                    addr_t temp =
                        Symbols::resolve_name(string_table + sym->st_name);
                    if (!temp) {
                        Log::printk(Log::LogLevel::ERROR,
                                    "Undefined reference to %s\n",
                                    string_table + sym->st_name);
                        return;
                    }
                    symaddr = temp;
                } else if (sym->st_shndx) {
                    symaddr =
                        (shdr_base + sym->st_shndx)->sh_addr + sym->st_value;
                }
                // TODO: Resolve local addresses
                switch (ELF_R_TYPE(rel->r_info)) {
                    case R_X86_64_64:
                        Log::printk(Log::LogLevel::INFO,
                                    "R_X86_64_64: %p %p %p %X\n", rel->r_addend,
                                    rel->r_offset, symaddr, shdr->sh_info);
                        break;
                    default:
                        Log::printk(Log::LogLevel::ERROR,
                                    "Unsupported relocation type: %d\n",
                                    ELF_R_TYPE(rel->r_info));
                        break;
                }
                addr_t target = (shdr_base + shdr->sh_info)->sh_addr;
                target += rel->r_offset;
                *((uint64_t*)target) = symaddr + rel->r_addend;
            }
        }
    }
    ELF::Elf_Sym* sym = symtab;
    for (int j = 1; j <= num_syms; j++) {
        sym++;
        if (ELF64_ST_TYPE(sym->st_info) != STT_FUNC)
            continue;
        if (!String::strcmp(string_table + sym->st_name, "init")) {
            Log::printk(Log::LogLevel::INFO, "Located entry point at %p\n",
                        (shdr_base + sym->st_shndx)->sh_addr + sym->st_value);
            break;
        }
    }
    int (*mod)() =
        (int (*)())((shdr_base + sym->st_shndx)->sh_addr + sym->st_value);
    mod();
}

void init_stage2(void*)
{
    Process* parent = Scheduler::get_current_process();
    Ref<Filesystem::Descriptor> root = parent->get_root();
    Ref<Filesystem::Descriptor> init = root->open("/sbin/init", 0, 0);
    if (!init) {
        Log::printk(Log::LogLevel::ERROR, "Failed to open init\n");
        for (;;)
            CPU::halt();
    }
    struct Filesystem::stat st;
    init->stat(&st);
    Log::printk(Log::LogLevel::DEBUG, "init binary has size of %zu bytes\n",
                st.st_size);
    uint8_t* init_raw = new uint8_t[st.st_size];
    init->read(init_raw, st.st_size);
    int argc = 2;
    const char* argv[] = {
        "/sbin/init",
        "test",
    };
    int envc = 1;
    const char* envp[] = {
        "hello=world",
    };
    struct ThreadContext ctx;
    if (!Scheduler::get_current_thread()->load(
            reinterpret_cast<addr_t>(init_raw), argc, argv, envc, envp, ctx)) {
        Log::printk(Log::LogLevel::ERROR, "Failed to load thread state\n");
    } else {
        Log::printk(Log::LogLevel::DEBUG, "Preparing to jump into userspace\n");
    }
    delete[] init_raw;
    load_registers(ctx);
}

void init_stage1()
{
    addr_t cloned = Memory::Virtual::fork();
    Process* initp = new Process(nullptr);
    Scheduler::add_process(initp);
    initp->set_root(Scheduler::get_current_process()->get_root());
    initp->set_cwd(Scheduler::get_current_process()->get_cwd());
    initp->set_dtable(Ref<Filesystem::DTable>(new Filesystem::DTable));
    initp->address_space = cloned;

    Thread* stage2 = create_kernel_thread(initp, init_stage2, nullptr);
    Scheduler::insert(stage2);
    Scheduler::idle();
}

void kmain(struct Boot::info& info)
{
    Log::printk(Log::LogLevel::INFO, "%s\n", OS_STRING);
    Log::printk(Log::LogLevel::INFO, "Command line: %s\n", info.cmdline);
    Memory::init(info);
    Interrupt::init();
    Interrupt::enable();
    Time::init();
    Scheduler::init();
    Signal::init();
    Filesystem::init();
    Filesystem::Initrd::init(info);
    Syscall::init();

    /*
     * Core subsystems are online, let's starting bringing up the rest of
     * the kernel.
     */
    do_initcall(InitLevel::EARLY);
    do_initcall(InitLevel::CORE);
    do_initcall(InitLevel::ARCH);
    do_initcall(InitLevel::SUBSYS);
    do_initcall(InitLevel::FS);
    do_initcall(InitLevel::DEVICE);
    do_initcall(InitLevel::LATE);

    load_module();

    init_stage1();
}

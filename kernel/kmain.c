#include <boot/limine.h>
#include <utils/common.h>

#include <cpu/gdt.h>
#include <cpu/idt.h>
#include <cpu/cpu.h>

#include <mem/pmm.h>
#include <mem/vmm.h>

#include <tasking/scheduler.h>

#include <devices/pci.h>
#include <devices/timer.h>

#include <cpu/acpi.h>

#include <fs/vfs.h>
#include <fs/partfs/partfs.h>

#include <modules/modules.h>

limine_stack_size_request_t limine_stack_size_request = {
    .header = {
        .id = LIMINE_STACK_SIZE_REQUEST,
        .revision = 0,
        .response = NULL
    },
    .stack_size = 0x10000
};

__attribute__((section(".limine_reqs")))
void *stack_size_req = &limine_stack_size_request;

limine_memmap_request_t limine_memmap_request = {
    .header = {
        .id = LIMINE_MEMMAP_REQUEST,
        .revision = 0,
        .response = NULL
    }
};

__attribute__((section(".limine_reqs")))
void *memmap_req = &limine_memmap_request;

limine_kernel_address_request_t limine_kernel_address_request = {
    .header = {
        .id = LIMINE_KERNEL_ADDRESS_REQUEST,
        .revision = 0,
        .response = NULL
    }
};

__attribute__((section(".limine_reqs")))
void *kernel_address_req = &limine_kernel_address_request;

limine_rsdp_request_t limine_rsdp_request = {
    .header = {
        .id = LIMINE_RSDP_REQUEST,
        .revision = 0,
        .response = NULL
    }
};

__attribute__((section(".limine_reqs")))
void *rsdp_req = &limine_rsdp_request;

void kentry_threaded();

void kentry() {
    printf("You can break now\n");
    gdt_init();

    limine_memmap_response_t *memmap = (limine_memmap_response_t *)limine_memmap_request.header.response;
    pmm_init(memmap->entries, memmap->entry_count);
    limine_kernel_address_response_t *kaddr = (limine_kernel_address_response_t *)limine_kernel_address_request.header.response;
    vmm_init((void*)kaddr->virtual_base, (void*)kaddr->physical_base, memmap->entries, memmap->entry_count);

    idt_init();

    acpi_init(((limine_rsdp_response_t *)limine_rsdp_request.header.response)->address);
    pci_preinit();

    acpi_post_init();
    timer_init();
    smp_init();

    scheduler_init(kentry_threaded);

    while(1);
}

void kentry_threaded() {
    printf("[Kentry] Threading reached\n");

    vfs_init();

    pci_init();

    partfs_init("nvme0");

    vfs_node_t *fat32 = kopen("/dev/fat32", 0);
    vfs_node_mount(fat32, "/dev/nvme0p0", 0, "/", NULL);
    vfs_close(fat32);
    kfree(fat32);

    //vfs_print_tree();
    modules_init();

    module_load("/test.wko");
    module_load("/virtio.wko");
    module_load("/virtiogpu.wko");
    module_load("/packetfs.wko");
    module_load("/shm.wko");

    /*vfs_node_t *testfat = kopen("/test/long_name_test.txt", 0);
    vfs_lseek(testfat, 0, SEEK_SET);
    vfs_read(testfat, testArray, 2056);
    printf("%2056s\n", testArray);*/

    const char *args[] = { "/tests/test2", NULL };
    const char *environ[] = { NULL };

    pt_t *pml4 = vmm_setup_pml4();
    kpid_t pid = process_create(pml4);

    vfs_node_t *logNode = kopen("/dev/log", 0);
    SchedulerProcesses[pid]->file_handles[0] = logNode;
    SchedulerProcesses[pid]->file_handles[1] = logNode;
    SchedulerProcesses[pid]->file_handles[2] = logNode;
    exec(pid, "/tests/test3", args, environ);

    printf("Init Done pid:%d, page_table:%llx\n", pid, SchedulerProcesses[pid]->page_table->entries);

    while(1);
}
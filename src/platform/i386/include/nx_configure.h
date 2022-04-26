#ifndef __NX_CONFIG__
#define __NX_CONFIG__
#define CONFIG_NX_DEBUG 1
#define CONFIG_NX_LOG_LEVEL 3
#define CONFIG_NX_DEBUG_COLOR 1
#define CONFIG_NX_DEBUG_TIMELINE 1
#define CONFIG_NX_PLATFORM_NAME "x86-i386"
#define CONFIG_NX_MULTI_CORES_NR 1
#define CONFIG_NX_ENABLE_PLATFORM_MAIN 1
#define CONFIG_NX_IRQ_NAME_LEN 48
#define CONFIG_NX_NR_IRQS 16
#define CONFIG_NX_KVADDR_OFFSET 0x00000000
#define CONFIG_NX_PAGE_SHIFT 12
#define CONFIG_NX_MAX_THREAD_NR 256
#define CONFIG_NX_THREAD_NAME_LEN 32
#define CONFIG_NX_THREAD_STACK_SIZE 8192
#define CONFIG_NX_ENABLE_SCHED 1
#define CONFIG_NX_THREAD_MAX_PRIORITY_NR 16
#define CONFIG_NX_PORCESS_ENV_ARGS 1024
#define CONFIG_NX_TICKS_PER_SECOND 100
#define CONFIG_NX_PLATFORM_I386_PC32 1
#define CONFIG_NX_DRIVER_CONSOLE 1
#define CONFIG_NX_PRINT_BUF_LEN 256
#define CONFIG_NX_DRIVER_NULL 1
#define CONFIG_NX_DRIVER_ZERO 1
#define CONFIG_NX_VFS_MAX_PATH 512
#define CONFIG_NX_VFS_MAX_NAME 256
#define CONFIG_NX_VFS_MAX_FD 256
#define CONFIG_NX_VFS_NODE_HASH_SIZE 256
#endif


//syscall - indirect system call
SYNOPSIS
       #define _GNU_SOURCE         /* See feature_test_macros(7) */
       #include <unistd.h>
       #include <sys/syscall.h>   /* For SYS_xxx definitions */

       int syscall(int number, ...);
       
/* sysconf( _SC_PAGESIZE );  此宏查看缓存内存页面的大小；打印用%ld长整型。
 sysconf( _SC_PHYS_PAGES ) 此宏查看内存的总页数；打印用%ld长整型。
 sysconf( _SC_AVPHYS_PAGES ) 此宏查看可以利用的总页数；打印用%ld长整型。
 sysconf( _SC_NPROCESSORS_CONF ) 查看cpu的个数；打印用%ld长整。
 sysconf( _SC_NPROCESSORS_ONLN ) 查看在使用的cpu个数；打印用%ld长整。
 (long long)sysconf(_SC_PAGESIZE) * (long long)sysconf(_SC_PHYS_PAGES) 计算内存大小。
 sysconf( _SC_LOGIN_NAME_MAX ) 查看最大登录名长度；打印用%ld长整。
 sysconf( _SC_HOST_NAME_MAX ) 查看最大主机长度；打印用%ld长整。
 sysconf( _SC_OPEN_MAX )  每个进程运行时打开的文件数目；打印用%ld长整。
 sysconf(_SC_CLK_TCK) 查看每秒中跑过的运算速率；打印用%ld长整。*/
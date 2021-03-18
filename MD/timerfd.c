网上关于timerfd的文章很多，在这儿归纳总结一下方便以后使用，顺便贴出一个timerfd配合epoll使用的简单例子
一、什么是timerfd
	timerfd是Linux为用户程序提供的一个定时器接口。这个接口基于文件描述符，通过文件描述符的可读事件进行超时通知，因此能够被用于select/poll的应用场景。
	下面对timerfd系列函数先做一个简单的介绍：

这些系统的原理是创建一个定时器，通过文件描述符的超时来进行计时；
#include <sys/timerfd.h>
int timerfd_create(int clockid, int flags);
/* 
timerfd_create函数创建一个定时器对象，同时返回一个与之关联的文件描述符。
clockid：clockid标识指定的时钟计数器，可选值（CLOCK_REALTIME、CLOCK_MONOTONIC。。。）
CLOCK_REALTIME:系统实时时间,随系统实时时间改变而改变,即从UTC1970-1-1 0:0:0开始计时,中间时刻如果系统时间被用户改成其他,则对应的时间相应改变
CLOCK_MONOTONIC:从系统启动这一刻起开始计时,不受系统时间被用户改变的影响
flags：参数flags（TFD_NONBLOCK(非阻塞模式)/TFD_CLOEXEC（表示当程序执行exec函数时本fd将被系统自动关闭,表示不传递）
*/

#include <sys/timerfd.h>

struct timespec {
    time_t tv_sec;                /* Seconds */
    long   tv_nsec;               /* Nanoseconds */
};

struct itimerspec {
    struct timespec it_interval;  /* Interval for periodic timer （定时间隔周期）*/
    struct timespec it_value;     /* Initial expiration (第一次超时时间)*/
};
int timerfd_settime(int fd, int flags, const struct itimerspec *new_value, struct itimerspec *old_value);
/*
	timerfd_settime()此函数用于设置新的超时时间，并开始计时,能够启动和停止定时器;
	fd: 参数fd是timerfd_create函数返回的文件句柄
	flags：参数flags为1代表设置的是绝对时间（TFD_TIMER_ABSTIME 表示绝对定时器）；为0代表相对时间。
	new_value: 参数new_value指定定时器的超时时间以及超时间隔时间
	old_value: 如果old_value不为NULL, old_vlaue返回之前定时器设置的超时时间，具体参考timerfd_gettime()函数
	
	** it_interval不为0则表示是周期性定时器。
	   it_value和it_interval都为0表示停止定时器
*/

int timerfd_gettime(int fd, struct itimerspec *curr_value);
/*
	timerfd_gettime()函数获取距离下次超时剩余的时间
	curr_value.it_value 字段表示距离下次超时的时间，如果改值为0，表示计时器已经解除
	改字段表示的值永远是一个相对值，无论TFD_TIMER_ABSTIME是否被设置
	curr_value.it_interval 定时器间隔时间
*/

uint64_t exp = 0;
read(fd, &exp, sizeof(uint64_t)); 
//可以用read函数读取计时器的超时次数，改值是一个8字节无符号的长整型


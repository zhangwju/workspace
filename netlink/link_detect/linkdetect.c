/******************************************
 * Filename: linkdetect.c
 * Author: zhangwj
 * Date: 2017-07-22
 * Description: uplink detect module of mutil gateway
 * Warnning:
 ******************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <net/if.h>

#include "nano_ipc.h"
#include "common.h"
#include "logger.h"
#include "l2_detect.h"
#include "l3_detect.h"
#include "iface_info.h"
#include "virt_notify.h"

#define SIGNAL_RELOAD					SIGHUP
#define SIGNAL_SIGUSR1					SIGUSR1

#define UPLINK_DEV_MAX_CNT				24
#define DEFAULT_DETECT_TIME				5

#define UPLINK_DETECT_CONIG_PATH		"/etc/config/linkdetect"
#define UPLINK_DEV_CONFIG_PATH			"/etc/config/special_policeman_common.conf"

struct linkmgt_info *plkmgt;

/* pthread id */
pthread_t g_l2_thread_handle;
pthread_t g_l3_thread_handle;
pthread_t g_virt_thread_handle;

/* l3 thread crotrl */
uint g_l3_thread_ctr  = 1;
uint g_virt_thread_ctr = 1;
uint g_detect_addr = 0;

/* Lock */
pthread_rwlock_t g_RWLock;
pthread_mutex_t g_mutex;

/* Nanomsg */
int nn_cli_fd;

/* log switch (default disable) */
int log_enable = 0;

/* uplink dev type string set */
const char *uplink_type[] = {"Fiber", "4G", "Micros", "Satellite", NULL, NULL, NULL, NULL, NULL, NULL, "Lan", "Unknown"};

/* singal */
static char *g_signal;

typedef struct {
    int     signo;
    char   *signame;
    char   *name;
    void  (*handler)(int signo);
} link_signal_t;

void link_signal_handler(int signo);

link_signal_t  signals[] = {
    { SIGNAL_RELOAD,
      "SIGHUP",
      "reload",
      link_signal_handler },
    { SIGNAL_SIGUSR1,
      "SIGUSR1",
      "lswitch",
      link_signal_handler },
    { 0, NULL, "", NULL }
};

static void help()
{
	printf("linkdetect -[uhl]\n"
	"Usage:\n"
	"  -s reload config file\n"
	"  -l enable|disable (enable or disable log)\n"
	"  -h help\n");

	exit(0);
}

static void uplink_detect_log_switch()
{
	if (log_enable) {
		log_enable = 0;
	} else {
		log_enable = 1;
	}
}

/* check ipaddr isvalid */
bool isvalidip(const char *ip)
{
    int dots = 0;
    int setions = 0;  
    
    if (NULL == ip || *ip == '.') {
        return false;
    }   
    
    while (*ip) {
        if (*ip == '.') {
            dots ++; 
            if (setions >= 0 && setions <= 255) {
                setions = 0;
                ip++;
                continue;
            }   
            return false;
        }   
        else if (*ip >= '0' && *ip <= '9') {
            setions = setions * 10 + (*ip - '0');
        } else {
            return false;
        }   
        ip++;   
    }   
    
    if (setions >= 0 && setions <= 255) {
        if (dots == 3) {
            return true;
        }   
    }    
    return false;
}

int get_uplink_dev_config(struct uplinks *dev)
{
	FILE * fp = NULL;
	char line[512];
	char ifname[IFNAMESZ];
	char lte_buf[128]= {0};
	int i = 0;
	char *s, *p;
	int sub_id = 0;  //for lte
	int fiber_ok = 0;
	int lte_ok = 0;
	int satellite_ok = 0;
	int microwave_ok = 0;
//	int lan_ok = 0;

	log_dbg("load uplink_dev config");
	fp = fopen(UPLINK_DEV_CONFIG_PATH, "r");
	if(fp) {
		while (fgets(line, 511, fp)) {
			if (line[0] == '#'	|| 
				line[0] == '\0' || 
				line[0] == '\r') {
				continue;
			}

			memset(ifname, 0, IFNAMESZ);
			if (strstr(line, "UPLINK_FIBER")) {
				if(sscanf(line, "UPLINK_FIBER=%s", ifname) > 0){
					dev->uplink[i].type = UPLINK_FIBER;
					dev->uplink[i].subid = 0;
					strcpy(dev->uplink[i].ifname, ifname);
					fiber_ok = 1;
					i++;
					log_dbg("UPLINK_FIBER: %s", ifname);
				} 
			}else if(strstr(line, "UPLINK_MICROWAVE")) {
				if(sscanf(line, "UPLINK_MICROWAVE=%s", ifname) > 0){
					dev->uplink[i].type = UPLINK_MICROWAVE;
					dev->uplink[i].subid = 0;
					strcpy(dev->uplink[i].ifname, ifname);
					microwave_ok = 1;
					i++;
					log_dbg("UPLINK_MICROWAVE: %s", ifname);
				}
			}else if(strstr(line, "UPLINK_SATELLITE")) {
				if(sscanf(line, "UPLINK_SATELLITE=%s", ifname) > 0){
					dev->uplink[i].type = UPLINK_SATELLITE;
					dev->uplink[i].subid = 0;
					strcpy(dev->uplink[i].ifname, ifname);
					satellite_ok = 1;
					i++;
					log_dbg("UPLINK_SATELLITE: %s", ifname);
				}
			}else if(strstr(line, "UPLINK_4G")) {
				if(sscanf(line, "UPLINK_4G=%s", lte_buf) > 0){
					log_dbg("UPLINK_4G: %s", lte_buf);
					s= lte_buf;
					while(i < UPLINK_DEV_MAX_CNT) {
						p = strstr(s, ",");
						if(p == NULL) {
							dev->uplink[i].type = UPLINK_LTE;
							dev->uplink[i].subid = sub_id;
							strcpy(dev->uplink[i].ifname, s);
							i++;
							break;
						}else {
							dev->uplink[i].type = UPLINK_LTE;
							dev->uplink[i].subid = sub_id;
							memcpy(dev->uplink[i].ifname, s, (p - s));
							*(dev->uplink[i].ifname + (p - s)) = '\0';
							s = p + 1;
							i++;
							sub_id++;
						}
					}
					lte_ok = 1;
				}
			} /*else if(strstr(line, "DOWNLINK_LAN")) {
				if(sscanf(line, "DOWNLINK_LAN=%s", ifname) > 0){
					conf->dev.uplink[i].type = DOWNLINK_LAN;
					conf->dev.uplink[i].subid = 0;
					strcpy(conf->dev.uplink[i].ifname, ifname);
					i++;
					lan_ok = 1;
					log_dbg("DOWNLINK_LAN: %s", ifname);
				}
			}*/
		}
		fclose(fp);	
	}
	
	dev->uplinks = i;
	log_dbg("uplink_dev count: %d", dev->uplinks);
	if (fiber_ok ||  microwave_ok ||
		lte_ok || satellite_ok) {
		log_dbg("load uplink_dev config over");
		return 0;
	}
	
	log_dbg("load uplink_dev config error");
	return -1;
}

int get_uplink_detect_config(struct globalargs *conf)
{
	FILE * fp = NULL;
	char line[512];
	char ip[16] = {0};
	uint addr = 0;
	uint period = 0;
	
	
	fp = fopen(UPLINK_DETECT_CONIG_PATH, "r");
	if(fp) {
		while (fgets(line, 511, fp)) {
			if (line[0] == '#'	|| 
				line[0] == '\0' || 
				line[0] == '\r') {
				continue;
			}
			
			if (strstr(line, "uplink_detect_period")) {
				sscanf(line, "uplink_detect_period=%u", &period); 
			}
			
			if (strstr(line, "uplink_detect_address")) {
				if (sscanf(line, "uplink_detect_address=%s", ip) > 0) {
					if (isvalidip(ip)) {
						addr = ntohl(inet_addr(ip));
					} else {
						log_dbg("uplink_detect_address %s is invalid", ip);
					}
				}
			}
		}
		fclose(fp);	
	}
	if (period == 0) {
		conf->period = DEFAULT_DETECT_TIME;
	} else {
		conf->period = period;
	}
	g_detect_addr = addr;
	
	log_dbg("uplink_detect_period=%u, detect_address: "IPSTR"", conf->period, IP2STR(g_detect_addr));
	return 0;
}

int create_detect_scokfd(struct uplink_info *uplink)
{
    int i;
    int sockfd;
    struct sockaddr_in dstaddr;
    struct ifreq data;
    struct timeval tv; 

    if ((sockfd = socket(AF_INET, SOCK_RAW, IPPROTO_ICMP)) == -1) 
    {   
        log_dbg("Create Socket Fail!\n");
        return -1; 
    }   

    /* Set socket option:  bind device */
    strncpy(data.ifr_name, uplink->ifname, IFNAMESZ);
    if (setsockopt(sockfd, SOL_SOCKET, SO_BINDTODEVICE, (char *)&data, sizeof(data))  < 0) {
           log_dbg("Set Socket option bind to device: %s failed", uplink->ifname);
           close(sockfd);
           return -1; 
    }   

    /*Set socket option: send timeout */
    tv.tv_sec  = 1;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_SNDTIMEO, &tv, sizeof(tv)) < 0)
    {   
        log_dbg("Set Socket option send timeout failed");
        close(sockfd);
        return -1; 
    }   

    /*Set socket option: receive timeout */
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0)
    {   
        log_dbg("Set Socket option receive timeout failed");
        close(sockfd);
        return -1; 
    }
    uplink->sockfd = sockfd;
    return sockfd;
}

int linkdetct_init()
{
	int i;
	int len;
	len = sizeof(struct linkmgt_info) + sizeof(struct uplink_info) * UPLINK_DEV_MAX_CNT;
	plkmgt = (struct linkmgt_info *)malloc(len);
	if (NULL == plkmgt) {
		return -1;
	}

	memset(plkmgt, 0, len);
	while(get_uplink_dev_config(&plkmgt->dev)) {
		sleep(2); //until load config success
	}
	get_uplink_detect_config(&plkmgt->args);

	/* init detect sockfd */
	for(i = 0; i < plkmgt->dev.uplinks; i++) {
		if (create_detect_scokfd(&plkmgt->dev.uplink[i]) < 0) {
			plkmgt->dev.uplink[i].crtflag = 1;
			plkmgt->dev.uplink[i].sockfd = -1;
		}
	}

	/* init uplink netinfo */
	for(i = 0; i < plkmgt->dev.uplinks; i++) {
		get_netinfo_by_ifname(plkmgt->dev.uplink[i].ifname, &(plkmgt->dev.uplink[i].net));
		log_dbg("ifname: %s, ip: "IPSTR", netmask: "IPSTR", gw: "IPSTR"", plkmgt->dev.uplink[i].ifname, IP2STR(plkmgt->dev.uplink[i].net.ip), 
			IP2STR(plkmgt->dev.uplink[i].net.mask),IP2STR(plkmgt->dev.uplink[i].net.gw));
	}
	return 0;
}

int init_uplink_info_list()
{
#if 0
	int i = 0;
	struct uplinks *dev;
	
	dev = &plkmgt->dev;
	for(i = 0; i < dev->uplinks; i++) {
		iface_node_insert(dev->uplink[i].ifname, dev->uplink[i].type, 
			dev->uplink[i].subid, DEV_EVENT_LINK_DOWN);
	}
#endif	
	return 0;
}

/**
 * @brief Option parser
 */
static void parse_options(int argc, char **argv)
{
	int opt;

	if (argc == 1) {
		return;
	}

	while ((opt = getopt(argc, argv, "shl")) != EOF) {
		switch (opt) {
			case 's':
				g_signal = argv[2];

				if (g_signal == NULL) {
					help();
				}
				if (strcmp(g_signal, "reload") != 0) {
					help();
				}
				break;
			case 'l':
				if (argv[2] == NULL) {
					help();
				}

				if((strncmp(argv[2], "enable", 6) == 0) ||
					(strncmp(argv[2], "disable", 7) == 0)) {
					g_signal = "SIGUSR1";
					break;
				}
				help();
				break;
			case 'h':
				help();
				break;
			default:
				help();
				break;
		}
	}
	return;
}

int signal_process_exec(const char *signal, int pid, int signo)
{
	if (pid < 0) {
		fprintf(stderr, "Signal error. pid:[%d]\n", pid);
		return -1;
	}

	if (kill(pid, signo) == -1) {
		fprintf(stderr, "Signal error. [%s]\n", signal);
		return -1;
	}
	return 0;
}

static int signal_process(const char *signal, const char *program_name)
{
	FILE *fp;
	int signo = -1, count = 0;
	char cmd[1024] = {0}, line[200], pid[20];

	pid_t this_pid = getpid();

	if (strcmp(signal, "reload") == 0) {
		signo = SIGNAL_RELOAD;
	} else if (strcmp(signal, "SIGUSR1") == 0) {
		signo = SIGNAL_SIGUSR1; 
	}
	
	if (signo == -1) {
		fprintf(stderr, "Signal not support. [%s]\n", signal);
		return -1;
	}

	if(strrchr(program_name, '/') == NULL)
		sprintf(cmd, "ps | grep '%s' | grep -v 'grep' | awk '{print $1}'", program_name);
	else
		sprintf(cmd, "ps | grep '%s' | grep -v 'grep' | awk '{print $1}'", strrchr(program_name, '/') + 1);

	if((fp = popen(cmd, "r")) != NULL)
	{
		while(fgets(line, 100, fp))
		{
    		if(strlen(line) < 3)
				continue;
			memset(pid, 0, sizeof(pid));
			sscanf(line, "%s", pid);
			if(this_pid == atoi(pid))
				continue;

			signal_process_exec(signal, atoi(pid), signo);

			count++;
		}
		pclose(fp);
	}

	if(count)
	{
		fprintf(stderr, "%s send signal num=%d\n", program_name, count);
	} else {
		fprintf(stderr, "Maybe program [%s] is not running send signal num=%d\n", program_name, count);
	}
	return 0;
	
}

int link_init_signals(void)
{
    link_signal_t      *sig;
    struct sigaction   sa;

    for (sig = signals; sig->signo != 0; sig++) {
        memset(&sa, 0, sizeof(struct sigaction));
        sa.sa_handler = sig->handler;
        sigemptyset(&sa.sa_mask);
        if (sigaction(sig->signo, &sa, NULL) == -1) {
			fprintf(stderr, "Signal error.sigaction(%s) failed", sig->signame);
            return -1;
        }
    }
    return 0;
}

void link_signal_handler(int signo)
{
    link_signal_t    *sig;

    for (sig = signals; sig->signo != 0; sig++) {
        if (sig->signo == signo) {
            break;
        }
    }

	switch (signo) {
	case SIGNAL_RELOAD:
		get_uplink_detect_config(&plkmgt->args);
		break;
	case SIGNAL_SIGUSR1:
		uplink_detect_log_switch();
		break;
	default:
		fprintf(stderr, "Signal not support. signo=[%d]", signo);
		break;
	}
	return;
}

static int thread_is_exists(pthread_t tid)
{
	int kill_rc = 0;
	
	kill_rc = pthread_kill(tid, 0);
	if (kill_rc == ESRCH) {
		log_dbg("the thread [0x%x] did not exists or already quit", (unsigned int )tid);
		return 1;
	} else if(kill_rc == EINVAL) {
		log_dbg("signal is invalid");
		return 1;
	}
	
	log_dbg("the thread [0x%x] is alive", (unsigned int )tid);
	return 0;
}

void free_plkmgt()
{
	int i = 0;
	struct uplinks *dev;
	
	if(!plkmgt) {
		return;
	}
	
	dev = &plkmgt->dev;
	for(i = 0; i < dev->uplinks; i++) {
		if(dev->uplink[i].sockfd){
			close(dev->uplink[i].sockfd);
		}
	}
	free(plkmgt);
}

static void sighandler(int sig)
{
	log_dbg("Recvice signal ID[%d]\n", sig);
	
	__sync_lock_test_and_set(&g_l3_thread_ctr, 0);
	__sync_lock_test_and_set(&g_virt_thread_ctr, 0);
	
	if (!thread_is_exists(g_l2_thread_handle)) {
		pthread_cancel(g_l2_thread_handle);
	}

	nn_socket_close(nn_cli_fd);
	iface_node_release();
	
	free_plkmgt();
	
	pthread_mutex_destroy(&g_mutex);
	pthread_rwlock_destroy(&g_RWLock);

	/* exit process */
	exit(0);
}

	
int globle_init(void)
{
	pthread_rwlockattr_t attr;

	signal(SIGPIPE, SIG_IGN); /* ignore SIGPIPE signal */
	signal(SIGTERM, sighandler);
	signal(SIGINT, sighandler);

	/* init signal */
	link_init_signals();

	/* init log file */
    if (!log_init("logserver", "/tmp/link_detect.log", 10*1024*1000, LOG_LEVEL_DEBUG, 0)) {
        return -1; 
    }

	/* init link config */
	if(linkdetct_init()){
		return -1; 
	}
	
	/* init nanomsg */
	if (nn_socket_ipc_init(&nn_cli_fd, NN_SOCKET_ADDR)) {
		return -1;
	}
	
	/*init pthread mutex */
	pthread_mutex_init(&g_mutex, NULL);

	/* set write priorityi */
    pthread_rwlockattr_init(&attr);
    pthread_rwlockattr_setkind_np(&attr, PTHREAD_RWLOCK_PREFER_WRITER_NONRECURSIVE_NP);
    pthread_rwlock_init(&g_RWLock, &attr);
	
	init_uplink_info_list();
	return 0;
}

int main(int argc, char **argv)
{	
	parse_options(argc, argv);
	if (g_signal) {
		return signal_process(g_signal, "link_detect");
	}
	
	if (globle_init() < 0) {
		sighandler(SIGTERM);
		return -1;
	}

	if (pthread_create(&g_l2_thread_handle, 
						NULL,
						l2_thread_handler,
						NULL)) {
		log_dbg("Layer 2 thread created failed");
		goto EXIT;
	}

	if(pthread_create(&g_l3_thread_handle, 
						NULL, 
						l3_thread_handler, 
						NULL)) {
		log_dbg("Layer 3 thread created failed");
		goto EXIT;
	}

	/* for virt platform */
	if(pthread_create(&g_virt_thread_handle, 
						NULL, 
						virt_notify_handle, 
						NULL)) {
		log_dbg("virt server thread created failed");
		goto EXIT;
	}

	pthread_join(g_l2_thread_handle, NULL);
	pthread_join(g_l3_thread_handle, NULL);
	pthread_join(g_virt_thread_handle, NULL);
	return 0;
	
EXIT:
	sighandler(SIGTERM);
	return -1;
}

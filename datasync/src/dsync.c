/*********************************************
 * Filename: dsync.c
 * Author: zhangwju
 * Date: 2018-02-01
 * Email: zhangwju@gmail.com
 *********************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <sys/epoll.h>
#include <sys/types.h>          /* See NOTES */
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

#include "common.h"
#include "dsync.h"
#include "dsync_msg.h"
#include "ring_queue.h"

int addsock_epoll(int epfd, int sock)
{
	struct epoll_event ev;

	ev.data.fd = sock; 
	ev.events = EPOLLIN | EPOLLET;

	if (epoll_ctl(epfd, EPOLL_CTL_ADD, sock, &ev) < 0) {
		log_dbg("epoll add, Error:[%d:%s]", errno, strerror(errno));
		return -1; 
	}   

    return 0;
}

int delsock_epoll(int epfd, int sock)
{
	return epoll_ctl(epfd, EPOLL_CTL_DEL, sock, NULL);
}

/* epoll call */
int accept_init(int accept_sock, PDSYNC_MINFO pdsync)
{
	int i;
	int ret;

	for (i = 0; i < MAX_ACCEPTS; i++) {
		if (((pdsync->accmask & (1 << i)) == 0)) {
			pdsync->accepts[i].sock = accept_sock;

			if (pdsync->accepts[i].rcv_queue == NULL) {
				ret = rqueue_init(&(pdsync->accepts[i].rcv_queue), RQUEUE_SIZE);
				if (ret < 0) {
					break;
				}
			}
			rqueue_clear(pdsync->accepts[i].rcv_queue);

			ret = addsock_epoll(pdsync->epfd, accept_sock);
			if (ret < 0) {
				break;
			} 
			pdsync->accmask |= (1 << i);
			return 0;
		}
	}

	return -1;	
}

int accept_release(int accid, PDSYNC_MINFO pdsync) 
{
	PSMSG_INFO pscb;

	if ((pdsync->accmask & (1 << accid))) {	
		pdsync->accmask &= ~(1 << accid);

		pscb = pdsync->accepts + accid;
		if (pscb->sock > 0) {
			delsock_epoll(pdsync->epfd, pscb->sock);
			close(pscb->sock);
			pscb->sock = -1;
		}

		if (pscb->rcv_queue) {
			rqueue_release(pscb->rcv_queue);
			pscb->rcv_queue = NULL;	
		}
	}
}

/***************************************************************************/
/* dsync recv task */

int find_acceptid_bysock(int sock, PDSYNC_MINFO pdsync)
{
	int i;

	for (i = 0; i < MAX_ACCEPTS; i++) {
		if (((pdsync->accmask & (1 << i)) &&
			(sock == pdsync->accepts[i].sock))) {
			return i;
		}
	}
	return -1;
}

int chk_dmsghdr(PDSYNC_MSGHDR pmsghdr)
{
	if (memcmp(pmsghdr->sign, MSG_SIGN, MSG_SIGNLEN) != 0) {
		return -MSGERR_SIGN;
	}
	
	if (!MODID_ISVALID(pmsghdr->modid)) {
		return MSGERR_MODID;
	}
	
	if (!ACCTYPE_ISVALID(pmsghdr->acctype)) {

		return MSGERR_ACCTYPE;
	}
	
	return MSGERR_OK;
}

void *delay_dsync_task(void *arg)
{
	int i;
	int ret;
	int size;
	int do_shut;
	int do_process;
	PDSYNC_MINFO pdsync;
	PDSYNC_MSGHDR pmsghdr;
	uchar msgbuf[MAX_MSGLEN];
	PCMSG_INFO pccb;

	pdsync = (PDSYNC_MINFO)arg;
	while (pdsync->delay_thread_ctrl) {
		do_process = 0;
		for (i = 0; i < MAX_CLIENTS; i++) { 
			do_shut = 0;
			if ((pdsync->notify_mask & (1 << i)) && (__sync_lock_test_and_set(&(pdsync->clients[i].use), 1) == 0)) {
				do {	
					//log_dbg("dsync dstaddr: "IPSTR"", IP2STR(pdsync->clients[i].addr));
					pccb = &pdsync->clients[i];
					size = rqueue_poll(pccb->delay_queue, msgbuf, MAX_MSGLEN);	
					if (size > 0) {
						do_process ++;
						ret = send_dmsg(pccb->sock, msgbuf, size);	
						if (ret != size) {
							pccb->errcnt ++;
							do_shut = 1; 
							break;	
						}
						ret = recv_dmsg(pccb->sock, msgbuf, sizeof(DSYNC_MSGHDR));
						if (ret != sizeof(DSYNC_MSGHDR)) {
							pccb->errcnt ++;
							do_shut = 1;
							break;
						}

						pmsghdr = (PDSYNC_MSGHDR)msgbuf;
						ret = chk_dmsghdr(pmsghdr);
						if (ret != MSGERR_OK) {
							break;
						}
						pccb->send_ok++;
					} else break;

				} while(1); //end while(0)

				if (do_shut) {
					close(pccb->sock);
					pccb->sock = -1;
					pdsync->notify_mask &= ~(1 << i);
				}
				__sync_lock_release(&(pdsync->clients[i].use));
			}	
		}//end for(i)	

		/* empty loop */
		if (do_process == 0) {
			usleep(20000);
		}
	} // end while()	

	return NULL;
}

void *exec_dsync_task(void *arg)
{
	int i;
	int ret;
	int size;
	int do_shut;
	int do_process;
	PDSYNC_MINFO pdsync;
	PDSYNC_MSGHDR pmsghdr;
	PSMSG_INFO pscb;
	uchar msgbuf[MAX_MSGLEN];
	PDSYNCPROC_MODULE pdsync_module;
	
	pdsync = (PDSYNC_MINFO)arg;
	while (pdsync->execdsync_thread_ctrl) {
		do_process = 0;
		for (i = 0; i < MAX_ACCEPTS; i++) {
			do_shut = 0;		
			if (((pdsync->accmask & (1 << i)) && 
				(__sync_lock_test_and_set(&(pdsync->accepts[i].use), 1) == 0))) {
				pscb = &(pdsync->accepts[i]);
				do {
					size = rqueue_poll(pscb->rcv_queue, msgbuf, MAX_MSGLEN);
					if (size > 0) {
						do_process ++;
						pmsghdr = (PDSYNC_MSGHDR)msgbuf;
						pdsync_module = pdsync->dsyncprocs + pmsghdr->modid;
						//log_dbg("modid: %d, acctype: %d, msglen: %d", pmsghdr->modid, pmsghdr->acctype, pmsghdr->msglen);
						if (pdsync_module) {
							ret = pdsync_module->dsync_call[pmsghdr->acctype]((uchar *)(pmsghdr + 1), pmsghdr->msglen, pdsync_module->callarg);
							if (ret == 0) {
								continue;
							} 
						} 
					} 
					break;
				} while(1);	//end while(0)
				__sync_lock_release(&(pdsync->accepts[i].use));
			}
		}//end for(i)

		/* is empty loop */
		if (do_process == 0) {
			usleep(20000);
		}
	}
	return NULL;
}


void *dsync_recv_task(void *arg)
{
	log_dbg("dsync recv task [0x%x] running....", pthread_self());
	int i, id;
	int fd_cnt;
	int sock;
	int timeout;
	int size;
	int realsize;
	int do_shut;
	int ret;
	PDSYNC_MINFO pdsync;
	PSMSG_INFO pscb;
	PDSYNC_MSGHDR pmsghdr;
	PDSYNCPROC_MODULE pdsync_module;
	struct epoll_event events[MAX_EVENTS];
	uchar msgbuf[MAX_MSGLEN];
	
	pdsync = (PDSYNC_MINFO)arg;
	timeout = 1000;
	while(pdsync->recvmsg_thread_ctrl) {	
		fd_cnt = epoll_wait(pdsync->epfd, events, MAX_EVENTS, timeout);
		for (i = 0; i < fd_cnt; i++) {
			do_shut = 0;
			ret = -1;
			do {
				sock = events[i].data.fd;
				if (events[i].events & EPOLLIN) {
					id = find_acceptid_bysock(sock, pdsync);
					if ((id >= 0) && (id < MAX_DSYNCMOD)) {
						pscb = &pdsync->accepts[id];
						size = recv_dmsg(pscb->sock, msgbuf, sizeof(DSYNC_MSGHDR));
						if (size != sizeof(DSYNC_MSGHDR)) {
							do_shut = 1;
							break;
						}

						pmsghdr = (PDSYNC_MSGHDR)msgbuf;
						ret = chk_dmsghdr(pmsghdr);
						if(ret != 0) {
							do_shut = 1;
							break;
						}
						
						if (pmsghdr->msglen > 0) {
							size = recv_dmsg(pscb->sock, msgbuf + sizeof(DSYNC_MSGHDR), pmsghdr->msglen);
							if(size != pmsghdr->msglen) {
								do_shut = 1;
								break;
							}

							/* callback dsync module  */
							realsize = sizeof(DSYNC_MSGHDR) + pmsghdr->msglen;
							pdsync_module = pdsync->dsyncprocs + pmsghdr->modid;
							if (pdsync_module->valid && 
								pdsync_module->dsync_call[pmsghdr->acctype] != 0) {
								ret = rqueue_push(pscb->rcv_queue, msgbuf, realsize);
								if (ret == 0) {
									//log_dbg("rpush success");
								} 
							}
						}
					} else {
						delsock_epoll(pdsync->epfd, sock);
						close(sock);
					}
				}
			} while(0); //end for while(0)
			
			if (ret >= 0) {
				FILL_DSYNCHDR(pmsghdr, pmsghdr->modid, pmsghdr->acctype, 0, 0, ret);	
				size = send_dmsg(pscb->sock, msgbuf, sizeof(DSYNC_MSGHDR));
				if (size != sizeof(DSYNC_MSGHDR)) {
					do_shut = 1;
				}
			}

			if (do_shut) {
				delsock_epoll(pdsync->epfd, sock);
				close(pdsync->accepts[id].sock);
				pdsync->accepts[id].sock = -1;
				pdsync->accmask &= ~(1 << id);
			}
		} //end for(i)
	}

	return NULL;
}

void *dsync_server_task(void *arg)
{
	log_dbg("dsync server task [0x%x] running....", pthread_self());
	PDSYNC_MINFO pdsync;
	int accept_sock;
	int ret;
	int errcnt = 0;
	
	pdsync = (PDSYNC_MINFO)arg;
	
	while (pdsync->server_thread_ctrl) {
		if (pdsync->servsock < 0) {
			pdsync->servsock = init_dsync_server(0, pdsync->sport, pdsync->listen);
			if (pdsync->servsock < 0) {
				usleep(200);
				continue;
			}
		}
		
		accept_sock = accept_dmsg(pdsync->servsock);
		if (accept_sock >= 0) {
			ret = accept_init(accept_sock, pdsync);
			if (ret < 0) {
				close(accept_sock);
			}	
		} else {
			if(!((errno == EINTR)
				|| (errno == ETIMEDOUT)
				|| (errno == EAGAIN))) {
				errcnt++;
			}
		}
		if (errcnt >= 10) {
			errcnt = 0;
			close(pdsync->servsock);
			pdsync->servsock = -1;	
		}
	}

	return NULL;
}

int start_dsyncproc(PDSYNC_MINFO pdsync)
{	
	int ret = 0;
	int i = 0;

	pdsync->server_thread_ctrl = 1;
    pdsync->recvmsg_thread_ctrl = 1;
    pdsync->execdsync_thread_ctrl = 1;
    pdsync->delay_thread_ctrl = 1;

	/* dsync server task */ 
	ret = pthread_create(&(pdsync->pid_server_dsync), NULL, (void *)dsync_server_task, (void *)pdsync);
	if(ret < 0) {
		log_dbg("dsync server task, error:[%d:%s]", errno, strerror(errno));
		return -1;
	}
	pthread_detach(pdsync->pid_server_dsync);

	/* dsync recv task */
	ret = pthread_create(&(pdsync->pid_recv_dsync), NULL, (void *)dsync_recv_task, (void *)pdsync);
	if(ret < 0) {
		log_dbg("dsync recv task, error:[%d:%s]", errno, strerror(errno));
		return -1;
	}
	pthread_detach(pdsync->pid_recv_dsync);

	/* exec dsync threads */	
	for (i = 0; i < pdsync->exec_threads; i++) {
		ret = pthread_create(&(pdsync->pid_exec_dsync[i]), NULL, (void *)exec_dsync_task, (void *)pdsync);
		if (ret < 0) {
			return -1;
		}
		pthread_detach(pdsync->pid_exec_dsync[i]);
	}
	log_dbg("dsync exec %d threads is running....", pdsync->exec_threads);
	
	/* delay proc threads */
	for (i = 0; i < pdsync->delay_threads; i++) {
		ret = pthread_create(&(pdsync->pid_delay_dsync[i]), NULL, (void *)delay_dsync_task, (void *)pdsync);
		if (ret < 0) {
			return -1;
		}
		pthread_detach(pdsync->pid_delay_dsync[i]);
	}
	log_dbg("dsync dealy %d threads is running....", pdsync->delay_threads);

	return 0;
}

int client_release(int client_id, PDSYNC_MINFO pdsync)
{
	
	if (pdsync->notify_mask & (1 << client_id)) {
		pdsync->notify_mask &= ~(1 << client_id);	
		if (pdsync->clients[client_id].sock > 0) {
			close(pdsync->clients[client_id].sock);
			pdsync->clients[client_id].sock = -1;
		}
		
		if (pdsync->clients[client_id].delay_queue) {
			rqueue_release(pdsync->clients[client_id].delay_queue);
			pdsync->clients[client_id].delay_queue = NULL;
		}
	}
}

int stop_dsyncproc(PDSYNC_MINFO pdsync)
{
	int i = 0;
	
	if (pdsync) {
		pdsync->server_thread_ctrl = 0;
		pdsync->recvmsg_thread_ctrl = 0;
		pdsync->execdsync_thread_ctrl = 0;
		pdsync->delay_thread_ctrl = 0;	
		
		for (i = 0; i < MAX_ACCEPTS; i++) {
			accept_release(i, pdsync);
		}

		for (i = 0; i < MAX_CLIENTS; i++) {
			client_release(i, pdsync);
		}
	}
	
	return 0;
}

int get_clientid(uint dstaddr, PDSYNC_MINFO pdsync)
{
	int i;
	int ret = -1;
	
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (((pdsync->notify_mask & (1 << i)) 
			&& (pdsync->clients[i].addr == dstaddr))) {
			return i; /* already connected */
		}
	}

	/* new client, notify dsync module refresh */
	for (i = 0; i < MAX_CLIENTS; i++) {
		if (((pdsync->notify_mask & (1 << i)) == 0)) {
			pdsync->clients[i].sock = init_dsync_client(dstaddr, pdsync->sport);
			if (pdsync->clients[i].sock < 0) {
				ret = -1;
				break;
			} 
		
			if (pdsync->clients[i].delay_queue == NULL) {
				ret = rqueue_init(&(pdsync->clients[i].delay_queue), RQUEUE_SIZE);	
				if (ret < 0 ) {
					break;	
				}
			}
			rqueue_clear(pdsync->clients[i].delay_queue);
			pdsync->clients[i].addr = dstaddr;
			pdsync->notify_mask |= (1 << i);
			return (i | CLIENT_NEWUP); 
		}
	}
	return ret;
}

int _do_notify(uint dstaddr, int modid, int acctype, void *data, int dlen, int active, PDSYNC_MINFO pdsync)
{
	int ret;
	int clientid;
	int payload;
	PDSYNC_MSGHDR pmsghdr;
	PCMSG_INFO pccb;
	uchar msgbuf[MAX_MSGLEN];
	
	clientid = get_clientid(dstaddr, pdsync);
	if (clientid < 0) {	
		return -1;	
	}

	pmsghdr = (PDSYNC_MSGHDR)msgbuf;
	FILL_DSYNCHDR(pmsghdr, modid, acctype, dlen, 0, 0);
	if (chk_dmsghdr(pmsghdr) != 0) {
		return -1;
	}

	/* a client is first connect 
	 * is sync all ??  */	
	if (clientid & CLIENT_NEWUP) {	
		if (pdsync->dsync_newup) {
			pdsync->dsync_newup(dstaddr, 0);
		}
		return 1;
	}
	
	pccb = pdsync->clients + clientid; 
	if (active) {
		if (((dlen > 0) && (dlen <= MAX_PAYLOAD))) {
			memcpy(msgbuf + sizeof(DSYNC_MSGHDR), data, dlen);
			dlen += sizeof(DSYNC_MSGHDR);
			ret = send_dmsg(pccb->sock, msgbuf, dlen);
			if(ret != dlen) {
				return -1;
			}
			ret = recv_dmsg(pccb->sock, msgbuf, sizeof(DSYNC_MSGHDR));
			if (ret != sizeof(DSYNC_MSGHDR)) {
				return -1;
			}

			pmsghdr = (PDSYNC_MSGHDR)msgbuf;
			ret = chk_dmsghdr(pmsghdr);
			if (ret != 0) {
				return -1;
			}
		} 
	} else {
		memcpy(msgbuf + sizeof(DSYNC_MSGHDR), data, dlen);
		dlen += sizeof(DSYNC_MSGHDR);
		ret = rqueue_push(pccb->delay_queue, msgbuf, dlen);
		if (ret < 0) {
			log_dbg("rpush error");
			return -1;
		}
		log_dbg("rpush success, data_len: %d, msghdr: %d", dlen, sizeof(DSYNC_MSGHDR));
	}

	return 0;
}

int do_notify(uint dstaddr, int modid, int acctype, void *data, int dlen, int flag, PDSYNC_MINFO pdsync) 
{
	return _do_notify(dstaddr, modid, acctype, data, dlen, 1, pdsync);
}

int register_syncmod(PDSYNC_MINFO pdsync, PDSYNCPROC_MODULE pmodule)
{
	int ret = -1;
	
	do {
		if (!(pdsync && pmodule)) {
			break;
		}
		if (!((pmodule->modid >= 0) && 
			MODID_ISVALID(pmodule->modid))) {
			break;
		}
		if (pdsync->dsyncprocs[pmodule->modid].valid) {
			log_dbg("Register Module Failure,  ID:[%d] already exists", pmodule->modid);
			break;
		}
		memcpy(&pdsync->dsyncprocs[pmodule->modid], pmodule, sizeof(DSYNCPROC_MODULE));
		pdsync->dsyncprocs[pmodule->modid].valid = 1;
		ret = 0;
		log_dbg("Register Module Success mod_name: %s, mod_id: %d", pmodule->name, pmodule->modid);
	
	}while(0);

	return ret;
}

int unregister_syncmod(int modid, const char *mod_name, PDSYNC_MINFO pdsync)
{

	if (!((pdsync) && MODID_ISVALID(modid))) {
		log_dbg("Module ID:[%d] is invalid", modid);
		return -1;
	}
	
	if (pdsync->dsyncprocs[modid].valid && 
		(strcmp(pdsync->dsyncprocs[modid].name, mod_name) == 0)) {
		memset(&pdsync->dsyncprocs[modid], 0, sizeof(DSYNCPROC_MODULE));
		return 0;
	}

	return -1;
}

int dsync_init(PDSYNC_MINFO pdsync, ushort sport, const char *ifname)
{
	int i;
	int ret = -1;
	
	if (pdsync == NULL || !port_isvalid(sport)) {
		return -1;
	}

	signal(SIGPIPE, SIG_IGN); /* ignore SIGPIPE signal */
	memset(pdsync, 0, sizeof(DSYNC_MINFO));
	pdsync->exec_threads = 1;
	pdsync->delay_threads = 1;
	pdsync->sport = sport;
	if (ifname) {
		strncpy(pdsync->listen, ifname, IFNAMESZ);
	}

	pdsync->servsock = -1;
	pdsync->epfd = epoll_create(MAX_EVENTS);
	if (pdsync->epfd < 0) {
		log_dbg("create epoll, error:[%d:%s]", errno, strerror(errno));
		return -1;
	}

	ret = start_dsyncproc(pdsync);
	if (ret < 0) {
		stop_dsyncproc(pdsync);
		return -1;
	}
	return 0;
}

int dsync_release(PDSYNC_MINFO pdsync)
{
	/* release oprations ... */
	if (pdsync) {
		stop_dsyncproc(pdsync);
		if (pdsync->servsock > 0) {
			close(pdsync->servsock);
			pdsync->servsock = -1;
		}
		if (pdsync->epfd > 0) {
			close(pdsync->epfd);
			pdsync->epfd = -1;
		}
	}

	return 0;
}


/*	$KAME: pim6.c,v 1.11 2003/09/02 09:57:04 itojun Exp $	*/

/*
 * Copyright (C) 1998 WIDE Project.
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
/*
 * Copyright (c) 1998-2001
 * The University of Southern California/Information Sciences Institute.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the project nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE PROJECT AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE PROJECT OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include "defs.h"
#include <sys/uio.h>

/*
 * Exported variables.
 */
char	*pim6dd_pim6_recv_buf;		/* input packet buffer   */
char	*pim6dd_pim6_send_buf;		/* output packet buffer  */

struct sockaddr_in6 pim6dd_allpim6routers_group; /* ALL_PIM_ROUTERS group       */
int	pim6dd_pim6_socket;		/* socket for PIM control msgs */

/*
 * Local variables. 
 */
static struct sockaddr_in6 pim6dd_from;
static struct msghdr pim6dd_sndmh;
static struct iovec pim6dd_sndiov[2];
static struct in6_pktinfo *pim6dd_sndpktinfo;
static u_char *pim6dd_pim6_sndcmsgbuf = NULL;

/*
 * Local function definitions.
 */
static void pim6dd_pim6_read   __P((int f, fd_set *rfd));
static void pim6dd_accept_pim6 __P((int recvlen));
static int pim6dd_pim6_cksum __P((u_int16_t *, struct in6_addr *,
			   struct in6_addr *, int)); 

int
pim6dd_init_pim6()
{
	static int sndcmsglen;
	struct cmsghdr *cmsgp = (struct cmsghdr *)pim6dd_pim6_sndcmsgbuf;

	sndcmsglen = CMSG_SPACE(sizeof(struct in6_pktinfo));
	if ((pim6dd_pim6_socket = socket(AF_INET6, SOCK_RAW, IPPROTO_PIM)) < 0) 
		pim6dd_log_msg(LOG_ERR, errno, "PIM6 socket");

	pim6dd_k_set_rcvbuf(pim6dd_pim6_socket, SO_RECV_BUF_SIZE_MAX,
		     SO_RECV_BUF_SIZE_MIN); /* lots of input buffering */
	pim6dd_k_set_hlim(pim6dd_pim6_socket, MINHLIM);  /* restrict multicasts to one hop */
	pim6dd_k_set_loop(pim6dd_pim6_socket, FALSE);	  /* disable multicast loopback     */

	pim6dd_allpim6routers_group.sin6_len = sizeof(pim6dd_allpim6routers_group);
	pim6dd_allpim6routers_group.sin6_family = AF_INET6;
	if (inet_pton(AF_INET6, "ff02::d",
		      (void *)&pim6dd_allpim6routers_group.sin6_addr) != 1)
		pim6dd_log_msg(LOG_ERR, 0, "inet_pton failed for ff02::d");

	if ((pim6dd_pim6_recv_buf = malloc(RECV_BUF_SIZE)) == NULL ) {
		pim6dd_log_msg(LOG_ERR, 0, "pim6dd_init_pim6: pim6dd_pim6_recv_buf malloc failed\n");
		return -1;
	}
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, 0);
#endif

	if ((pim6dd_pim6_send_buf = malloc(RECV_BUF_SIZE)) == NULL) {
		free(pim6dd_pim6_recv_buf);
		pim6dd_log_msg(LOG_ERR, 0, "pim6dd_init_pim6: pim6dd_pim6_send_buf malloc failed\n");

#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
		return -1;
	}
#ifdef ECOS_DBG_STAT
       dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, 0);
#endif

	/* initialize msghdr for sending packets */
	pim6dd_sndmh.msg_namelen = sizeof(struct sockaddr_in6);
	pim6dd_sndmh.msg_iov = pim6dd_sndiov;
	pim6dd_sndmh.msg_iovlen = 1;
	if (pim6dd_pim6_sndcmsgbuf == NULL && (pim6dd_pim6_sndcmsgbuf = malloc(sndcmsglen)) == NULL) {
		pim6dd_log_msg(LOG_ERR, 0, "malloc failed");
		return -1;
		}
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_ADD, 0);
#endif
	pim6dd_sndmh.msg_control = (caddr_t)pim6dd_pim6_sndcmsgbuf;
	pim6dd_sndmh.msg_controllen = sndcmsglen;
	/* initilization cmsg for specifing outgoing interfaces and source */
	cmsgp=(struct cmsghdr *)pim6dd_pim6_sndcmsgbuf;
	cmsgp->cmsg_len = CMSG_LEN(sizeof(struct in6_pktinfo));
	cmsgp->cmsg_level = IPPROTO_IPV6;
	cmsgp->cmsg_type = IPV6_PKTINFO;
	pim6dd_sndpktinfo = (struct in6_pktinfo *)CMSG_DATA(cmsgp);

	if (pim6dd_register_input_handler(pim6dd_pim6_socket, pim6dd_pim6_read) < 0)
		pim6dd_log_msg(LOG_ERR, 0,
		    "cannot register pim6dd_pim6_read() as an input handler");

	IF_ZERO(&pim6dd_nbr_mifs);

	return 0;
}

/* Read a PIM message */
static void
pim6dd_pim6_read(f, rfd)
	int f;
	fd_set *rfd;
{
	register int pim6_recvlen;
	int fromlen = sizeof(pim6dd_from);
#ifdef SYSV
	sigset_t block, oblock;
#elif defined (__ECOS)
	sigset_t block, oblock;
#else
	register int omask;
#endif

	pim6_recvlen = recvfrom(pim6dd_pim6_socket, pim6dd_pim6_recv_buf, RECV_BUF_SIZE,
			      0, (struct sockaddr *)&pim6dd_from, &fromlen);

	if (pim6_recvlen < 0) {
		if (errno != EINTR)
			pim6dd_log_msg(LOG_ERR, errno, "PIM6 recvmsg");
		return;
	}

#ifdef SYSV
	(void)sigemptyset(&block);
	(void)sigaddset(&block, SIGALRM);
	if (sigprocmask(SIG_BLOCK, &block, &oblock) < 0)
		pim6dd_log_msg(LOG_ERR, errno, "sigprocmask");
#elif defined (__ECOS)
	(void)sigemptyset(&block);
	(void)sigaddset(&block, SIGALRM);
	if (sigprocmask(SIG_BLOCK, &block, &oblock) < 0)
		pim6dd_log_msg(LOG_ERR, errno, "sigprocmask");
#else
	/* Use of omask taken from main() */
	omask = sigblock(sigmask(SIGALRM));
#endif /* SYSV */
    
	pim6dd_accept_pim6(pim6_recvlen);
    
#ifdef SYSV
	(void)sigprocmask(SIG_SETMASK, &oblock, (sigset_t *)NULL);
#elif defined (__ECOS)
	(void)sigprocmask(SIG_SETMASK, &oblock, (sigset_t *)NULL);
#else
	(void)sigsetmask(omask);
#endif /* SYSV */
}


static void
pim6dd_accept_pim6(pimlen)
	int pimlen;
{
	register struct pim *pim;
	struct sockaddr_in6 *src = &pim6dd_from;

	/* sanity check */
	if (pimlen < sizeof(pim)) {
		pim6dd_log_msg(LOG_WARNING, 0,
		    "data field too short (%u bytes) for PIM header, from %s", 
		    pimlen, pim6dd_inet6_fmt(&src->sin6_addr));
		return;
	}
	pim = (struct pim *)pim6dd_pim6_recv_buf;

#ifdef NOSUCHDEF   /* TODO: delete. Too noisy */
	IF_DEBUG(DEBUG_PIM_DETAIL) {
		IF_DEBUG(DEBUG_PIM) {
			pim6dd_log_msg(LOG_DEBUG, 0, "Receiving %s from %s",
			    pim6dd_packet_kind(IPPROTO_PIM, pim->pim_type, 0), 
			    pim6dd_inet6_fmt(&src->sin6_addr));
			pim6dd_log_msg(LOG_DEBUG, 0, "PIM type is %u", pim->pim_type);
		}
	}
#endif /* NOSUCHDEF */


	/* Check of PIM version is already done in the kernel */
	/*
	 * TODO: check the dest. is ALL_PIM_ROUTERS (if multicast address)
	 *	 is it necessary?
	 */
	/* Checksum verification is done in the kernel. */

	switch (pim->pim_type) {
	 case PIM_HELLO:
		 pim6dd_receive_pim6_hello(src, (char *)(pim), pimlen); 
		 break;
	 case PIM_REGISTER:
		 pim6dd_log_msg(LOG_INFO, 0, "ignore %s from %s",
		     pim6dd_packet_kind(IPPROTO_PIM, pim->pim_type, 0),
		     pim6dd_inet6_fmt(&src->sin6_addr));
		 break;
	 case PIM_REGISTER_STOP:
		 pim6dd_log_msg(LOG_INFO, 0, "ignore %s from %s",
		     pim6dd_packet_kind(IPPROTO_PIM, pim->pim_type, 0),
		     pim6dd_inet6_fmt(&src->sin6_addr));
		 break;
	 case PIM_JOIN_PRUNE:
		 pim6dd_receive_pim6_join_prune(src, (char *)(pim), pimlen); 
		 break;
	 case PIM_BOOTSTRAP:
		 pim6dd_log_msg(LOG_INFO, 0, "ignore %s from %s",
		     pim6dd_packet_kind(IPPROTO_PIM, pim->pim_type, 0),
		     pim6dd_inet6_fmt(&src->sin6_addr));
		 break;
	 case PIM_ASSERT:
		 pim6dd_receive_pim6_assert(src, (char *)(pim), pimlen); 
		 break;
	 case PIM_GRAFT:
	 case PIM_GRAFT_ACK:
		 pim6dd_receive_pim6_graft(src, (char *)(pim), pimlen, pim->pim_type);
		 break;
	 case PIM_CAND_RP_ADV:
		 pim6dd_log_msg(LOG_INFO, 0, "ignore %s from %s",
		     pim6dd_packet_kind(IPPROTO_PIM, pim->pim_type, 0),
		     pim6dd_inet6_fmt(&src->sin6_addr));
		 break;
	 default:
		 pim6dd_log_msg(LOG_INFO, 0,
		     "ignore unknown PIM message code %u from %s",
		     pim->pim_type,
		     pim6dd_inet6_fmt(&src->sin6_addr));
		 break;
	}
}


/*
 * Send a multicast PIM packet from src to dst, PIM message type = "type"
 * and data length (after the PIM header) = "datalen"
 */
void 
pim6dd_send_pim6(buf, src, dst, type, datalen)
	char *buf;
	struct sockaddr_in6 *src, *dst;
	int type, datalen;
{
	struct pim *pim;
	int setloop = 0;
	int ifindex = 0, sendlen = sizeof(struct pim) + datalen;

	/* Prepare the PIM packet */
	pim = (struct pim *)buf;
	pim->pim_type = type;
	pim->pim_ver = PIM_PROTOCOL_VERSION;
	pim->pim_rsv = 0;
	pim->pim_cksum = 0;
	/*
	 * TODO: XXX: if start using this code for PIM_REGISTERS, exclude the
	 * encapsulated packet from the checksum.
	 */
	pim->pim_cksum = pim6dd_pim6_cksum((u_int16 *)pim,
				    &src->sin6_addr, &dst->sin6_addr,
				    sendlen);

	/*
	 * Specify the source address of the packet. Also, specify the
	 * outgoing interface and the source address if possible.
	 */
	memcpy(&pim6dd_sndpktinfo->ipi6_addr, &src->sin6_addr,
	       sizeof(src->sin6_addr));
	if ((ifindex = src->sin6_scope_id) != 0) {
		pim6dd_sndpktinfo->ipi6_ifindex = ifindex;
	}
	else {
		pim6dd_sndpktinfo->ipi6_ifindex = 0; /* make sure to be cleared */
		pim6dd_log_msg(LOG_WARNING, 0,
		    "pim6dd_send_pim6: could not determine the outgoint IF; send anyway");
	}

	if (IN6_IS_ADDR_MULTICAST(&dst->sin6_addr)) {
		pim6dd_k_set_if(pim6dd_pim6_socket, ifindex);
		if (IN6_ARE_ADDR_EQUAL(&dst->sin6_addr,
				       &pim6dd_allnodes_group.sin6_addr) ||
		    IN6_ARE_ADDR_EQUAL(&dst->sin6_addr,
				       &pim6dd_allrouters_group.sin6_addr) ||
		    IN6_ARE_ADDR_EQUAL(&dst->sin6_addr,
				       &pim6dd_allpim6routers_group.sin6_addr)) {
			setloop = 1;
			pim6dd_k_set_loop(pim6dd_pim6_socket, TRUE);  
		}
	}

	pim6dd_sndmh.msg_name = (caddr_t)dst;
	pim6dd_sndiov[0].iov_base = (caddr_t)buf;
	pim6dd_sndiov[0].iov_len = sendlen;
	if (sendmsg(pim6dd_pim6_socket, &pim6dd_sndmh, 0) < 0) {
		if (errno == ENETDOWN)
			pim6dd_check_vif_state();
		else
			pim6dd_log_msg(LOG_WARNING, errno, "sendto from %s to %s",
			    pim6dd_inet6_fmt(&src->sin6_addr),
			    pim6dd_inet6_fmt(&dst->sin6_addr));
		if (setloop)
			pim6dd_k_set_loop(pim6dd_pim6_socket, FALSE); 
		return;
	}

	if (setloop)
		pim6dd_k_set_loop(pim6dd_pim6_socket, FALSE); 

	IF_DEBUG(DEBUG_PIM_DETAIL) {
		IF_DEBUG(DEBUG_PIM) {
			char ifname[IFNAMSIZ];

			pim6dd_log_msg(LOG_DEBUG, 0, "SENT %s from %-15s to %s on %s",
			    pim6dd_packet_kind(IPPROTO_PIM, type, 0),
			    pim6dd_inet6_fmt(&src->sin6_addr),
			    pim6dd_inet6_fmt(&dst->sin6_addr),
			    ifindex ? if_indextoname(ifindex, ifname) : "?");
		}
	}
}

//u_int pim6dd_pim_send_cnt = 0;
#define SEND_DEBUG_NUMBER 50

/* ============================== */

/*
 * Checksum routine for Internet Protocol family headers (Portable Version).
 *
 * This routine is very heavily used in the network
 * code and should be modified for each CPU to be as fast as possible.
 */

#define ADDCARRY(x)  (x > 65535 ? x -= 65535 : x)
#define REDUCE {l_util.l = sum; sum = l_util.s[0] + l_util.s[1]; ADDCARRY(sum);}

static union {
	u_int16_t phs[4];
	struct {
		u_int32_t ph_len;
		u_int8_t ph_zero[3];
		u_int8_t ph_nxt;
	} ph;
} pim6dd_uph;

/*
 * Our algorithm is simple, using a 32 bit accumulator (sum), we add
 * sequential 16 bit words to it, and at the end, fold back all the
 * carry bits from the top 16 bits into the lower 16 bits.
 */
static int
pim6dd_pim6_cksum(addr, src, dst, len)
	u_int16_t *addr;
	struct in6_addr *src, *dst;
	int len;
{
	register int nleft = len;
	register u_int16_t *w;
	register int32_t sum = 0;
	u_int16_t answer = 0;

	/*
	 * First create IP6 pseudo header and calculate a summary.
	 */
	w = (u_int16_t *)src;
	pim6dd_uph.ph.ph_len = htonl(len);
	pim6dd_uph.ph.ph_nxt = IPPROTO_PIM;

	/* IPv6 source address */
	sum += w[0];
	/* XXX: necessary? */
	if (!(IN6_IS_ADDR_LINKLOCAL(src) || IN6_IS_ADDR_MC_LINKLOCAL(src)))
		sum += w[1];
	sum += w[2]; sum += w[3]; sum += w[4]; sum += w[5];
	sum += w[6]; sum += w[7];
	/* IPv6 destination address */
	w = (u_int16_t *)dst;
	sum += w[0];
	/* XXX: necessary? */
	if (!(IN6_IS_ADDR_LINKLOCAL(dst) || IN6_IS_ADDR_MC_LINKLOCAL(dst)))
		sum += w[1];
	sum += w[2]; sum += w[3]; sum += w[4]; sum += w[5];
	sum += w[6]; sum += w[7];
	/* Payload length and upper layer identifier */
	sum += pim6dd_uph.phs[0];  sum += pim6dd_uph.phs[1];
	sum += pim6dd_uph.phs[2];  sum += pim6dd_uph.phs[3];

	/*
	 * Secondly calculate a summary of the first mbuf excluding offset.
	 */
	w = addr;
	while (nleft > 1)  {
		sum += *w++;
		nleft -= 2;
	}

	/* mop up an odd byte, if necessary */
	if (nleft == 1) {
		*(u_char *)(&answer) = *(u_char *)w ;
		sum += answer;
	}

	/* add back carry outs from top 16 bits to low 16 bits */
	sum = (sum >> 16) + (sum & 0xffff);	/* add hi 16 to low 16 */
	sum += (sum >> 16);			/* add carry */
	answer = ~sum;				/* truncate to 16 bits */
	return(answer);
}

int pim6dd_clean_pim6()
{
	if (pim6dd_pim6_recv_buf) {
		free(pim6dd_pim6_recv_buf);
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
		pim6dd_pim6_recv_buf = NULL;
		}

	if (pim6dd_pim6_send_buf) {
		free(pim6dd_pim6_send_buf);
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
		pim6dd_pim6_send_buf = NULL;
		}

	if (pim6dd_pim6_sndcmsgbuf) {
		free(pim6dd_pim6_sndcmsgbuf);
#ifdef ECOS_DBG_STAT
        dbg_stat_add(dbg_mldproxy_index, DBG_TYPE_MALLOC, DBG_ACTION_DEL, 0);
#endif
		pim6dd_pim6_sndcmsgbuf = NULL;
		}
}

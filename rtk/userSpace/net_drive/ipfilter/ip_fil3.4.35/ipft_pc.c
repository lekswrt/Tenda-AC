/*
 * Copyright (C) 1993-2001 by Darren Reed.
 *
 * See the IPFILTER.LICENCE file for details on licencing.
 */
#if defined(__sgi) && (IRIX > 602)
# include <sys/ptimers.h>
#endif
#include <stdio.h>
#include <string.h>
#if !defined(__SVR4) && !defined(__GNUC__)
#include <strings.h>
#endif
#include <sys/types.h>
#include <sys/time.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <sys/param.h>
#include <netinet/in.h>
#include <netinet/in_systm.h>
#ifndef	linux
#include <netinet/ip_var.h>
#endif
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <net/if.h>
#include "ip_compat.h"
#include <netinet/tcpip.h>
#include "ipf.h"
#include "pcap.h"
#include "bpf.h"
#include "ipt.h"

#if !defined(lint)
static const char rcsid[] = "@(#)$Id: ipft_pc.c,v 1.1.1.1 2010-04-30 02:12:04 Exp $";
#endif

struct	llc	{
	int	lc_type;
	int	lc_sz;	/* LLC header length */
	int	lc_to;	/* LLC Type offset */
	int	lc_tl;	/* LLC Type length */
};

/*
 * While many of these maybe the same, some do have different header formats
 * which make this useful.
 */

static	struct	llc	llcs[] = {
	{ DLT_NULL, 0, 0, 0 },
	{ DLT_EN10MB, 14, 12, 2 },
	{ DLT_EN3MB, 0, 0, 0 },
	{ DLT_AX25, 0, 0, 0 },
	{ DLT_PRONET, 0, 0, 0 },
	{ DLT_CHAOS, 0, 0, 0 },
	{ DLT_IEEE802, 0, 0, 0 },
	{ DLT_ARCNET, 0, 0, 0 },
	{ DLT_SLIP, 0, 0, 0 },
	{ DLT_PPP, 0, 0, 0 },
	{ DLT_FDDI, 0, 0, 0 },
#ifdef DLT_ATMRFC1483
	{ DLT_ATMRFC1483, 0, 0, 0 },
#endif
	{ DLT_RAW, 0, 0, 0 },
#ifdef	DLT_ENC
	{ DLT_ENC, 0, 0, 0 },
#endif
#ifdef	DLT_SLIP_BSDOS
	{ DLT_SLIP_BSDOS, 0, 0, 0 },
#endif
#ifdef	DLT_PPP_BSDOS
	{ DLT_PPP_BSDOS, 0, 0, 0 },
#endif
#ifdef	DLT_HIPPI
	{ DLT_HIPPI, 0, 0, 0 },
#endif
#ifdef	DLT_HDLC
	{ DLT_HDLC, 0, 0, 0 },
#endif
#ifdef	DLT_PPP_SERIAL
	{ DLT_PPP_SERIAL, 4, 4, 0 },
#endif
#ifdef	DLT_PPP_ETHER
	{ DLT_PPP_ETHER, 8, 8, 0 },
#endif
#ifdef	DLT_ECONET
	{ DLT_ECONET, 0, 0, 0 },
#endif
	{ -1, -1, -1, -1 }
};

static	int	pcap_open __P((char *));
static	int	pcap_close __P((void));
static	int	pcap_readip __P((char *, int, char **, int *));
static	void	swap_hdr __P((pcaphdr_t *));
static	int	pcap_read_rec __P((struct pcap_pkthdr *));

static	int	pfd = -1, s_type = -1, swapped = 0;
static	struct llc	*llcp = NULL;

struct	ipread	pcap = { pcap_open, pcap_close, pcap_readip };

#define	SWAPLONG(y)	\
	((((y)&0xff)<<24) | (((y)&0xff00)<<8) | (((y)&0xff0000)>>8) | (((y)>>24)&0xff))
#define	SWAPSHORT(y)	\
	( (((y)&0xff)<<8) | (((y)&0xff00)>>8) )

static	void	swap_hdr(p)
pcaphdr_t	*p;
{
	p->pc_v_maj = SWAPSHORT(p->pc_v_maj);
	p->pc_v_min = SWAPSHORT(p->pc_v_min);
	p->pc_zone = SWAPLONG(p->pc_zone);
	p->pc_sigfigs = SWAPLONG(p->pc_sigfigs);
	p->pc_slen = SWAPLONG(p->pc_slen);
	p->pc_type = SWAPLONG(p->pc_type);
}

static	int	pcap_open(fname)
char	*fname;
{
	pcaphdr_t ph;
	int fd, i;

	if (pfd != -1)
		return pfd;

	if (!strcmp(fname, "-"))
		fd = 0;
	else if ((fd = open(fname, O_RDONLY)) == -1)
		return -1;

	if (read(fd, (char *)&ph, sizeof(ph)) != sizeof(ph))
		return -2;

	if (ph.pc_id != TCPDUMP_MAGIC) {
		if (SWAPLONG(ph.pc_id) != TCPDUMP_MAGIC) {
			(void) close(fd);
			return -2;
		}
		swapped = 1;
		swap_hdr(&ph);
	}

	if (ph.pc_v_maj != PCAP_VERSION_MAJ) {
		(void) close(fd);
		return -2;
	}

	for (i = 0; llcs[i].lc_type != -1; i++)
		if (llcs[i].lc_type == ph.pc_type) {
			llcp = llcs + i;
			break;
		}

	if (llcp == NULL) {
		(void) close(fd);
		return -2;
	}

	pfd = fd;
	s_type = ph.pc_type;
	printf("opened pcap file %s:\n", fname);
	printf("\tid: %08x version: %d.%d type: %d snap %d\n",
		ph.pc_id, ph.pc_v_maj, ph.pc_v_min, ph.pc_type, ph.pc_slen);

	return fd;
}


static	int	pcap_close()
{
	return close(pfd);
}


/*
 * read in the header (and validate) which should be the first record
 * in a pcap file.
 */
static	int	pcap_read_rec(rec)
struct	pcap_pkthdr *rec;
{
	int	n, p;

	if (read(pfd, (char *)rec, sizeof(*rec)) != sizeof(*rec))
		return -2;

	if (swapped) {
		rec->ph_clen = SWAPLONG(rec->ph_clen);
		rec->ph_len = SWAPLONG(rec->ph_len);
		rec->ph_ts.tv_sec = SWAPLONG(rec->ph_ts.tv_sec);
		rec->ph_ts.tv_usec = SWAPLONG(rec->ph_ts.tv_usec);
	}
	p = rec->ph_clen;
	n = MIN(p, rec->ph_len);
	if (!n || n < 0)
		return -3;

	return p;
}


#ifdef	notyet
/*
 * read an entire pcap packet record.  only the data part is copied into
 * the available buffer, with the number of bytes copied returned.
 */
static	int	pcap_read(buf, cnt)
char	*buf;
int	cnt;
{
	struct	pcap_pkthdr rec;
	static	char	*bufp = NULL;
	int	i, n;

	if ((i = pcap_read_rec(&rec)) <= 0)
		return i;

	if (!bufp)
		bufp = malloc(i);
	else
		bufp = realloc(bufp, i);

	if (read(pfd, bufp, i) != i)
		return -2;

	n = MIN(i, cnt);
	bcopy(bufp, buf, n);
	return n;
}
#endif


/*
 * return only an IP packet read into buf
 */
static	int	pcap_readip(buf, cnt, ifn, dir)
char	*buf, **ifn;
int	cnt, *dir;
{
	static	char	*bufp = NULL;
	struct	pcap_pkthdr rec;
	struct	llc	*l;
	char	*s, ty[4];
	char *new_str = NULL;
	int	i, n;

	l = llcp;

	/* do { */
		if ((i = pcap_read_rec(&rec)) <= 0)
			return i;

		if (!bufp)
			bufp = malloc(i);
		else
		{
			new_str = realloc(bufp, i);
			if(NULL == new_str)
			{
				free(bufp);
				return -2;
			}
			bufp = new_str;
		}
		s = bufp;

		if (read(pfd, s, i) != i)
			return -2;

		i -= l->lc_sz;
		s += l->lc_to;
		bcopy(s, ty, l->lc_tl);
		s += l->lc_tl;
	/* } while (ty[0] != 0x8 && ty[1] != 0); */
	n = MIN(i, cnt);
	bcopy(s, buf, n);
	return n;
}

//==========================================================================
//
//      include/netinet/ip_encap.h
//
//==========================================================================
// ####BSDCOPYRIGHTBEGIN####                                    
// -------------------------------------------                  
// This file is part of eCos, the Embedded Configurable Operating System.
//
// Portions of this software may have been derived from FreeBSD 
// or other sources, and if so are covered by the appropriate copyright
// and license included herein.                                 
//
// Portions created by the Free Software Foundation are         
// Copyright (C) 2002 Free Software Foundation, Inc.            
// -------------------------------------------                  
// ####BSDCOPYRIGHTEND####                                      
//==========================================================================

/*	$KAME: ip_encap.h,v 1.12 2001/09/04 08:43:18 itojun Exp $	*/

/*
 * Copyright (C) 1995, 1996, 1997, and 1998 WIDE Project.
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

#ifndef _NETINET_IP_ENCAP_H_
#define _NETINET_IP_ENCAP_H_

#ifdef _KERNEL

#ifndef RNF_NORMAL
#include <net/radix.h>
#endif

struct encaptab {
	struct radix_node nodes[2];
	LIST_ENTRY(encaptab) chain;
	int af;
	int proto;			/* -1: don't care, I'll check myself */
	struct sockaddr *addrpack;	/* malloc'ed, for radix lookup */
	struct sockaddr *maskpack;	/* ditto */
	struct sockaddr *src;		/* my addr */
	struct sockaddr *srcmask;
	struct sockaddr *dst;		/* remote addr */
	struct sockaddr *dstmask;
	int (*func) __P((const struct mbuf *, int, int, void *));
	const struct protosw *psw;	/* only pr_input will be used */
	void *arg;			/* passed via m->m_pkthdr.aux */
};

void	encap_init __P((void));
#if (defined(__FreeBSD__) && __FreeBSD__ >= 4)
void	encap4_input __P((struct mbuf *, int));
#else
void	encap4_input __P((struct mbuf *, ...));
#endif /* (defined(__FreeBSD__) && __FreeBSD__ >= 4) */
int	encap6_input __P((struct mbuf **, int *, int));
const struct encaptab *encap_attach __P((int, int, const struct sockaddr *,
	const struct sockaddr *, const struct sockaddr *,
	const struct sockaddr *, const struct protosw *, void *));
const struct encaptab *encap_attach_func __P((int, int,
	int (*) __P((const struct mbuf *, int, int, void *)),
	const struct protosw *, void *));
void	encap6_ctlinput __P((int, struct sockaddr *, void *));
int	encap_detach __P((const struct encaptab *));
void	*encap_getarg __P((struct mbuf *));
#endif

#endif /*_NETINET_IP_ENCAP_H_*/

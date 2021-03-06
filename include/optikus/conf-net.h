/* -*-c++-*-  vi: set ts=4 sw=4 :

  (C) Copyright 2006-2007, vitki.net. All rights reserved.

  This program is free software; you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation; either version 2 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA 02111-1307 USA

  $Date$
  $Revision$
  $Source$

  Network code portability.

*/

#ifndef OPTIKUS_CONF_NET_H
#define OPTIKUS_CONF_NETL_H

#if defined(HAVE_CONFIG_H)
#include <config.h>		/* for VXWORKS,SOLARIS,... */
#endif /* HAVE_CONFIG_H */

#if defined(VXWORKS)
#include <sockLib.h>		/* for sockaddr */
#include <ioLib.h>			/* for ioctl,FIONBIO */
#include <hostLib.h>		/* for hostGetByName */
#else /* UNIX */
#include <sys/types.h>		/* for socklen_t (if any) */
#include <sys/socket.h>		/* for sockaddr */
#include <fcntl.h>			/* for F_GETFL,F_SETFL,O_NONBLOCK */
#include <netdb.h>			/* for gethostbyname */
#endif /* UNIX vs VXWORKS */

#include <stdio.h>			/* for perror */
#include <netinet/in.h>		/* for sockaddr_in */

#include <optikus/conf-mem.h>	/* for OX_INLINE,memcpy */

OPTIKUS_BEGIN_C_DECLS


/* maximum of two file descriptors (for select) */
#define OMAX(a,b)		((a)>(b)?(a):(b))


/*				byte order				*/

#define OROTS(x)	(short)((((ushort_t)(x) & 0xff) << 8) \
							| (((ushort_t)(x) >> 8) & 0xff))
#define OROTL(x)	(long)(((((ulong_t)(x)) & 0xff) << 24) \
							| ((((ulong_t)(x) >> 8) & 0xff) << 16) \
							| ((((ulong_t)(x) >> 16) & 0xff) << 8) \
							| (((ulong_t)(x) >> 24) & 0xff))

#if defined(WORDS_BIGENDIAN)
#define ONTOHS(x)	((short)(x))
#define OHTONS(x)	((short)(x))
#define ONTOHL(x)	((long)(x))
#define OHTONL(x)	((long)(x))
#else /* !WORDS_BIGENDIAN */
#define ONTOHS(x)	OROTS(x)
#define OHTONS(x)	OROTS(x)
#define ONTOHL(x)	OROTL(x)
#define OHTONL(x)	OROTL(x)
#endif /* WORDS_BIGENDIAN */

#define OUNTOHS(x)	((ushort_t)ONTOHS(x))
#define OUHTONS(x)	((ushort_t)OHTONS(x))
#define OUNTOHL(x)	((ulong_t)ONTOHL(x))
#define OUHTONL(x)	((ulong_t)OHTONL(x))


/*
	FIXME: autoconfiscate.
	MSG_NOSIGNAL is an recv() option that disables SIGPIPE. Unfortunately,
	it is not defined in older versions of VxWorks and Solaris.
*/
#if defined(MSG_NOSIGNAL)
#define O_MSG_NOSIGNAL		MSG_NOSIGNAL
#else
#define O_MSG_NOSIGNAL		0
#endif


/*
	FIXME: autoconfiscate.
	Workaround for socklen_t which is not defined everywhere.
*/
#if (defined(VXWORKS) || defined(SOLARIS)) && !defined(_SOCKLEN_T)
typedef int socklen_t;
#endif


/*		resolve host name		*/

OX_INLINE int
inetResolveHostName(const char *hostname, struct sockaddr_in *addr_p)
{
	/* FIXME: autoconfiscate. */
#if defined(VXWORKS)
	int host_entry = hostGetByName((char *) hostname);
	if (host_entry == ERROR)
		return ERROR;
	addr_p->sin_family = AF_INET;
	addr_p->sin_addr.s_addr = host_entry;
#else /* UNIX */
	struct hostent *host_entry = gethostbyname(hostname);
	if (NULL == host_entry)
		return ERROR;
	memcpy(&(addr_p->sin_addr), host_entry->h_addr, host_entry->h_length);
	addr_p->sin_family = host_entry->h_addrtype;
#endif /* VXWORKS vx UNIX */
	return OK;
}


/*		socket options			*/

OX_INLINE int
setNonBlocking(int fd)
{
	/* FIXME: autoconfiscate. */
#if defined(VXWORKS)
	int on = 1;
	int ret = ioctl(fd, FIONBIO, (int) &on);
	if (ret == ERROR)
		perror("ioctl(FIONBIO)");
#else /* UNIX */
	int ret = fcntl(fd, F_GETFL, 0);
	if (ret == ERROR) {
		perror("fcntl(GET,O_NONBLOCK)");
	} else {
		ret = fcntl(fd, F_SETFL, ret | O_NONBLOCK);
		if (ret == ERROR)
			perror("fcntl(SET,O_NONBLOCK)");
	}
#endif /* UNIX vs VXWORKS */
	return ret;
}


OPTIKUS_END_C_DECLS

#endif /* OPTIKUS_CONF_NET_H */

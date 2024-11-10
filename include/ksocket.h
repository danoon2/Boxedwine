/*
 *  Copyright (C) 2016  The BoxedWine Team
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA 02111-1307, USA.
 */

#ifndef __KSOCKET_H__
#define __KSOCKET_H__

#define	K_SOCK_STREAM 1
#define K_SOCK_DGRAM 2
#define K_SOCK_RAW 3
#define K_SOCK_RDM 4
#define K_SOCK_SEQPACKET 5

#define	K_AF_UNIX      1
#define	K_AF_INET      2
#define K_AF_INET6     10
#define K_AF_NETLINK   16

#define	K_PF_UNIX      1
#define	K_PF_INET      2

#define K_MSG_OOB 1
#define	K_MSG_PEEK     0x2
#define K_MSG_NOSIGNAL 0x4000

#define K_SHUT_RD      0
#define K_SHUT_WR      1
#define K_SHUT_RDWR    2

#define K_SO_DEBUG     1
#define K_SO_REUSEADDR 2
#define K_SO_TYPE      3
#define	K_SO_ERROR     4
#define K_SO_DONTROUTE 5
#define K_SO_BROADCAST 6
#define K_SO_SNDBUF    7
#define K_SO_RCVBUF    8
#define K_SO_KEEPALIVE 9
#define K_SO_OOBINLINE 10
#define K_SO_NO_CHECK  11
#define K_SO_PRIORITY  12
#define K_SO_LINGER    13
#define K_SO_BSDCOMPAT 14
#define K_SO_REUSEPORT 15
#define K_SO_PASSCRED  16
#define K_SO_PEERCRED  17
#define K_SO_RCVLOWAT  18
#define K_SO_SNDLOWAT  19
#define K_SO_RCVTIMEO  20
#define K_SO_SNDTIMEO  21
#define K_SO_ATTACH_FILTER  26
#define K_SO_DETACH_FILTER  27
#define K_SO_PEERNAME  28
#define K_SO_TIMESTAMP 29
#define K_SO_ACCEPTCONN 30
#define K_SO_SNDBUFFORCE 32
#define K_SO_RCVBUFFORCE 33
#define K_SO_PROTOCOL     38
#define K_SO_DOMAIN       39

#define K_IPPROTO_IP 0
#define K_SOL_SOCKET  1
#define K_IPPROTO_TCP 6

#define K_SCM_RIGHTS  1

#define K_IP_TOS      1
#define K_IP_TTL      2
#define K_IP_HDRINCL  3
#define K_IP_OPTIONS  4
#define K_IP_ROUTER_ALERT 5
#define K_IP_RECVOPTS 6
#define K_IP_RETOPTS  7
#define K_IP_PKTINFO  8
#define K_IP_PKTOPTIONS   9
#define K_IP_MTU_DISCOVER 10
#define K_IP_RECVERR  11
#define K_IP_RECVTTL  12
#define K_IP_RECVTOS  13
#define K_IP_MTU      14
#define K_IP_FREEBIND 15
#define K_IP_IPSEC_POLICY 16
#define K_IP_XFRM_POLICY  17
#define K_IP_PASSSEC  18
#define K_IP_TRANSPARENT  19

#define K_TCP_NODELAY		1	/* Turn off Nagle's algorithm. */
#define K_TCP_MAXSEG		2	/* Limit MSS */
#define K_TCP_CORK		3	/* Never send partially complete segments */
#define K_TCP_KEEPIDLE		4	/* Start keeplives after this period */
#define K_TCP_KEEPINTVL		5	/* Interval between keepalives */
#define K_TCP_KEEPCNT		6	/* Number of keepalives before death */
#define K_TCP_SYNCNT		7	/* Number of SYN retransmits */
#define K_TCP_LINGER2		8	/* Life time of orphaned FIN-WAIT-2 state */
#define K_TCP_DEFER_ACCEPT	9	/* Wake up listener only when data arrive */
#define K_TCP_WINDOW_CLAMP	10	/* Bound advertised window */
#define K_TCP_INFO		11	/* Information about this connection. */
#define K_TCP_QUICKACK		12	/* Block/reenable quick acks */
#define K_TCP_CONGESTION		13	/* Congestion control algorithm */
#define K_TCP_MD5SIG		14	/* TCP MD5 Signature (RFC2385) */
#define K_TCP_THIN_LINEAR_TIMEOUTS 16      /* Use linear timeouts for thin streams*/
#define K_TCP_THIN_DUPACK         17      /* Fast retrans. after 1 dupack */
#define K_TCP_USER_TIMEOUT	18	/* How long for loss retry before timeout */
#define K_TCP_REPAIR		19	/* TCP sock is under repair right now */
#define K_TCP_REPAIR_QUEUE	20
#define K_TCP_QUEUE_SEQ		21
#define K_TCP_REPAIR_OPTIONS	22
#define K_TCP_FASTOPEN		23	/* Enable FastOpen on listeners */
#define K_TCP_TIMESTAMP		24
#define K_TCP_NOTSENT_LOWAT	25	

U32 ksocket(U32 domain, U32 type, U32 protocol);
U32 kbind(KThread* thread, U32 socket, U32 address, U32 len);
U32 kconnect(KThread* thread, U32 socket, U32 address, U32 len);
U32 klisten(KThread* thread, U32 socket, U32 backog);
U32 kaccept(KThread* thread, U32 socket, U32 address, U32 len, U32 flags);
U32 kgetsockname(KThread* thread, U32 socket, U32 address, U32 len);
U32 kgetpeername(KThread* thread, U32 socket, U32 address, U32 len);
U32 ksocketpair(KThread* thread, U32 af, U32 type, U32 protocol, U32 socks, U32 flags);
U32 ksend(KThread* thread, U32 socket, U32 buffer, U32 len, U32 flags);
U32 krecv(KThread* thread, U32 socket, U32 buffer, U32 len, U32 flags);
U32 kshutdown(KThread* thread, U32 socket, U32 how);
U32 ksetsockopt(KThread* thread, U32 socket, U32 level, U32 name, U32 value, U32 len);
U32 kgetsockopt(KThread* thread, U32 socket, U32 level, U32 name, U32 value, U32 len);
U32 ksendmsg(KThread* thread, U32 socket, U32 msg, U32 flags);
U32 ksendmmsg(KThread* thread, U32 socket, U32 address, U32 vlen, U32 flags);
U32 krecvmsg(KThread* thread, U32 socket, U32 msg, U32 flags);
U32 ksendto(KThread* thread, U32 socket, U32 message, U32 length, U32 flags, U32 dest_addr, U32 dest_len);
U32 krecvfrom(KThread* thread, U32 socket, U32 buffer, U32 length, U32 flags, U32 address, U32 address_len);

BString socketAddressName(KMemory* memory, U32 address, U32 len);
bool isNativeSocket(KThread* thread, int desc);
#endif
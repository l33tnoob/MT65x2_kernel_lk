/*
 * U-boot - errno.h Error number defines
 *
 * Copyright (c) 2005-2007 Analog Devices Inc.
 *
 * See file CREDITS for list of people who contributed to this
 * project.
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as
 * published by the Free Software Foundation; either version 2 of
 * the License, or (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin St, Fifth Floor, Boston,
 * MA 02110-1301 USA
 */

#ifndef _GENERIC_ERRNO_H
#define _GENERIC_ERRNO_H

#define	EMEDIUMTYPE	124	/* Wrong medium type */
#define	ENOMEDIUM	123	/* No medium found */
#define	EDQUOT		122	/* Quota exceeded */
#define	EREMOTEIO	121	/* Remote I/O error */
#define	EISNAM		120	/* Is a named type file */
#define	ENAVAIL		119	/* No XENIX semaphores available */
#define	ENOTNAM		118	/* Not a XENIX named type file */
#define	EUCLEAN		117	/* Structure needs cleaning */
#define	ESTALE		116	/* Stale NFS file handle */
#define	EINPROGRESS	115	/* Operation now in progress */
#define	EALREADY	114	/* Operation already in progress */
#define	EHOSTUNREACH	113	/* No route to host */
#define	EHOSTDOWN	112	/* Host is down */
#define	ECONNREFUSED	111	/* Connection refused */

#define	ETIMEDOUT	110	/* Connection timed out */
#define	ETOOMANYREFS	109	/* Too many references: cannot splice */
#define	ESHUTDOWN	108	/* Cannot send after transport endpoint shutdown */
#define	ENOTCONN	107	/* Transport endpoint is not connected */
#define	EISCONN		106	/* Transport endpoint is already connected */
#define	ENOBUFS		105	/* No buffer space available */
#define	ECONNRESET	104	/* Connection reset by peer */
#define	ECONNABORTED	103	/* Software caused connection abort */
#define	ENETRESET	102	/* Network dropped connection because of reset */
#define	ENETUNREACH	101	/* Network is unreachable */

#define	ENETDOWN	100	/* Network is down */
#define	EADDRNOTAVAIL	99	/* Cannot assign requested address */
#define	EADDRINUSE	98	/* Address already in use */
#define	EAFNOSUPPORT	97	/* Address family not supported by protocol */
#define	EPFNOSUPPORT	96	/* Protocol family not supported */
#define	EOPNOTSUPP	95	/* Operation not supported on transport endpoint */
#define	ESOCKTNOSUPPORT	94	/* Socket type not supported */
#define	EPROTONOSUPPORT	93	/* Protocol not supported */
#define	ENOPROTOOPT	92	/* Protocol not available */
#define	EPROTOTYPE	91	/* Protocol wrong type for socket */

#define	EMSGSIZE	90	/* Message too long */
#define	EDESTADDRREQ	89	/* Destination address required */
#define	ENOTSOCK	88	/* Socket operation on non-socket */
#define	EUSERS		87	/* Too many users */
#define	ESTRPIPE	86	/* Streams pipe error */
#define	ERESTART	85	/* Interrupted system call should be restarted */
#define	EILSEQ		84	/* Illegal byte sequence */
#define	ELIBEXEC	83	/* Cannot exec a shared library directly */
#define	ELIBMAX		82	/* Attempting to link in too many shared libraries */
#define	ELIBSCN		81	/* .lib section in a.out corrupted */

#define	ELIBBAD		80	/* Accessing a corrupted shared library */
#define	ELIBACC		79	/* Can not access a needed shared library */
#define	EREMCHG		78	/* Remote address changed */
#define	EBADFD		77	/* File descriptor in bad state */
#define	ENOTUNIQ	76	/* Name not unique on network */
#define	EOVERFLOW	75	/* Value too large for defined data type */
#define	EBADMSG		74	/* Not a data message */
#define	EDOTDOT		73	/* RFS specific error */
#define	EMULTIHOP	72	/* Multihop attempted */
#define	EPROTO		71	/* Protocol error */

#define	ECOMM		70	/* Communication error on send */
#define	ESRMNT		69	/* Srmount error */
#define	EADV		68	/* Advertise error */
#define	ENOLINK		67	/* Link has been severed */
#define	EREMOTE		66	/* Object is remote */
#define	ENOPKG		65	/* Package not installed */
#define	ENONET		64	/* Machine is not on the network */
#define	ENOSR		63	/* Out of streams resources */
#define	ETIME		62	/* Timer expired */
#define	ENODATA		61	/* No data available */

#define	ENOSTR		60	/* Device not a stream */
#define	EBFONT		59	/* Bad font file format */
#define	EDEADLOCK	EDEADLK
#define	EBADSLT		57	/* Invalid slot */
#define	EBADRQC		56	/* Invalid request code */
#define	ENOANO		55	/* No anode */
#define	EXFULL		54	/* Exchange full */
#define	EBADR		53	/* Invalid request descriptor */
#define	EBADE		52	/* Invalid exchange */
#define	EL2HLT		51	/* Level 2 halted */

#define	ENOCSI		50	/* No CSI structure available */
#define	EUNATCH		49	/* Protocol driver not attached */
#define	ELNRNG		48	/* Link number out of range */
#define	EL3RST		47	/* Level 3 reset */
#define	EL3HLT		46	/* Level 3 halted */
#define	EL2NSYNC	45	/* Level 2 not synchronized */
#define	ECHRNG		44	/* Channel number out of range */
#define	EIDRM		43	/* Identifier removed */
#define	ENOMSG		42	/* No message of desired type */
#define	EWOULDBLOCK	EAGAIN	/* Operation would block */

#define	ELOOP		40	/* Too many symbolic links encountered */
#define	ENOTEMPTY	39	/* Directory not empty */
#define	ENOSYS		38	/* Function not implemented */
#define	ENOLCK		37	/* No record locks available */
#define	ENAMETOOLONG	36	/* File name too long */
#define	EDEADLK		35	/* Resource deadlock would occur */
#define	ERANGE		34	/* Math result not representable */
#define	EDOM		33	/* Math argument out of domain of func */
#define	EPIPE		32	/* Broken pipe */
#define	EMLINK		31	/* Too many links */

#define	EROFS		30	/* Read-only file system */
#define	ESPIPE		29	/* Illegal seek */
#define	ENOSPC		28	/* No space left on device */
#define	EFBIG		27	/* File too large */
#define	ETXTBSY		26	/* Text file busy */
#define	ENOTTY		25	/* Not a typewriter */
#define	EMFILE		24	/* Too many open files */
#define	ENFILE		23	/* File table overflow */
#define	EINVAL		22	/* Invalid argument */
#define	EISDIR		21	/* Is a directory */

#define	ENOTDIR		20	/* Not a directory */
#define	ENODEV		19	/* No such device */
#define	EXDEV		18	/* Cross-device link */
#define	EEXIST		17	/* File exists */
#define	EBUSY		16	/* Device or resource busy */
#define	ENOTBLK		15	/* Block device required */
#define	EFAULT		14	/* Bad address */
#define	EACCES		13	/* Permission denied */
#define	ENOMEM		12	/* Out of memory */
#define	EAGAIN		11	/* Try again */

#define	ECHILD		10	/* No child processes */
#define	EBADF		9	/* Bad file number */
#define	ENOEXEC		8	/* Exec format error */
#define	E2BIG		7	/* Argument list too long */
#define	ENXIO		6	/* No such device or address */
#define	EIO		5	/* I/O error */
#define	EINTR		4	/* Interrupted system call */
#define	ESRCH		3	/* No such process */
#define	ENOENT		2	/* No such file or directory */
#define	EPERM		1	/* Operation not permitted */

#endif

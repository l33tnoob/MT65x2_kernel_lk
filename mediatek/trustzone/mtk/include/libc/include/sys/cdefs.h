

#ifndef	_SYS_CDEFS_H_
#define	_SYS_CDEFS_H_

#if defined(__cplusplus)
#define	__BEGIN_DECLS		extern "C" {
#define	__END_DECLS		}
#define	__static_cast(x,y)	static_cast<x>(y)
#else
#define	__BEGIN_DECLS
#define	__END_DECLS
#define	__static_cast(x,y)	(x)y
#endif

#endif /* _SYS_CDEFS_H_ */


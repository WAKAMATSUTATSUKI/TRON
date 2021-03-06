/*
 *----------------------------------------------------------------------
 *    T2EX Software Package
 *
 *    Copyright 2015 by Nina Petipa.
 *    This software is distributed under the latest version of T-License 2.x.
 *----------------------------------------------------------------------
 *
 *----------------------------------------------------------------------
 */
/*
 * This software package is available for use, modification, 
 * and redistribution in accordance with the terms of the attached 
 * T-License 2.x.
 * If you want to redistribute the source code, you need to attach 
 * the T-License 2.x document.
 * There's no obligation to publish the content, and no obligation 
 * to disclose it to the TRON Forum if you have modified the 
 * software package.
 * You can also distribute the modified source code. In this case, 
 * please register the modification to T-Kernel traceability service.
 * People can know the history of modifications by the service, 
 * and can be sure that the version you have inherited some 
 * modification of a particular version or not.
 *
 *    http://trace.tron.org/tk/?lang=en
 *    http://trace.tron.org/tk/?lang=ja
 *
 * As per the provisions of the T-License 2.x, TRON Forum ensures that 
 * the portion of the software that is copyrighted by Ken Sakamura or 
 * the TRON Forum does not infringe the copyrights of a third party.
 * However, it does not make any warranty other than this.
 * DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
 * responsible for any consequences or damages caused directly or
 * indirectly by the use of this software package.
 *
 * The source codes in bsd_source.tar.gz in this software package are 
 * derived from NetBSD or OpenBSD and not covered under T-License 2.x.
 * They need to be changed or redistributed according to the 
 * representation of each source header.
 */

#ifndef	__BK_UAPI_STAT_H__
#define	__BK_UAPI_STAT_H__

#include <basic.h>
#include <stdint.h>
#include <tk/typedef.h>
#include <sys/types.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/

/*
==================================================================================

	DEFINE 

==================================================================================
*/
/*
----------------------------------------------------------------------------------
	stat
----------------------------------------------------------------------------------
*/
struct stat {
	dev_t	st_dev;		/* inode's device */
	ino_t	st_ino;		/* inode's number */
	mode_t	st_mode;	/* inode protection mode */
	nlink_t	st_nlink;	/* number of hard links */
	uid_t	st_uid;		/* user ID of the file's owner */
	gid_t	st_gid;		/* group ID of the file's group */
	dev_t	st_rdev;	/* Device ID */
	off_t	st_size;	/* file size, in bytes */
	time_t	st_atime;	/* time of last access (sec) */
	time_t	st_mtime;	/* time of last data modification (sec) */
	time_t	st_ctime;	/* time of last file status change (sec) */
	blksize_t st_blksize;	/* I/O block size */
	blkcnt_t  st_blocks;	/* Number of blocks allocated */
};

/*
----------------------------------------------------------------------------------
	stat64 for long file size
----------------------------------------------------------------------------------
*/
struct stat64 {
	dev_t	st_dev;		/* inode's device				*/
	ino_t	st_ino;		/* inode's number				*/
	mode_t	st_mode;	/* inode protection mode			*/
	nlink_t	st_nlink;	/* number of hard links				*/
	uid_t	st_uid;		/* user ID of the file's owner			*/
	gid_t	st_gid;		/* group ID of the file's group			*/
	dev_t	st_rdev;	/* Device ID					*/
	off64_t	st_size;	/* file size, in bytes				*/
	blksize_t st_blksize;	/* I/O block size				*/
	blkcnt_t  st_blocks;	/* Number of blocks allocated			*/
	time_t	st_atime;	/* time of last access (sec)			*/
	time_t	st_mtime;	/* time of last data modification (sec)		*/
	time_t	st_ctime;	/* time of last file status change (sec)	*/

};

/*
----------------------------------------------------------------------------------
	stat64 for long file size (for i386)
----------------------------------------------------------------------------------
*/
struct stat64_i386 {
#if 1
	uint64_t	st_dev;		/* inode's device			*/
	char		_pad0[4];	/* padding for glibc			*/
	uint32_t	_st_ino;	/* inode's number			*/
	mode_t		st_mode;	/* inode protection mode		*/
	nlink_t		st_nlink;	/* number of hard links			*/
	uid_t		st_uid;		/* user ID of the file's owner		*/
	gid_t		st_gid;		/* group ID of the file's group		*/
	uint64_t	st_rdev;	/* Device ID				*/
	char		_pad1[4];	/* padding for glibc			*/
	off64_t		st_size;	/* file size, in bytes			*/
	uint32_t	st_blksize;	/* I/O block size			*/
	uint64_t	st_blocks;	/* Number of blocks allocated		*/
	time_t		st_atime;	/* time of last access (sec)		*/
	time_t		st_atime_nsec;	/* time of last access (nano sec)	*/
	time_t		st_mtime;	/* time of last data modification (sec)	*/
	time_t		st_mtime_nsec;	/* time of last data modification (nano)*/
	time_t		st_ctime;	/* time of last file status change (sec)*/
	time_t		st_ctime_nsec;	/* time of last file status change (nano)*/
	uint64_t	st_ino;	/* inode's number			*/
#else
	mode_t		st_mode;
	uint64_t	st_ino;
	dev_t		st_dev;
	nlink_t		st_nlink;
	uid_t		st_uid;
	gid_t		st_gid;
	off64_t		st_size;
	time_t		st_atime;
	time_t		st_mtime;
	time_t		st_ctime;
#endif
};

#define	S_BLKSIZE	512	/* Block size (number of bytes)
				   to be used in st_blocks */

/*
 * File mode (st_mode)
 */
#define	S_IRWXU		0x01C0U	/* 0000700 Owner RWX mask */
#define	S_IRUSR		0x0100U	/* 0000400 Owner R read enable */
#define	S_IWUSR		0x0080U	/* 0000200 Owner W write enable */
#define	S_IXUSR		0x0040U	/* 0000100 Owner X execution enable */

#define	S_IUSR_SHIFT	6

#define	S_IRWXG		0x0038U	/* 0000070 Group RWX mask */
#define	S_IRGRP		0x0020U	/* 0000040 Group R read enable */
#define	S_IWGRP		0x0010U	/* 0000020 Group W write enable */
#define	S_IXGRP		0x0008U	/* 0000010 Group X execution enable */

#define	S_IGRP_SHIFT	3

#define	S_IRWXO		0x0007U	/* 0000007 Other RWX mask */
#define	S_IROTH		0x0004U	/* 0000004 Other R read enable */
#define	S_IWOTH		0x0002U	/* 0000002 Other W write enable */
#define	S_IXOTH		0x0001U	/* 0000001 Other X execution enable */

#define	S_ISUID		0x0800U	/* 0004000 Runtime user ID setting */
#define	S_ISGID		0x0400U	/* 0002000 Runtime group ID setting */
#define	S_ISVTX		0x0200U	/* 0001000 Sticky bit */

#define	S_IFMT		0xF000U	/* 0170000 File type mask */
#define	S_IFIFO		0x1000U	/* 0010000 Name pipe (FIFO) */
#define	S_IFCHR		0x2000U	/* 0020000 Character-type special file */
#define	S_IFDIR		0x4000U	/* 0040000 Directory */
#define	S_IFBLK		0x6000U	/* 0060000 Block-type special file */
#define	S_IFREG		0x8000U	/* 0100000 Normal file */
#define	S_IFLNK		0xA000U	/* 0120000 Symbolic link */
#define	S_IFSOCK	0xC000U	/* 0140000 Socket */

#define	S_IF_SHIFT	12

#define	S_ISFIFO(m)	( ((m) & S_IFMT) == S_IFIFO )
#define	S_ISCHR(m)	( ((m) & S_IFMT) == S_IFCHR )
#define	S_ISDIR(m)	( ((m) & S_IFMT) == S_IFDIR )
#define	S_ISBLK(m)	( ((m) & S_IFMT) == S_IFBLK )
#define	S_ISREG(m)	( ((m) & S_IFMT) == S_IFREG )
#define	S_ISLNK(m)	( ((m) & S_IFMT) == S_IFLNK )
#define	S_ISSOCK(m)	( ((m) & S_IFMT) == S_IFSOCK )

/*
==================================================================================

	Management 

==================================================================================
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/

#endif	// __BK_UAPI_STAT_H__

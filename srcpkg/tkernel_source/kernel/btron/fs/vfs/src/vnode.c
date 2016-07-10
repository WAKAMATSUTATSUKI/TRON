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

#include <bk/kernel.h>
#include <bk/unistd.h>
#include <bk/fs/vfs.h>
#include <bk/memory/slab.h>
#include <bk/uapi/sys/stat.h>

#include <t2ex/limits.h>

#include <tstdlib/round.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
LOCAL struct file_name*
lookup_at(struct path *dir_path, const struct qstr *path_name, unsigned int flags);

/*
==================================================================================

	DEFINE 

==================================================================================
*/

/*
==================================================================================

	Management 

==================================================================================
*/
LOCAL struct kmem_cache *vnode_cache;
LOCAL const char vnode_cache_name[] = "vnode_cache";

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Open Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_vnode
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize vnode management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int init_vnode(void)
{
	vnode_cache = kmem_cache_create(vnode_cache_name,
					sizeof(struct vnode), 0, 0, NULL);
	
	if (UNLIKELY(!vnode_cache)) {
		vd_printf("error:vnode_cache\n");
		return(-ENOMEM);
	}
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_vnode_cache
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy a vnode cache
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void destroy_vnode_cache(void)
{
	kmem_cache_destroy(vnode_cache);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vnode_cache_alloc
 Input		:void
 Output		:void
 Return		:struct vnode*
 		 < vnode object >
 Description	:allocate a vnode object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT struct vnode* vnode_cache_alloc(void)
{
	struct vnode *vnode = (struct vnode*)kmem_cache_alloc(vnode_cache, 0);
	
	if (UNLIKELY(!vnode)) {
		return(vnode);
	}
	
	INIT_VNODE(vnode);
	
	return(vnode);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vnode_cache_free
 Input		:struct vnode *vnode
 		 < vnode to free >
 Output		:void
 Return		:void
 Description	:free a vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void vnode_cache_free(struct vnode *vnode)
{
	kmem_cache_free(vnode_cache, vnode);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:INIT_VNODE
 Input		:struct vnode *vnode
 		 < vnode object to initialize >
 Output		:void
 Return		:void
 Description	:initialize vnode object
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void INIT_VNODE(struct vnode *vnode)
{
	memset((void*)vnode, 0x00, sizeof(struct vnode));
	
	init_list(&vnode->v_dentry);
	init_list(&vnode->v_devices);
	init_list(&vnode->v_sb_list);
	init_list(&vnode->v_lru);
	init_list(&vnode->v_cache);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:setup_vnode
 Input		:struct vnode *dir
 		 < directory vnode >
 		 struct dentry *dentry
 		 < dentry of a file >
 		 struct vnode *vnode
 		 < vnode to set up >
 Output		:void
 Return		:int
 		 < result >
 Description	:set up a vnode. before calling this function, must be set v_mode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
setup_vnode(struct vnode *dir, struct dentry *dentry, struct vnode *vnode)
{
	/* -------------------------------------------------------------------- */
	/* file attributes							*/
	/* -------------------------------------------------------------------- */
	vnode->v_uid = dir->v_uid;
	vnode->v_gid = dir->v_gid;
	vnode->v_nlink++;
	atomic_inc(&vnode->v_count);
	vnode->v_mode |= dir->v_mode & ~S_IFMT;
	
	/* -------------------------------------------------------------------- */
	/* set up vnode								*/
	/* -------------------------------------------------------------------- */
	vnode->v_op = dir->v_op;
	vnode->v_fops = dir->v_fops;
	
	/* -------------------------------------------------------------------- */
	/* set up dentry relationship						*/
	/* -------------------------------------------------------------------- */
	dentry_associated(dentry, vnode);
	//add_list(&dentry->d_parent->d_subdirs, &dentry->d_child);
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:init_special_vnode
 Input		:struct vnode *vnode
 		 < special vnode >
 		 umode_t mode
 		 < mode >
 		 dev_t dev
 		 < device number >
 Output		:void
 Return		:void
 Description	:initialize a special vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void init_special_vnode(struct vnode *vnode, umode_t mode, dev_t dev)
{
	vnode->v_mode = mode;
	
	if (S_ISCHR(mode)) {
		vnode->v_rdev = dev;
		vnode->v_size = 1024;
	} else if (S_ISBLK(mode)) {
		vnode->v_rdev = dev;
	} else if (S_ISFIFO(mode)) {
	}
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kmknod
 Input		:const char *pathname
 		 < pathname to create a special file >
 		 mode_t mode
 		 < file creation mode >
 		 dev_t dev
 		 < device number of a special file >
 Output		:void
 Return		:int
 		 < result >
 Description	:create a special file in kernel space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int kmknod(const char *pathname, mode_t mode, dev_t dev)
{
	struct file_name *filename;
	struct dentry *dentry;
	struct vnode *dir;
	int err;
	
	err = vfs_lookup(pathname, &filename, LOOKUP_CREATE);
	
	if (err) {
		put_file_name(filename);
		return(err);
	}
	
	dir = filename->parent->d_vnode;
	dentry = filename->dentry;
	
	dentry_add_dir(filename->parent, filename->dentry);
	
	put_file_name(filename);
	
	return(vfs_mknod(dir, dentry, mode, dev));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:kmkdir
 Input		:const char *pathname
 		 < path name for new directory >
 		 mode_t mode
 		 < file creation mode >
 Output		:void
 Return		:int
 		 < result >
 Description	:make a new directory in kernel space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int kmkdir(const char *pathname, mode_t mode)
{
	struct file_name *fname;
	struct dentry *dentry;
	struct vnode *dir;
	int err;
	
	err = vfs_lookup(pathname, &fname, LOOKUP_CREATE);
	
	if (err) {
		put_file_name(fname);
		return(err);
	}
	
	dir = fname->parent->d_vnode;
	dentry = fname->dentry;
	
	dentry_add_dir(fname->parent, fname->dentry);
	
	put_file_name(fname);
	
	return(vfs_mkdir(dir, dentry, mode));
}

/*
----------------------------------------------------------------------------------
	system call operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:mknod
 Input		:const char *pathname
 		 < pathname to create a special file >
 		 mode_t mode
 		 < file creation mode >
 		 dev_t dev
 		 < device number of a special file >
 Output		:void
 Return		:int
 		 < result >
 Description	:create a special file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int mknod(const char *pathname, mode_t mode, dev_t dev)
{
	struct file_name *filename;
	struct dentry *dentry;
	struct vnode *dir;
	int err;
	
	//printf("mknod:%s\n", pathname);
	
	if (UNLIKELY(!pathname)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)pathname, sizeof(char), PROT_READ);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	err = vfs_lookup(pathname, &filename, LOOKUP_CREATE);
	
	if (err) {
		put_file_name(filename);
		return(err);
	}
	
	dir = filename->parent->d_vnode;
	dentry = filename->dentry;
	
	dentry_add_dir(filename->parent, filename->dentry);
	
	put_file_name(filename);
	
	return(vfs_mknod(dir, dentry, mode, dev));
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:mkdir
 Input		:const char *pathname
 		 < path name for new directory >
 		 mode_t mode
 		 < file creation mode >
 Output		:void
 Return		:int
 		 < result >
 Description	:make a new directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int mkdir(const char *pathname, mode_t mode)
{
	struct file_name *fname;
	struct dentry *dentry;
	struct vnode *dir;
	int err;
	
	if (UNLIKELY(!pathname)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)pathname, sizeof(char), PROT_READ);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	err = vfs_lookup(pathname, &fname, LOOKUP_CREATE);
	
	if (err) {
		put_file_name(fname);
		return(err);
	}
	
	//vd_printf("mkdir:%s\n", pathname);
	
	dir = fname->parent->d_vnode;
	dentry = fname->dentry;
	
	dentry_add_dir(fname->parent, fname->dentry);
	
	put_file_name(fname);
	
	return(vfs_mkdir(dir, dentry, mode));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:symlink
 Input		:const char *target
 		 < target name >
 		 const char *linkpath
 		 < link path name >
 Output		:void
 Return		:int
 		 < result >
 Description	:make a symbolic link. linkpath is a symbolic link which has
 		 a contents of a target path.
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int symlink(const char *target, const char *linkpath)
{
	struct file_name *fname;
	struct dentry *dentry;
	struct vnode *dir;
	char *target_path;
	char *symname;
	ssize_t len;
	int err;
	
	if (UNLIKELY(!target)) {
		return(-EFAULT);
	}
	
	if (UNLIKELY(!linkpath)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)target, sizeof(char), PROT_READ);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)linkpath, sizeof(char), PROT_READ);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	err = vfs_lookup(linkpath, &fname, LOOKUP_CREATE);
	
	if (err) {
		put_file_name(fname);
		return(err);
	}
	
	//vd_printf("symlink:%s to %s\n", linkpath, target);
	
	dir = fname->parent->d_vnode;
	dentry = fname->dentry;
	
	dentry_add_dir(fname->parent, fname->dentry);
	
	put_file_name(fname);
	
	target_path = kmalloc(PATH_MAX + 1, 0);
	
	if (UNLIKELY(!target_path)) {
		return(-ENOMEM);
	}
	
	target_path = strncpy(target_path, target, PATH_MAX + 1);
	
	len = strnlen(target_path, PATH_MAX + 1);
	
	if (UNLIKELY(PATH_MAX <= len)) {
		kfree(target_path);
		return(-ENAMETOOLONG);
	}
	
	if (UNLIKELY(len <= 0)) {
		kfree(target_path);
		return(len);
	}
	
	symname = kstrndup(target_path, PATH_MAX, 0);
	
	if (UNLIKELY(!symname)) {
		kfree(target_path);
		return(-ENOMEM);
	}
	
	kfree(target_path);
	
	return(vfs_symlink(dir, dentry, symname));
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:stat64
 Input		:const char *path
 		 < file path to get its status >
 		 struct stat64_i386 *buf
 		 < stat structure >
 Output		:struct stat64_i386 *buf
 		 < stat structure >
 Return		:int
 		 < result >
 Description	:get file status
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int stat64(const char *path, struct stat64_i386 *buf)
{
	struct file_name *fname;
	struct vnode *vnode;
	int err;
	
	//printf("stat64:path[%s] ", path);
	
	
	if (UNLIKELY(!path)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)buf, sizeof(struct stat64_i386), PROT_WRITE);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	err = vfs_lookup(path, &fname, LOOKUP_ENTRY);
	
	if (err) {
		//printf("err:%d\n", -err);
		return(err);
	} else {
		//printf("\n");
	}
	
	vnode = fname->dentry->d_vnode;
	
	put_file_name(fname);
	
	return(vfs_stat64(vnode, buf));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:lstat64
 Input		:const char *path
 		 < file path to get its status >
 		 struct stat64_i386 *buf
 		 < stat structure >
 Output		:struct stat64_i386 *buf
 		 < stat structure >
 Return		:int
 		 < result >
 Description	:get file status
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int lstat64(const char *path, struct stat64_i386 *buf)
{
	struct file_name *fname;
	struct vnode *vnode;
	int err;
	
	//printf("lstat64:path[%s] ", path);
	
	
	if (UNLIKELY(!path)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)buf, sizeof(struct stat64_i386), PROT_WRITE);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	err = vfs_lookup(path, &fname, LOOKUP_ENTRY);
	
	if (err) {
		//printf("err:%d\n", -err);
		return(err);
	}
	
	vnode = fname->dentry->d_vnode;
	
	put_file_name(fname);
	
	
	err = vfs_stat64(vnode, buf);
	
	return(err);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:fstat64
 Input		:int fd
 		 < open file descriptor >
 		 struct stat64_i386 *buf
 		 < stat structure >
 Output		:struct stat64_i386 *buf
 		 < stat structure >
 Return		:int
 		 < result >
 Description	:get file status
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int fstat64(int fd, struct stat64_i386 *buf)
{
	struct file *filp;
	struct vnode *vnode;
	int err;
	
	//printf("fstat64:fd=%d\n", fd);
	
	err = vm_check_access((void*)buf, sizeof(struct stat64_i386), PROT_WRITE);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	if (UNLIKELY(get_soft_limit(RLIMIT_NOFILE) < fd)) {
		return(-EBADF);
	}
	
	filp = get_open_file(fd);
	
	if (UNLIKELY(!filp)) {
		return(-EBADF);
	}
	
	vnode = filp->f_vnode;
	
	return(vfs_stat64(vnode, buf));
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:access
 Input		:const char *pathname
 		 < file path name to check permissions >
 		 int mode
 		 < permission >
 Output		:void
 Return		:int
 		 < result >
 Description	:check real user's permissions for a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int access(const char *pathname, int mode)
{
	struct file_name *fname;
	struct dentry *dentry;
	struct vnode *vnode;
	int err;
	
	//printf("access:%s\n", pathname);
	
	if (UNLIKELY(!pathname)) {
		return(-EFAULT);
	}
	
	err = vm_check_access((void*)pathname, sizeof(char), PROT_READ);
	
	if (UNLIKELY(err)) {
		return(-EFAULT);
	}
	
	err = vfs_lookup(pathname, &fname, LOOKUP_ENTRY);
	
	if (err) {
		return(err);
	}
	
	dentry = fname->dentry;
	put_file_name(fname);
	
	vnode = dentry->d_vnode;
	
	return(vfs_access(vnode, mode));
	
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:fcntl
 Input		:int fd
 		 < open file descriptor >
 		 int cmd
 		 < file control command >
 		 ...
 		 < arguments >
 Output		:void
 Return		:int
 		 < result >
 Description	:file control
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
SYSCALL int fcntl(int fd, int cmd, ...)
{
	panic("%s is not implemented\n", __func__);
	return(0);
}

/*
----------------------------------------------------------------------------------
	vfs vnode operations
----------------------------------------------------------------------------------
*/
/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_lookup_at
 Input		:struct path *dir_path
 		 < directory path >
 		 const char *pathname
 		 < path name to look up >
 		 struct file_name **found
 		 < looked up dentry result >
 		 unsigned int flags
 		 < lookup flags >
 Output		:struct file_name **found
 		 < looked up dentry result >
 Return		:int
 		 < result >
 Description	:lookup dentry relative to a directory vnode
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
vfs_lookup_at(struct path *dir_path, const char *pathname,
			struct file_name **found, unsigned int flags)
{
	struct qstr *kpathname;
	int err;
	
	if (UNLIKELY(!pathname)) {
		err = -EFAULT;
		return(err);
	}
	
	kpathname = dup_pathname(pathname);
	
	if (UNLIKELY(!kpathname)) {
		vd_printf("failed dup pathname\n");
		err = -EFAULT;
		goto error_out;
	}
	
	if (UNLIKELY(kpathname == PTR_ERR_NAME_TOO_LONG)) {
		vd_printf("vfs_lookup:name is too long\n");
		err = -ENAMETOOLONG;
		goto error_out;
	}
	
	*found = lookup_at(dir_path, kpathname, flags);
	
	if (!*found) {
		if(UNLIKELY(is_lookup_test(flags))) {
			vd_printf("vfs_lookup:cannot found %s\n", pathname);
		}
		err = -ENOENT;
		goto error_out;
	}
	
	if (UNLIKELY(is_lookup_create(flags))) {
		if (!(*found)->dentry->d_vnode) {
			put_pathname(kpathname);
			return(0);
		}
		//vd_printf("vfs_lookup:LOOKUP_CREATE failed:%s\n", pathname);
		put_file_name(*found);
		err = -EEXIST;
		goto error_out;
	}
	
	put_pathname(kpathname);
	
	return(0);
	
error_out:
	*found = NULL;
	put_pathname(kpathname);
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_lookup
 Input		:const char *pathname
 		 < path name to look up >
 		 struct file_name **found
 		 < looked up dentry result >
 		 unsigned int flags
 		 < lookup flags >
 Output		:struct file_name **found
 		 < looked up dentry result >
 Return		:int
 		 < result >
 Description	:lookup dentry
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
vfs_lookup(const char *pathname, struct file_name **found, unsigned int flags)
{
	return(vfs_lookup_at(NULL, pathname, found, flags));
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:follow_link
 Input		:const char *symname
 		 < content of a symbolic link >
 		 struct file_name **found
 		 < looked up dentry result >
 		 unsigned int flags
 		 < lookup flags >
 Output		:struct file_name **found
 		 < looked up dentry result >
 Return		:int
 		 < result >
 Description	:follow a link
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
follow_link(const char *symname, struct file_name **found, unsigned int flags)
{
	struct qstr *kpathname;
	int err = 0;
	
	kpathname = dup_pathname(symname);
	
	if (UNLIKELY(!kpathname)) {
		vd_printf("failed dup pathname at %s\n", __func__);
		return(-EFAULT);
	}
	
	if (UNLIKELY(kpathname == PTR_ERR_NAME_TOO_LONG)) {
		vd_printf("name is too long at %s\n", __func__);
		return(-ENAMETOOLONG);
	}
	
	*found = lookup_at(NULL, kpathname, flags & ~LOOKUP_CREATE);
	
	if (UNLIKELY(!*found)) {
		err = -ENOENT;
		goto out;
	}
	
	if (!(*found)->dentry->d_vnode) {
		err = -ENOENT;
	}
	
out:
	put_pathname(kpathname);
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_mknod
 Input		:struct vnode *dir
 		 < a directory in which a special file is made >
 		 struct dentry *dentry
 		 < directory entry for a special file >
 		 umode_t mode
 		 < file open mode >
 		 dev_t dev
 		 < device number >
 Output		:void
 Return		:int
 		 < result >
 Description	:vfs make a special file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
vfs_mknod(struct vnode *dir, struct dentry *dentry, umode_t mode, dev_t dev)
{
	struct vnode *vnode;
	
	if (dir->v_op && dir->v_op->mknod) {
		return(dir->v_op->mknod(dir, dentry, mode, dev));
	}
	
	if (UNLIKELY(S_ISREG(mode) || S_ISDIR(mode))) {
		return(-ENXIO);
	}
	
	vnode = vfs_alloc_vnode(dir->v_sb);
	
	if (UNLIKELY(!vnode)) {
		return(-ENOMEM);
	}
	
	init_special_vnode(vnode, mode, dev);
	
	setup_vnode(dir, dentry, vnode);
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_create
 Input		:struct vnode *dir
 		 < a directory in which a file is made >
 		 struct dentry *dentry
 		 < directory entry for a file >
 		 umode_t mode
 		 < file open mode >
 		 bool excl
 		 < boolean : exclude create a file from other threads >
 Output		:void
 Return		:int
 		 < result >
 Description	:vfs create a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
vfs_create(struct vnode *dir, struct dentry *dentry, umode_t mode, bool excl)
{
	struct vnode *vnode;
	int err;
	
	if (dir->v_op && dir->v_op->create) {
		return(dir->v_op->create(dir, dentry, mode, excl));
	}
	
	if (UNLIKELY(!S_ISREG(mode))) {
		return(-ENXIO);
	}
	
	vnode = vfs_alloc_vnode(dir->v_sb);
	
	if (UNLIKELY(!vnode)) {
		return(-ENOMEM);
	}
	
	vnode->v_mode = S_IFREG | mode;
	vnode->v_fops = &ramfs_file_ope;
	
	err = setup_vnode(dir, dentry, vnode);
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_mkdir
 Input		:struct vnode *dir
 		 < a directory in which a new directory is made >
 		 struct dentry *dentry
 		 < directory entry for a new directory >
 		 umode_t mode
 		 < directory creation mode >
 Output		:void
 Return		:int
 		 < result >
 Description	:create a new directory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int vfs_mkdir(struct vnode *dir, struct dentry *dentry, umode_t mode)
{
	struct vnode *vnode;
	
	if (dir->v_op && dir->v_op->mkdir) {
		return(dir->v_op->mkdir(dir, dentry, mode));
	}
	
	vnode = vfs_alloc_vnode(dir->v_sb);
	
	if (UNLIKELY(!vnode)) {
		return(-ENOMEM);
	}
	
#if 0
	vd_printf("vfs_mkdir:");
	
	if (dentry->d_name.name) {
		vd_printf("dentry name:%s\n", dentry->d_name.name);
	} else {
		vd_printf("dentry name:%s\n", dentry->d_iname);
	}
#endif
	setup_vnode(dir, dentry, vnode);
	
	vnode->v_mode = S_IFDIR | mode;
	vnode->v_size = dir->v_size;
	
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_symlink
 Input		:struct vnode *dir
 		 < directory inode of a symbolic link >
 		 struct dentry *dentry
 		 < symbolic link dentry >
 		 const char *symname
 		 < target name >
 Output		:void
 Return		:int
 		 < result >
 Description	:create a symbolic link
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int
vfs_symlink(struct vnode *dir, struct dentry *dentry, const char *symname)
{
	struct vnode *vnode;
	int err = 0;

	if (dir->v_op && dir->v_op->symlink) {
		err = dir->v_op->symlink(dir, dentry, symname);
		goto out;
	}
	
	vnode = vfs_alloc_vnode(dir->v_sb);
	
	if (UNLIKELY(!vnode)) {
		err = -ENOMEM;
		goto out;
	}
	
	setup_vnode(dir, dentry, vnode);
	
	vnode->v_mode = (dir->v_mode & ~S_IFMT) | S_IFLNK;
	
	vnode->v_link = (char*)symname;
	
	return(0);

out:
	kfree((void*)symname);
	
	return(err);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_stat64
 Input		:struct vnode *vnode
 		 < vnode to get its file status >
 		 struct stat64_i386 *buf
 		 < file status buffer to outpu >
 Output		:struct stat64_i386 *buf
 		 < file status buffer to outpu >
 Return		:int
 		 < result >
 Description	:get file status
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int vfs_stat64(struct vnode *vnode, struct stat64_i386 *buf)
{
	struct super_block *sb = vnode->v_sb;
	
	buf->st_dev		= sb->s_dev;
	buf->st_ino		= (ino_t)vnode->v_ino;
	buf->_st_ino		= (ino_t)vnode->v_ino;
	buf->st_mode		= vnode->v_mode;
	buf->st_nlink		= vnode->v_nlink;
	buf->st_uid		= vnode->v_uid;
	buf->st_gid		= vnode->v_gid;
	buf->st_rdev		= vnode->v_rdev;
	buf->st_size		= (off64_t)vnode->v_size;
	buf->st_atime		= vnode->v_atime.tv_sec;
	buf->st_atime_nsec	= 0;
	buf->st_mtime		= vnode->v_mtime.tv_sec;
	buf->st_mtime_nsec	= 0;
	buf->st_ctime		= vnode->v_ctime.tv_sec;
	buf->st_ctime_nsec	= 0;
	buf->st_blocks		= DIV_ROUNDUP(vnode->v_size, S_BLKSIZE);
	buf->st_blksize		= buf->st_blocks * S_BLKSIZE;

#if 0
	printf("st_dev:%d ", buf->st_dev);
	printf("st_ino:%d ", buf->st_ino);

	printf("st_mode:0x%08X ", buf->st_mode);
	printf("st_nlink:%d\n", buf->st_nlink);
	printf("st_uid:%d ", buf->st_uid);
	printf("st_gid:%d ", buf->st_gid);
	printf("st_rdev:%d\n", buf->st_rdev);
	printf("st_size:%d ", buf->st_size);
	printf("st_atime:%d ", buf->st_atime);
	printf("st_mtime:%d\n", buf->st_mtime);
	printf("st_ctime:%d ", buf->st_ctime);
	printf("st_blksize:%d ", buf->st_blksize);
	printf("st_blocks:%d\n", buf->st_blocks);

#endif
	return(0);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vfs_access
 Input		:struct vnode *vnode
 		 < a vnode for checking permissions >
 		 int mode
 		 < check permission >
 Output		:void
 Return		:int
 		 < result >
 Description	:check real user's permissions for a file
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT int vfs_access(struct vnode *vnode, int mode)
{
	struct process *proc = get_current();
	int vmode;
	int err = 0;
	
	if (!proc->uid) {
		return(err);
	}
	
	if (!mode) {	// F_OK
		return(err);
	}
	
	if (proc->uid == vnode->v_uid) {
		vmode = (vnode->v_mode & S_IRWXU) >> S_IUSR_SHIFT;
	} else if (proc->gid == vnode->v_gid) {
		vmode = (vnode->v_mode & S_IRWXG) >> S_IGRP_SHIFT;
	} else {
		vmode = vnode->v_mode & S_IRWXO;
	}
	
	if (mode & R_OK) {
		if (!(vmode & R_OK)) {
			err = -EACCES;
			goto out;
		}
	} else if (mode & W_OK) {
		if (!(vmode & W_OK)) {
			err = -EACCES;
			goto out;
		}
	} else if (mode & X_OK) {
		if (!(vmode & X_OK)) {
			err = -EACCES;
			goto out;
		}
	}

out:
	return(err);
}


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/


/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:lookup_at
 Input		:struct path *dir_path
 		 < directory path >
 		 const struct qstr *path_name
 		 < path to lookup >
 		 unsigned int flags
 		 < lookup flags >
 Output		:void
 Return		:struct file_name*
 		 < looked up >
 Description	:look up an entry in a directory relative to a directory
==================================================================================
*/
LOCAL struct file_name*
lookup_at(struct path *dir_path, const struct qstr *path_name, unsigned int flags)
{
	struct path *path;
	struct file_name *fname;
	struct dentry *parent;
	struct dentry *dentry;
	
	if (UNLIKELY(!path_name->len)) {
		return(NULL);
	}
	
	if (UNLIKELY(!path_name->name)) {
		return(NULL);
	}
	
	/* -------------------------------------------------------------------- */
	/* absolute path : path name starts '/'					*/
	/* -------------------------------------------------------------------- */
	if (path_name->name[0] == '/') {
		path = vfs_get_root(get_current());
		
		if (UNLIKELY(path_name->len == 1)) {
			parent = path->dentry;
			dentry = path->dentry;
		}
		
	/* -------------------------------------------------------------------- */
	/* relative path : path name starts except for '/'			*/
	/* -------------------------------------------------------------------- */
	} else {
		if (!dir_path) {
			path = vfs_get_cwd(get_current());
		} else {
			path = dir_path;
		}
		
		if (path_name->len == 1 && path_name->name[0] == '.') {
			parent = path->dentry->d_parent;
			dentry = path->dentry;
			goto found_dots;
		} else if (path_name->len == 2 &&
				path_name->name[0] == '.' &&
				path_name->name[1] == '.') {
			if (LIKELY(path->dentry->d_parent)) {
				parent = path->dentry->d_parent;
				dentry = parent;
				goto found_dots;
			}
			
			path = vfs_get_root(get_current());
			
			parent = path->dentry;
			dentry = path->dentry;
			
			goto found_dots;
		}
	}
	
	/* -------------------------------------------------------------------- */
	/* parse a path								*/
	/* -------------------------------------------------------------------- */
	return(parse_path(path_name, path, flags));

found_dots:
	fname = alloc_file_name(path_name);
	
	if (UNLIKELY(!fname)) {
		return(NULL);
	}
	
	fname->parent = parent;
	fname->dentry = dentry;
	fname->mnt = path->mnt;
		
	
	return(fname);

}

/*
==================================================================================
 Funtion	:void
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
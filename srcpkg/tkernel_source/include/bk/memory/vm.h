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

#ifndef	__BK_VM_H__
#define	__BK_VM_H__

#include <bk/memory/slab.h>
#include <bk/memory/page.h>
#include <tstdlib/bitop.h>
#include <tstdlib/list.h>

#if STD_SH7727
#endif
#if STD_SH7751R
#endif
#if MIC_M32104
#endif
#if STD_S1C38K
#endif
#if STD_MC9328
#endif
#if MIC_VR4131
#endif
#if STD_VR5500
#endif
#if STD_MB87Q1100
#endif
#if STD_SH7760
#endif
#if TEF_EM1D
#  include <bk/memory/sysdepend/em1d/vm.h>
#endif
#if _STD_X86_
#  include <bk/memory/sysdepend/x86/vm.h>
#endif

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
struct vnode;

/*
==================================================================================

	DEFINE 

==================================================================================
*/
#define	VM_READ		MAKE_BIT32(0)	// read permission
#define	VM_WRITE	MAKE_BIT32(1)	// write permission
#define	VM_EXEC		MAKE_BIT32(3)	// execute permission
#define	VM_COW		MAKE_BIT32(4)	// copy on write

struct memory_space;

struct vm {
	struct list		link_vm;	// list node of list_vm
	unsigned long		start;		// start address of a memory
	unsigned long		end;		// end address of a memory
	size_t			length;		// length of memory map
	unsigned int		prot;		// vm protection flags
	unsigned int		mmap_flags;	// mmap flags
	struct page		**pages;	// vm pages
	unsigned int		nr_pages;	// number of pages
	struct memory_space	*mspace;
	struct vnode		*vnode;		// associated file
	loff_t			offset;		// file offset
};

struct memory_space {
	pde_t			*pde;		/* page directory		*/
	struct list		list_vm;	/* list of virtual memory	*/
	unsigned long		nr_vms;		/* number of vms		*/
	unsigned long		process_size;	/* size of process vm space	*/
	unsigned long		count;		/* reference count		*/
	unsigned long		shared_vm;	/* shrared pages		*/
	unsigned long		exec_vm;
	unsigned long		stack_vm;
	unsigned long		start_shared;
	unsigned long		end_shared;
	unsigned long		start_code;
	unsigned long		end_code;
	unsigned long		start_data, end_data;
	unsigned long		start_bss, end_bss;
	unsigned long		start_brk, end_brk;
	unsigned long		start_stack, end_stack;
	unsigned long		start_arg, end_arg;
	unsigned long		start_env, end_env;
	int			exe_fd;		/* executable file of the proc	*/
};

#define	SHARED_START		(PROCESS_SIZE / 3)
#define	MMAP_START		SHARED_START
#define	PROC_STACK_TOP		(PROCESS_SIZE)
#define	PROC_STACK_SIZE		(8192)
#define	PROC_STACK_START	(PROCESS_SIZE - PROC_STACK_SIZE)

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
 Funtion	:init_mm
 Input		:void
 Output		:void
 Return		:int
 		 < result >
 Description	:initialize virtuam memory management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int init_mm(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:destroy_mm
 Input		:void
 Output		:void
 Return		:void
 Description	:destroy virtual memory management
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void destroy_mm(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_memory_space
 Input		:void
 Output		:void
 Return		:struct memory_space*
 		 < memory space for user space >
 Description	:initialize memory space on space spawning
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct memory_space* alloc_memory_space(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_memory_space
 Input		:struct memory_space *mspace
 		 < memory space >
 Output		:void
 Return		:void
 Description	:free memory space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void free_memory_space(struct memory_space *mspace);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_vm
 Input		:void
 Output		:void
 Return		:struct vm*
 		 < vm struct >
 Description	:allocate a vm struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct vm*
//alloc_vm(unsigned long start, unsigned long end, unsigned int prot)
alloc_vm(void);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_vm_all
 Input		:struct process *proc
 		 < process which has vm areas >
 Output		:void
 Return		:void
 Description	:free virtual memory for a user process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void free_vm_all(struct process *proc);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:alloc_vm_pages
 Input		:int nr_pages
 		 < number of pages >
 Output		:void
 Return		:struct pages**
 		 < pages struct array >
 Description	:allocate a vm_pages struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct page** alloc_vm_pages(int nr_pages);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_vm
 Input		:struct process *proc
 		 < process which has vm areas >
 Output		:void
 Return		:void
 Description	:free virtual memory for a user process
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void free_vm(struct process *proc);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:free_vm_pages
 Input		:struct vm *vm
 		 < vm which has pages >
 Output		:void
 Return		:void
 Description	:free pages allocated for virtual memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void free_vm_pages(struct vm *vm);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:get_address_vm
 Input		:struct process *proc
 		 < get vm from the process >
 		 unsigned long start
 		 < user start address which belongs to vm of process >
 		 unsigned long end
 		 < user end address which belongs to vm of process >
 Output		:void
 Return		:struct vm*
 		 < vm to which address belongs >
 Description	:get vm to which address belongs
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT struct vm*
get_address_vm(struct process *proc, unsigned long start, unsigned long end);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_mm
 Input		:struct process *from
 		 < copy from >
 		 struct process *to
 		 < copy to >
 Output		:void
 Return		:int
 		 < result >
 Description	:copy memory_space struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int copy_mm(struct process *from, struct process *to);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_vm
 Input		:struct process *from
 		 < copy from >
 		 struct process *to
 		 < copy to >
 Output		:void
 Return		:int
 		 < result >
 Description	:copy vm struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int copy_vm(struct process *from, struct process *to);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:copy_vm_pages
 Input		:struct vm *vm_from
 		 < copy from >
 		 struct vm *vm_to
 		 < copy to >
 Output		:void
 Return		:int
 		 < result >
 Description	:copy vm_pages struct
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int copy_vm_pages(struct vm *vm_from, struct vm *vm_to);


/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:map_vm
 Input		:struct process *proc
 		 < a process to create its vm >
 		 unsigned long mmap_start
 		 < start address of user space >
 		 unsigned long mmap_end
 		 < end address of user space >
 		 size_t lenght
 		 < length of a memory map >
 		 unsigned int prot
 		 < permission >
 		 unsigned int mmap_flags
 		 < mmap flags >
 		 int fd
 		 < file descriptor to map >
 		 off_t offset
 		 < offset in a file >
 Output		:void
 Return		:int
 		 < result >
 Description	:map a virtual memory for user space
 		 this function is currently used for test only
 		 future work;
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int map_vm(struct process *proc,
			unsigned long mmap_start, unsigned long mmap_end,
			size_t length, unsigned int prot,
			unsigned int mmap_flags, int fd, loff_t offset);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:unmap_vm
 Input		:struct process *proc
 		 < a process to unmap its vm >
 		 unsigned long mmap_start
 		 < start address to unmap >
 		 unsigned long mmap_end
 		 < end address to unmap >
 Output		:void
 Return		:int
 		 < result >
 Description	:unmap vm
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int unmap_vm(struct process *proc,
			unsigned long mmap_start, unsigned long mmap_end);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:search_mmap_candidate
 Input		:struct process *proc
 		 < process to search its vm >
 		 size_t length
 		 < size of memory to request a mapping >
 		 int prot
 		 < page protection >
 		 int flags
 		 < mmap flags >
 		 int fd
 		 < file descriptor >
 		 off_t offset
 		 < offset in a file >
 Output		:void
 Return		:void*
 		 < candidate memory address >
 Description	:seach memory area as a candidate for mmap
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void* search_mmap_candidate(struct process *proc, size_t length,
					int prot, int flags, int fd, off_t offset);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:switch_ms
 Input		:struct process *from
 		 < switch from >
 		 struct process *to
 		 < switch to >
 Output		:void
 Return		:void
 Description	:switch memory space
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void switch_ms(struct process *from, struct process *to);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vm_extend_brk
 Input		:struct process *proc
 		 < process to extend its brk >
 		 unsigned long new_brk
 		 < end address of new break. must be aligned to page size >
 Output		:void
 Return		:void*
 		 < new break >
 Description	:extend break
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void* vm_extend_brk(struct process *proc, unsigned long new_brk);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vm_extend_stack
 Input		:struct process *proc
 		 < process to extend its stack area >
 		 unsigned long new_extend
 		 < new extension size >
 Output		:void
 Return		:int
 		 < result >
 Description	:extend stack
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int vm_extend_stack(struct process *proc, unsigned long new_extend);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vm_initial_stack
 Input		:struct process *proc
 		 < process to map its initial stack >
 		 int nr_pages
 		 < number of pages for stack >
 		 struct page **pages
 		 < pages of initial stack >
 Output		:void
 Return		:int
 		 < result >
 Description	:map initial stack page to vm
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int
vm_initial_stack(struct process *proc, int nr_pages, struct page **pages);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:activate_vm_page
 Input		:struct process *proc
 		 < process to activate is page >
 		 struct vm *vm
 		 < virtual memory to activate its page >
 		 unsigned int la
 		 < virtual address which vm includes >
 		 unsigned int error_code
 		 < error code of a page fault >
 Output		:void
 Return		:int
 		 < result >
 Description	:activate a paged whic is already mapped to vm
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int activate_vm_page(struct process *proc, struct vm *vm,
				unsigned long la, unsigned int error_code);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vm_check_access
 Input		:void *user_addr
 		 < user space address >
 		 size_t size
 		 < memory size to check >
 		 int prot
 		 < memory protection >
 Output		:void
 Return		:int
 		 < result >
 Description	:check user memory permission
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int vm_check_access(void *user_addr, size_t size, int prot);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:vm_mprotect
 Input		:void *addr
 		 < start address of a memory region to set memory protectoin >
 		 size_t len
 		 < size of a memory region >
 		 int prot
 		 < protection >
 Output		:void
 Return		:int
 		 < result >
 Description	:set protection on a region of memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int vm_mprotect(void *addr, size_t len, int prot);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:show_vm_list
 Input		:struct process *proc
 		 < show its vms >
 Output		:void
 Return		:void
 Description	:show vm list
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT void show_vm_list(struct process *proc);

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:is_vm_cow
 Input		:unsigned long addr
 		 < address to check whether its page is cow >
 Output		:void
 Return		:int
 		 < result >
 Description	:check whether address is in cow page
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
IMPORT int is_vm_cow(unsigned long addr);

#endif	// __BK_VM_H__

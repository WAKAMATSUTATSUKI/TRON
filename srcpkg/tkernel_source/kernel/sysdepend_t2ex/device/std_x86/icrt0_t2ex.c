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

#include <string.h>
#include <tk/typedef.h>
#include <tstdlib/bitop.h>
#include <sys/sysdepend/std_x86/multiboot.h>
#include <sys/sysdepend/std_x86/multiboot_def.h>
#include <bk/memory/page.h>

#include <debug/vdebug.h>

#include <cpu/x86/cpuid.h>

/*
==================================================================================

	PROTOTYPE

==================================================================================
*/
// kernel/sysdepend/device/std_x86/devinit.c
IMPORT ER before_startup( void );
// kernel/sysinit/src/sysinit_main.c
IMPORT int main( void );
// kernel_t2ex_img.lnk
IMPORT unsigned long end_bss;

LOCAL void handle_multiboot1(struct multiboot_info *info);
LOCAL void handle_multiboot2(void *info);

/*
==================================================================================

	Kernel Config 

==================================================================================
*/

/*
==================================================================================

	Kernel Management 

==================================================================================
*/
//EXPORT uint32_t lowmem_top;
//EXPORT int32_t lowmem_limit;
EXPORT uint32_t monitor_stacktop;

LOCAL struct boot_info boot_info;

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:_kernel_entry
 Input		:void* info
 Output		:void
 Return		:void
 Description	:Kernel Core
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void _kernel_entry( uint32_t magic, void *info )
{
	int err;
	
	initVdebug(80,25);
	
	/* -------------------------------------------------------------------- */
	/* handle for multiboot header						*/
	/* -------------------------------------------------------------------- */
	switch( magic ) {
	case	MULTIBOOT_BOOTLOADER_MAGIC:
		boot_info.version = MULTIBOOT_BOOTLOADER_MAGIC;
		vd_printf("multiboot1\n");
		handle_multiboot1(info);
		break;
	case	MULTIBOOT2_BOOTLOADER_MAGIC:
		boot_info.version = MULTIBOOT2_BOOTLOADER_MAGIC;
		vd_printf("multiboot2\n");
		handle_multiboot2((void*)info);
		break;
	default:
		vd_printf("Invalid magic number: 0x%X\n", magic);
		return;
	}
	
	/* -------------------------------------------------------------------- */
	/* kernel startup							*/
	/* -------------------------------------------------------------------- */
	before_startup();

	if ((err = main())) {
		vd_printf("cannot startup the kernel\n");
	}

}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:getBootInfo
 Input		:void
 Output		:void
 Return		:struct boot_info*
 		 < boot information >
 Description	:get boot information
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT __inline__ struct boot_info* getBootInfo( void )
{
	return(&boot_info);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:allocLowMemory
 Input		:uint32_t size
		 < size to allocate >
 Output		:void
 Return		:void*
		 < allocated memory address >
 Description	:allocate a memory area from lowmem
		 this method only use for boot sequence after analyzing memory map
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void* allocLowMemory(uint32_t size)
{
	unsigned long allocated;
	unsigned long mask = sizeof(unsigned long) - 1;
	
	if (!size) {
		return(NULL);
	}
	
	if (boot_info.lowmem_top & ~mask) {
		boot_info.lowmem_top = (boot_info.lowmem_top + mask) & ~mask;
	}
	
	allocated = boot_info.lowmem_top | KERNEL_BASE_ADDR;
	boot_info.lowmem_top += size;
	
	return((void*)allocated);
}

/*
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
 Funtion	:align_low_memory
 Input		:unsigned long align
 		 < alignment unit >
 Output		:void
 Return		:void
 Description	:alignment low memory
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
*/
EXPORT void align_low_memory(unsigned long align)
{
	if (boot_info.lowmem_top & (align - 1)) {
		unsigned long comp = align - (boot_info.lowmem_top & (align - 1));
		
		boot_info.lowmem_top += comp;
	}
}

/*
++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
	
	< Local Functions >

++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
*/
/*
==================================================================================
 Funtion	:handle_multiboot1
 Input		:struct multiboot_info *info
		 < multiboot information >
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
LOCAL void handle_multiboot1(struct multiboot_info *info)
{
	vd_printf("flags = 0x%X\n", info->flags);

	/* -------------------------------------------------------------------- */
	/* lower/upper memory info bit						*/
	/* -------------------------------------------------------------------- */
	if (info->flags & MULTIBOOT_INFO_MEMORY) {
		vd_printf("mem_lower = 0x%08X, mem_upper = 0x%08X\n",
			info->mem_lower, info->mem_upper);
		boot_info.mem_lower = info->mem_lower;
		boot_info.mem_upper = info->mem_upper;
	} else {
		boot_info.mem_lower = 0;
		boot_info.mem_upper = 0;
	}
	/* -------------------------------------------------------------------- */
	/* count memory map							*/
	/* -------------------------------------------------------------------- */
	if (info->flags & MULTIBOOT_INFO_MEM_MAP) {
		struct multiboot_entry *mmap;
		boot_info.num_mmap_entries = 0;
		
		for (mmap = (struct multiboot_entry*)info->mmap_addr;
			(unsigned long)mmap <
			((unsigned long)info->mmap_addr + info->mmap_length) ;
			mmap = (struct multiboot_entry*)((unsigned long)mmap
			+ mmap->size + sizeof(mmap->size))) {
			boot_info.num_mmap_entries++;
		}
	}
	/* -------------------------------------------------------------------- */
	/* full memory map?							*/
	/* -------------------------------------------------------------------- */
	boot_info.lowmem_top = (unsigned long)&end_bss - KERNEL_BASE_ADDR;
	if (info->flags & MULTIBOOT_INFO_MEM_MAP) {
		struct multiboot_entry *mmap;
		unsigned long len = 0;
		int mmap_len;
		int i = 0;
		
		vd_printf("mmap_addr = 0x%08X, mmap_length = 0x%X\n",
			info->mmap_addr, info->mmap_length );
		
		//boot_info.mmap = (struct multiboot_entry*)
		//	(info->mmap_addr | KERNEL_BASE_ADDR);
		boot_info.mmap2 = allocLowMemory(sizeof(struct multiboot_mmap_entry) *
						boot_info.num_mmap_entries);
		boot_info.lowmem_limit = 0;
		
		mmap_len = info->mmap_length / (0x14 + sizeof(mmap->size));
		vd_printf("mmap_len = %d\n", mmap_len);
		
		for (mmap = (struct multiboot_entry*)info->mmap_addr;
			(unsigned long)mmap <
			((unsigned long)info->mmap_addr + info->mmap_length) ;
			mmap = (struct multiboot_entry*)((unsigned long)mmap
			+ mmap->size + sizeof(mmap->size))) {
			vd_printf("size = 0x%X, base_addr = 0x%08X%08X,"
				" length = 0x%08X%08X, type = 0x%X\n",
				mmap->size,
				(uint32_t)(mmap->addr >> 32),
				(uint32_t)(mmap->addr & 0xFFFFFFFF),
				(uint32_t)(mmap->len >> 32),
				(uint32_t)(mmap->len & 0xFFFFFFFF),
				mmap->type );
			boot_info.mmap2[i].addr = mmap->addr;
			boot_info.mmap2[i].len = mmap->len;
			boot_info.mmap2[i].type = mmap->type;
			if (mmap->type == MULTIBOOT_MEMORY_AVAILABLE
				|| mmap->type == MULTIBOOT_MEMORY_ACPI_RECLAIMABLE) {
				unsigned long mem_addr;
				unsigned long limit;

				mem_addr = (unsigned long)mmap->addr;
				limit = mem_addr + (unsigned long)mmap->len;

				if (boot_info.lowmem_limit < limit) {
					if (mmap->type == 1 && !(len < mmap->len)) {
						continue;
					}
					len = mmap->len;
					boot_info.lowmem_limit = limit;
					boot_info.lowmem_base = mem_addr;
				}
			}
			i++;
		}
	} else {
		vd_printf("Unknown memory area. Cannot boot the kernel\n");
		boot_info.num_mmap_entries = 0;
		boot_info.mmap = NULL;
	}
	vd_printf("---------------------------------------------\n");
	vd_printf("lowmem_top:0x%08X\n", boot_info.lowmem_top);
	vd_printf("lowmem_limit:0x%08X\n", boot_info.lowmem_limit);
	/* -------------------------------------------------------------------- */
	/* is there a boot device set?						*/
	/* -------------------------------------------------------------------- */
	if (info->flags & MULTIBOOT_INFO_BOOTDEV) {
		vd_printf("boot_device = 0x%X\n", info->boot_device);
		boot_info.boot_device = info->boot_device;
	} else {
		boot_info.boot_device = 0;
	}
	/* -------------------------------------------------------------------- */
	/* command-line defined?						*/
	/* -------------------------------------------------------------------- */
	if (info->flags & MULTIBOOT_INFO_CMDLINE) {
		int len;
		vd_printf("cmdline = %s\n", info->cmdline);
		len = strlen((const char*)info->cmdline);
		boot_info.cmdline = (char*)allocLowMemory(len + 1);
		if (boot_info.cmdline) {
			strncpy((char*)info->cmdline,
						(char*)boot_info.cmdline, len);
		}
	} else {
		boot_info.cmdline = NULL;
	}
	/* -------------------------------------------------------------------- */
	/* a symbol table loaded?						*/
	/* -------------------------------------------------------------------- */
	if (info->flags & MULTIBOOT_INFO_AOUT_SYMS ) {
		vd_printf("only elf shdr is suported\n");
	}
	/* -------------------------------------------------------------------- */
	/* modules to do something with?					*/
	/* -------------------------------------------------------------------- */
	if (info->mods_count && (info->flags & MULTIBOOT_INFO_MODS)) {
		int i;
		boot_info.mods_count = info->mods_count;
		boot_info.mod_list = (struct multiboot_mod_list*)
			allocLowMemory(sizeof(struct multiboot_mod_list)
				* info->mods_count);
		
		if (boot_info.mod_list) {
			struct multiboot_mod_list *blist =
				(struct multiboot_mod_list *)info->mods_addr;
			struct multiboot_mod_list *list = boot_info.mod_list;
			for (i = 0;i < boot_info.mods_count;i++) {
				list[i].mod_start = blist[i].mod_start | KERNEL_BASE_ADDR;
				list[i].mod_end = blist[i].mod_end | KERNEL_BASE_ADDR;
				/* as for now, cmdline memory is not allocated	*/
				list[i].cmdline = blist[i].cmdline | KERNEL_BASE_ADDR;
				vd_printf("mod_list[%d] start:0x%08X\n", i,
						list[i].mod_start);
				vd_printf("mod_list[%d] end:0x%08X\n", i,
						list[i].mod_end);
			}
		}
		vd_printf("mods_count = %u\n", boot_info.mods_count);
		vd_printf("mods_addr = 0x%08X\n", boot_info.mod_list);
		vd_printf("mods_addr.start = 0x%08X\n", boot_info.mod_list[MULTIBOOT_VDSO].mod_start);
		vd_printf("mods_addr.start = 0x%08X\n", boot_info.mod_list[MULTIBOOT_INITRAMFS].mod_start);
	} else {
		boot_info.mods_count = 0;
		boot_info.mod_list = NULL;
	}
	/* -------------------------------------------------------------------- */
	/* ELF section header table?						*/
	/* -------------------------------------------------------------------- */
	if (info->flags & MULTIBOOT_INFO_ELF_SHDR ) {
		struct multiboot_elf_section_header_table *elf_sec;
		elf_sec = &info->u.elf_sec;
		
		vd_printf("multiboot_elf_sec: num = %u, size = 0x%X,"
			" addr = 0x%08X, shndx = 0x%X\n",
			elf_sec->num, elf_sec->size,
			elf_sec->addr, elf_sec->shndx);
		boot_info.u.elf_sec =
		(struct multiboot_elf_section_header_table *)
		((unsigned long)&info->u.elf_sec | KERNEL_BASE_ADDR);
	} else {
		boot_info.u.elf_sec = NULL;
	}
	/* -------------------------------------------------------------------- */
	/* drive inofo?								*/
	/* -------------------------------------------------------------------- */
	if (info->flags & MULTIBOOT_INFO_DRIVE_INFO) {
		vd_printf("drive_length:%u\n", info->drives_length);
		vd_printf("drives_addr:0x%08X\n", info->drives_addr);
		boot_info.drives_length = info->drives_length;
		boot_info.drives = (struct multiboot_drives*)
			((unsigned long)info->drives_addr | KERNEL_BASE_ADDR);
	} else {
		boot_info.drives_length = 0;
		boot_info.drives = NULL;
	}
	/* -------------------------------------------------------------------- */
	/* is there a config table?						*/
	/* -------------------------------------------------------------------- */
	if (info->flags & MULTIBOOT_INFO_CONFIG_TABLE) {
		vd_printf("config_table : 0x%08X\n", info->config_table);
		boot_info.config_table = (struct multiboot_config_table*)
			((unsigned long)info->config_table | KERNEL_BASE_ADDR);
	} else {
		boot_info.config_table = NULL;
	}
	/* -------------------------------------------------------------------- */
	/* is there a boot loader name?						*/
	/* -------------------------------------------------------------------- */
	if (info->flags & MULTIBOOT_INFO_BOOT_LOADER_NAME) {
		vd_printf("loader name:%s\n", info->boot_loader_name);
		boot_info.boot_loader_name = (char*)info->boot_loader_name;
	} else {
		boot_info.boot_loader_name = NULL;
	}
	/* -------------------------------------------------------------------- */
	/* an APM table?							*/
	/* -------------------------------------------------------------------- */
	if (info->flags & MULTIBOOT_INFO_APM_TABLE) {
		vd_printf("apm talbe:0x%08X\n", info->apm_table);
		boot_info.apm_table =
		(void*)((unsigned long)info->apm_table | KERNEL_BASE_ADDR);
	} else {
		boot_info.apm_table = NULL;
	}
	/* -------------------------------------------------------------------- */
	/* video information?							*/
	/* -------------------------------------------------------------------- */
	if (info->flags & MULTIBOOT_INFO_VIDEO_INFO) {
		void *vbe_addr = (void*)&info->vbe_control_info;
		
		vd_printf("vbe_control_info:0x%08X", info->vbe_control_info);
		vd_printf("vbe_mode_info:0x%08X", info->vbe_mode_info);
		vd_printf("vbe_mode:0x%08X", info->vbe_mode);
		vd_printf("vbe_interface_seg:0x%08X", info->vbe_interface_seg);
		vd_printf("vbe_interface_off:0x%08X", info->vbe_interface_off);
		vd_printf("vbe_interface_len:0x%08X", info->vbe_interface_len);
		boot_info.vbe_info = (struct multiboot_vbe_info*)
		((unsigned long)vbe_addr | KERNEL_BASE_ADDR);
		vd_printf("\n");
	} else {
		boot_info.vbe_info = NULL;
	}
}

/*
==================================================================================
 Funtion	:handle_multiboot2
 Input		:void
 Output		:void
 Return		:void
 Description	:void
==================================================================================
*/
LOCAL void handle_multiboot2(void *info)
{
	vd_printf("multiboot2 has not been yet implemented\n");
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

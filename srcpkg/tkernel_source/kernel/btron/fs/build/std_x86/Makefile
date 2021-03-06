#
# ----------------------------------------------------------------------
#    T2EX Software Package
#
#    Copyright 2012 by Ken Sakamura.
#    This software is distributed under the latest version of T-License 2.x.
# ----------------------------------------------------------------------
#
#    Released by T-Engine Forum(http://www.t-engine.org/) at 2012/12/12.
#    Modified by Nina Petipa at 2015/10/28
#
# ----------------------------------------------------------------------
#

#
# This software package is available for use, modification, 
# and redistribution in accordance with the terms of the attached 
# T-License 2.x.
# If you want to redistribute the source code, you need to attach 
# the T-License 2.x document.
# There's no obligation to publish the content, and no obligation 
# to disclose it to the TRON Forum if you have modified the 
# software package.
# You can also distribute the modified source code. In this case, 
# please register the modification to T-Kernel traceability service.
# People can know the history of modifications by the service, 
# and can be sure that the version you have inherited some 
# modification of a particular version or not.
#
#    http://trace.tron.org/tk/?lang=en
#    http://trace.tron.org/tk/?lang=ja
#
# As per the provisions of the T-License 2.x, TRON Forum ensures that 
# the portion of the software that is copyrighted by Ken Sakamura or 
# the TRON Forum does not infringe the copyrights of a third party.
# However, it does not make any warranty other than this.
# DISCLAIMER: TRON Forum and Ken Sakamura shall not be held
# responsible for any consequences or damages caused directly or
# indirectly by the use of this software package.
#
# The source codes in bsd_source.tar.gz in this software package are 
# derived from NetBSD or OpenBSD and not covered under T-License 2.x.
# They need to be changed or redistributed according to the 
# representation of each source header.
#

#
#	Makefile for gmake
#	T-Kernel/OS (system-dependent description)
#

# T-Engine type
MACHINE = x86
TETYPE	= std

BUILD_PATH = build/$(TETYPE)_$(MACHINE)

FS_TARGET = fs.o

OBJ = ../../vfs/$(BUILD_PATH)/vfs.o
OBJ += ../../load/$(BUILD_PATH)/load.o
OBJ += ../../fimp/$(BUILD_PATH)/fimp.o
OBJ += ../../ramfs/$(BUILD_PATH)/ramfs.o
OBJ += ../../sysdepend/$(BUILD_PATH)/vdso_kernel.o

# ----------------------------------------------------------------------------

.PHONY: all clean install source clean_source clean_install ready_for_rebuild

all:$(FS_TARGET)

$(FS_TARGET): $(OBJ)
	$(CC) $(TARGET_ARCH) -r -nostdlib $^ -o $@

$(OBJ):
	( cd ../../fs_if/$(BUILD_PATH) ; $(MAKE) ; $(MAKE) install )
	( cd ../../vfs/$(BUILD_PATH) ; $(MAKE) ; $(MAKE) install )
	( cd ../../fimp/$(BUILD_PATH) ; $(MAKE) ; $(MAKE) install )
	( cd ../../load/$(BUILD_PATH) ; $(MAKE) ; $(MAKE) install )
	( cd ../../ramfs/$(BUILD_PATH) ; $(MAKE) ; $(MAKE) install )
	( cd ../../sysdepend/$(BUILD_PATH) ; $(MAKE) )

clean:
	( cd ../../fs_if/$(BUILD_PATH) ; $(MAKE) clean )
	( cd ../../vfs/$(BUILD_PATH) ; $(MAKE) clean )
	( cd ../../fimp/$(BUILD_PATH) ; $(MAKE) clean )
	( cd ../../load/$(BUILD_PATH) ; $(MAKE) clean)
	( cd ../../ramfs/$(BUILD_PATH) ; $(MAKE) clean)
	( cd ../../sysdepend/$(BUILD_PATH) ; $(MAKE) clean )
	$(RM) $(FS_TARGET)

install:
	( cd ../../fs_if/$(BUILD_PATH) ; $(MAKE) install )
	( cd ../../vfs/$(BUILD_PATH) ; $(MAKE) install )
	( cd ../../fimp/$(BUILD_PATH) ; $(MAKE) install )
	( cd ../../load/$(BUILD_PATH) ; $(MAKE) install )

clean_install:
	( cd ../../fs_if/$(BUILD_PATH) ; $(MAKE) clean_install )
	( cd ../../vfs/$(BUILD_PATH) ; $(MAKE) clean_install )
	( cd ../../fimp/$(BUILD_PATH) ; $(MAKE) clean_install )
	( cd ../../load/$(BUILD_PATH) ; $(MAKE) clean_install )

source:
	( cd ../../fs_if/$(BUILD_PATH) ; $(MAKE) source )

ready_for_rebuild:
	( cd ../../fs_if/$(BUILD_PATH) ; $(MAKE) ready_for_rebuild )
	( cd ../../vfs/$(BUILD_PATH) ; $(MAKE) ready_for_rebuild )
	( cd ../../fimp/$(BUILD_PATH) ; $(MAKE) ready_for_rebuild )
	( cd ../../load/$(BUILD_PATH) ; $(MAKE) ready_for_rebuild)
	( cd ../../ramfs/$(BUILD_PATH) ; $(MAKE) ready_for_rebuild)
	( cd ../../sysdepend/$(BUILD_PATH) ; $(MAKE) ready_for_rebuild )
	$(RM) $(FS_TARGET)

clean_source:
	( cd ../../fs_if/$(BUILD_PATH) ; $(MAKE) clean_source )

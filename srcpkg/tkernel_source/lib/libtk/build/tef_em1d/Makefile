#
# ----------------------------------------------------------------------
#     T-Kernel 2.0 Software Package
#
#     Copyright 2011 by Ken Sakamura.
#     This software is distributed under the T-License 2.0.
# ----------------------------------------------------------------------
#
#     Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
#     Modified by T-Engine Forum at 2012/11/07.
#
# ----------------------------------------------------------------------
#

#
#	Makefile for gmake
#	libtk (system-dependent description)
#

# T-Engine type
MACHINE = em1d
TETYPE = tef

# default rules
include ../../../../etc/makerules

# source files (system-dependent)
SRC =	startup_elf.c \
	setspc.c waitusec.c waitnsec.c disint.S int.c ptimer.c

# compiler options
CFLAGS += -fomit-frame-pointer

# include main makefile (common description)
include ../../src/Makefile.common

/*
 *----------------------------------------------------------------------
 *    T-Kernel 2.0 Software Package
 *
 *    Copyright 2011 by Ken Sakamura.
 *    This software is distributed under the T-License 2.0.
 *----------------------------------------------------------------------
 *
 *    Released by T-Engine Forum(http://www.t-engine.org/) at 2011/05/17.
 *    Modified by Nina Petipa at 2015/11/03
 *
 *----------------------------------------------------------------------
 */

/*
 *	@(#)libinit.c (libtk)
 *
 *	libker library initialization
 *
 *	_InitLibtk() is always linked as it is called from the
 *	startup part.
 *	Note that adding too many processing can make the program
 *	quite large.
 */

#include <tk/libtk.h>
#include <sys/util.h>
#include <sys/memalloc.h>
#include <tk/util.h>

EXPORT MACB	_Kmacb;		/* Kmalloc management information */
EXPORT MACB	_Vmacb;		/* Vmalloc management information */
EXPORT MACB	_Smacb;		/* Smalloc management information */

LOCAL	BOOL	libtk_init_done = FALSE;

/*
 * Library initialization
 */
EXPORT void _InitLibtk( void )
{
	INT	rng;

	if ( libtk_init_done ) {
		return;  /* Initialized */
	}

	/* Kernel utility initialization */
	KnlInit();

	/* Lowest protection level where system calls can be issued */
	if ( tk_get_cfn("TSVCLimit", &rng, 1) < 1 ) {
		rng = 2;
	}
	rng <<= 8;

	/* Create exclusive control lock for library sharing */
	_init_liblock();

#ifndef _BTRON_
	/* malloc initialization */
	_tkm_init((UINT)rng, &_Kmacb);			/* Kmalloc init */
	_tkm_init((UINT)rng|TA_NORESIDENT, &_Vmacb);	/* Vmalloc init */
	_tkm_init(TA_RNG3|TA_NORESIDENT, &_Smacb);	/* Smalloc init */
#endif

	libtk_init_done = TRUE;  /* Initialization complete */
}

/*
 * Library finalization
 */
EXPORT void _FinishLibtk( void )
{
	if ( !libtk_init_done ) {
		return;
	}

	_delete_liblock();
}

//=============================================================================
//
//      hal_syscall.c
//
//      
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002, 2003 Free Software Foundation, Inc.
//
// eCos is free software; you can redistribute it and/or modify it under    
// the terms of the GNU General Public License as published by the Free     
// Software Foundation; either version 2 or (at your option) any later      
// version.                                                                 
//
// eCos is distributed in the hope that it will be useful, but WITHOUT      
// ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or    
// FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License    
// for more details.                                                        
//
// You should have received a copy of the GNU General Public License        
// along with eCos; if not, write to the Free Software Foundation, Inc.,    
// 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.            
//
// As a special exception, if other files instantiate templates or use      
// macros or inline functions from this file, or you compile this file      
// and link it with other works to produce a work based on this file,       
// this file does not by itself cause the resulting work to be covered by   
// the GNU General Public License. However the source code for this file    
// must still be made available in accordance with section (3) of the GNU   
// General Public License v2.                                               
//
// This exception does not invalidate any other reasons why a work based    
// on this file might be covered by the GNU General Public License.         
// -------------------------------------------                              
// ####ECOSGPLCOPYRIGHTEND####                                              
//=============================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):   msalter
// Contributors:msalter, gthomas
// Date:        2000-11-5
// Purpose:     
// Description: 
//              
//              
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>

#ifdef CYGPKG_REDBOOT
#include <pkgconf/redboot.h>
#endif

#if defined(CYGSEM_REDBOOT_BSP_SYSCALLS)

#include <cyg/hal/hal_stub.h>           // Our header
#include <cyg/hal/hal_arch.h>           // HAL_BREAKINST
#include <cyg/hal/hal_cache.h>          // HAL_xCACHE_x
#include <cyg/hal/hal_intr.h>           // interrupt disable/restore

#include <cyg/hal/hal_if.h>             // ROM calling interface
#include <cyg/hal/hal_misc.h>           // Helper functions

extern int __do_syscall(int func,		// syscall function number
			long arg1, long arg2,	// up to four args.
			long arg3, long arg4,
			int *retval,		// syscall return value
			int *sig);              // signal to return (or 0)

#define	SYS_exit	1
#define SYS_interrupt   1000

int
hal_syscall_handler(void)
{
    int func, arg1, arg2, arg3, arg4;
    int err, res, sig;

    func = get_register(R7);
    arg1 = get_register(R8);
    arg2 = get_register(R9);
    arg3 = get_register(R10);
    arg4 = get_register(R11);

    if (func == SYS_interrupt) {
	//  A console interrupt landed us here.
	//  Invoke the debug agent so as to cause a SIGINT.
        return SIGINT;
    }

    if ((res = __do_syscall(func, arg1, arg2, arg3, arg4, &err, &sig)) != 0) {
        // Skip over trap instruction
        put_register(PC, get_register(PC)+4);
        put_register(R8, err);
	return sig;
    }

    return SIGTRAP;
}
#endif // CYGSEM_REDBOOT_BSP_SYSCALLS

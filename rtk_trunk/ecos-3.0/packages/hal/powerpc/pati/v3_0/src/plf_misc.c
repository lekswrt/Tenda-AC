//=============================================================================
//
//      plf_misc.c
//
//      Platform miscellaneous code/data.
//
//=============================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 2008 Free Software Foundation, Inc.                        
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
// Author(s):   Steven Clugston
// Original:    Bob Koninckx
// Date:        2008-05-08
// Purpose:     Platform specific code and data
// Description: Tables for per-platform initialization
//
//####DESCRIPTIONEND####
//
//=============================================================================

#include <pkgconf/hal.h>
#include <cyg/hal/hal_mem.h>         // HAL memory definitions
#include <cyg/hal/hal_if.h>          // hal_if_init

#include <cyg/hal/hal_io.h>          // IO macros
#include <cyg/hal/hal_intr.h>        // interrupt vectors
#include <cyg/hal/hal_misc.h>        // Helper functions
#include <cyg/hal/drv_api.h>         // HANDLED
#include <cyg/hal/hal_diag.h>

// The memory map is weakly defined, allowing the application to redefine
// it if necessary. Leave the table empty, the mpc555 has no cache ...
CYGARC_MEMDESC_TABLE CYGBLD_ATTRIB_WEAK = {
    CYGARC_MEMDESC_TABLE_END
};

void
hal_platform_init(void){  
	hal_if_init();

#if defined CYGSEM_HAL_VIRTUAL_VECTOR_DIAG  
  HAL_DIAG_INIT();
#endif
}

// EOF plf_misc.c

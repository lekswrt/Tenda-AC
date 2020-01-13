//=================================================================
//
//        memset.c
//
//        Testcase for C library memset() function
//
//=================================================================
// ####ECOSGPLCOPYRIGHTBEGIN####                                            
// -------------------------------------------                              
// This file is part of eCos, the Embedded Configurable Operating System.   
// Copyright (C) 1998, 1999, 2000, 2001, 2002 Free Software Foundation, Inc.
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
//=================================================================
//#####DESCRIPTIONBEGIN####
//
// Author(s):     ctarpy, jlarmour
// Contributors:  
// Date:          2000-04-30
// Description:   Contains testcode for C library memset() function
//
//
//####DESCRIPTIONEND####

// INCLUDES

#include <pkgconf/isoinfra.h>
#include <string.h>
#include <cyg/infra/testcase.h>


// FUNCTIONS

// Functions to avoid having to use libc strings

static int my_strcmp(const char *s1, const char *s2)
{
    for ( ; *s1 == *s2 ; s1++,s2++ )
    {
        if ( *s1 == '\0' )
            break;
    } // for

    return (*s1 - *s2);
} // my_strcmp()


static char *my_strcpy(char *s1, const char *s2)
{
    while (*s2 != '\0') {
        *(s1++) = *(s2++);
    }
    *s1 = '\0';

    return s1; 
} // my_strcpy()


#if CYGINT_ISO_MAIN_STARTUP
int main( int argc, char *argv[] )
#else
void cyg_user_start(void)
#endif
{
    char x[300];
    char *ret;

    CYG_TEST_INIT();

    CYG_TEST_INFO("Starting tests from testcase " __FILE__ " for C library "
                  "memset() function");
    CYG_TEST_INFO("This testcase provides simple basic tests");

    // Check 1
    my_strcpy(x, "      ");
    ret = memset(x, 'X', 5);
    if ((my_strcmp(x, "XXXXX ") == 0) && (ret == x))
        CYG_TEST_PASS("Simple memset");
    else
        CYG_TEST_FAIL("Simple memset");

    // Check 2
    my_strcpy(x, "XXXXX ");
    ret = memset(x, 'Y', 0);
    if ((my_strcmp(x, "XXXXX ") == 0) && (ret == x))
        CYG_TEST_PASS("Boundary case of 0 bytes");
    else
        CYG_TEST_FAIL("Boundary case of 0 bytes");

//    CYG_TEST_NA("Testing is not applicable to this configuration");

    CYG_TEST_FINISH("Finished tests from testcase " __FILE__ " for C library "
                    "memset() function");

} // main()

// EOF memset.c

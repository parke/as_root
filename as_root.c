

//  as_root.c  -  An ultra-minimalist alternative to sudo.
//
//  Copyright (c) 2023 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv3 (see end of file) WITHOUT ANY WARRANTY.


//  as_root.c  version  20231012


#include  <stdio.h>     //  fprintf, snprintf
#include  <stdlib.h>    //  exit
#include  <string.h>    //  strspn
#include  <unistd.h>    //  getuid, setuid


typedef  const int     cint;    //  ---------------------------  typedef  cint
typedef  const char *  str;     //  ----------------------------  typedef  str


static str  ALLOW  =    //  -----------------------------------  global  ALLOW

  "btrfs  subv  list  PLUS  \n"

  ;    //  end  global  ALLOW  ---------------------------  end  global  ALLOW


//  lib  ----------------------------------------------------------------  lib


/*  20231011  xray() is used during debugging.
static void  xray  ( str m, str * argv )  {    //  ---------------------  xray
  fprintf ( stderr, "%s", m );
  for     (  ;  * argv;  argv ++ )
  {       fprintf  ( stderr, "%s  ", * argv );  }
  fprintf ( stderr, "\n" );  }
*/


static str  basename  ( str path )  {    //  -----------------------  basename
  str  p  =  strrchr ( path, '/' );
  return  p  ?  p+1  :  path  ;  }


static int  die  ( str m )  {    //  ------------------------------------  die
  fprintf ( stderr, "as_root  fatal error  %s\n", m );
  exit ( 1 );  }


static void  do_exec  ( str argv[] )  {    //  ----------------------  do_exec

  str   PATH[]     =  {  "/usr/bin",  "/usr/sbin",  "/bin",  "/sbin",
                          NULL  };
  str   filename   =  basename ( argv[0] );
  char  buf[80];

  for  (  str * p  =  PATH;  * p;  p ++ )
  {    cint  n     =  snprintf ( buf, sizeof buf, "%s/%s", * p, filename );
       n > ( sizeof buf ) - 5  &&  die("snprintf");    //  overflow
       argv[0]     =  buf;
       execv ( argv[0], (char**) argv );  }

  die ( "exec" );  }


static void  drop  ()  {    //  ----------------------------------------  drop
  setgid ( getgid() )  &&  die("setgid");
  setuid ( getuid() )  &&  die("setuid");  }


static str  skip_one  ( str p, cint c )  {    //  ------------------  skip_one
  return  p  &&  p[0] == c  ?  p+1  :  p  ;  }


static str  skip_past ( str p, str accept )  {    //  -------------  skip_past
  return  p  ?  p + strspn ( p, accept )  :  p  ;  }


static str  skip_to  ( str p, str accept )  {    //  ----------------  skip_to
  return  p  ?  p + strcspn ( p, accept )  :  p  ;  }


//  match  ------------------------------------------------------------  match


static str *  argv;    //  -------------------------------------  global  argv
static str *  arg;     //  -------------------------------------  global  arg
static str    p;       //  -------------------------------------  global  p
static int    pn;      //  -------------------------------------  global  pn


/*  20231011  p_xray() is used during debugging.
static void  p_xray  ( str m, str m2 )  {    //  ---------------------  p_xray
  char  buf[80];
  memcpy ( buf, p, pn );
  buf [ pn ]  =  '\0';
  fprintf ( stderr, "%s  %d  '%s'  %s\n", m, pn, buf, m2 );  }
*/


static int  p_is  ( str s )  {    //  ----------------------------------  p_is
  //  p_xray ( "p_is", s );    //  20231011
  if  ( s == NULL )
  {   return  p == NULL  ||  p[0] == '\0'  ||  p[0] == '\n';  }
  return  strncmp ( p, s, pn ) == 0;  }


static int  p_len  ()  {    //  ---------------------------------------  p_len
  return  strcspn ( p, " \n" );  }


static int  p_not  ( str s )  {    //  --------------------------------  p_not
  return  !  p_is(s);  }


static void  arg_next  ()  {    //  --------------------------------  arg_next
  if  ( * arg )  {  arg ++;  }
  p   =  skip_to   ( p, " \n" );
  p   =  skip_past ( p, " "   );
  pn  =  p_len();  }


static void  line_first  ( str argv_main[] )  {    //  ----------  line_first
  argv  =  argv_main;
  arg   =  argv;
  p     =  ALLOW;
  pn    =  p_len();  }


static void  line_next  ()  {    //  ------------------------------  line_next
  arg  =  argv;
  p    =  skip_to  ( p, "\n" );
  p    =  skip_one ( p, '\n' );
  pn   =  p_len();  }


static int  is_match  ()  {    //  ---------------------------------  is_match

  if  ( p_not ( basename ( * arg ) ) )  {  return  0;  }

  loop  :
  arg_next();
  if  ( * arg == NULL )  {  return  p_is(NULL)  ||  p_is("STAR");  }
  if  ( p_is("PLUS")  )  {  return  1;   }
  if  ( p_is("STAR")  )  {  return  1;   }
  if  ( p_is("ARG")   )  {  goto  loop;  }
  if  ( p_is(*arg)    )  {  goto  loop;  }

  return  0;  }


//  main  --------------------------------------------------------------  main


void  main  ( cint argc, str argv[] )  {    //  ------------------------  main
  for  (  line_first(argv);  * p;  line_next()  )
  {    if  ( is_match ()  )
       {   do_exec ( argv );  }  }

  //  xray ( "as_root  DROP  ", argv );    //  20231011
  drop();
  do_exec ( argv );  }








//  as_root.c  -  An ultra-minimalist alternative to sudo.
//
//  Copyright (c) 2023 Parke Bostrom, parke.nexus at gmail.com
//
//  This program is free software: you can redistribute it and/or
//  modify it under the terms of version 3 of the GNU General Public
//  License as published by the Free Software Foundation.
//
//  This program is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See version
//  3 of the GNU General Public License for more details.
//
//  You should have received a copy of version 3 of the GNU General
//  Public License along with this program.  If not, see
//  <https://www.gnu.org/licenses/>.

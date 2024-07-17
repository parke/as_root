

//  as_root.c  -  An ultra-minimalist alternative to sudo.
//
//  Copyright (c) 2024 Parke Bostrom, parke.nexus at gmail.com
//  Distributed under GPLv3 (see end of file) WITHOUT ANY WARRANTY.


//  as_root.c  version  20240717


#include  <stdbool.h>    //  bool  true  false
#include  <stdio.h>      //  printf  fprintf  snprintf
#include  <stdlib.h>     //  exit
#include  <string.h>     //  strcmp  strspn  strstr
#include  <unistd.h>     //  getgid  getuid  setgid  setuid


typedef  const int     cint;    //  ---------------------------  typedef  cint
typedef  const char *  str;     //  ----------------------------  typedef  str




static void  usage  ()  {    //  --------------------------------------  usage
  printf
    (  "\n"
       "usage:  as_root  command  [arg ...]\n"  );  }

//  end  usage  --------------------------------------------------  end  usage




static str  ALLOW  =    //  -----------------------------------  global  ALLOW

  "btrfs  subv  list  PLUS  \n"
  "cryptsetup  luksDump  ARG  \n"
  "id  \n"
  "id  -a  -a\n"

  ;    //  end  global  ALLOW  ---------------------------  end  global  ALLOW




//  mutable global variables  ----------------------  mutable global variables

static bool   as_root;    //  process was invoked as "as_root"?
static str *  argv;       //  points to command in argv_main[].
static str *  arg;        //  walks from argv[1] to argv[n].
static str    p;          //  walks through each line of ALLOW.
static int    p_n;        //  the length of the current token in ALLOW.

//  end  mutable global variables  ------------  end  mutable global variables




//  lib  ----------------------------------------------------------------  lib


static str  basename  ( str path )  {    //  -----------------------  basename
  str  p  =  strrchr ( path, '/' );
  return  p  ?  p+1  :  path  ;  }


static bool  die  ( str m )  {    //  -----------------------------------  die
  fprintf ( stderr, "as_root  fatal error  %s\n", m );
  exit ( 1 );  }


static void  do_exec  ()  {    //  ----------------------------------  do_exec

  str  envp[]  =  { NULL };    //  use an empty environment.

  if(  as_root  &&  strstr ( argv[0], "/" )  )
  {    //  PATH search is unnecessary.  exec argv without change.
       execve ( argv[0], (char**) argv, (char**) envp );
       die("execve");  return;  }    //  die on failure.

  //  PATH search is necessary.
  str   PATH[]    =  {  "/usr/bin",  "/usr/sbin",  "/bin",  "/sbin",
                          NULL  };
  str   filename  =  basename ( argv[0] );
  char  buf[80];    //  hopefully 80 bytes is enough.
  for(  str * p2  =  PATH;  * p2;  p2 ++ )
  {     cint  n   =  snprintf ( buf, sizeof buf, "%s/%s", * p2, filename );
        if(  n > ( (int) sizeof buf ) - 5  )    //  5 is an approximation.
        {    die("snprintf");  }                //  snprintf overflow
        argv[0]   =  buf;
        execve ( argv[0], (char**) argv, (char**) envp );  }

  die ( "execve" );  }


static void  drop  ()  {    //  ----------------------------------------  drop
  if(  setgid ( getgid() )  )  {  die("setgid");  }
  if(  setuid ( getuid() )  )  {  die("setuid");  }  }


static bool  is_equal  ( str a, str b )  {    //  ------------------  is_equal
  return  strcmp ( a, b ) == 0;  }


static str  skip_one  ( str p, cint c )  {    //  ------------------  skip_one
  return  p  &&  p[0] == c  ?  p+1  :  p  ;  }


static str  skip_past ( str p, str accept )  {    //  -------------  skip_past
  return  p  ?  p + strspn ( p, accept )  :  p  ;  }


static str  skip_to  ( str p, str accept )  {    //  ----------------  skip_to
  return  p  ?  p + strcspn ( p, accept )  :  p  ;  }


//  end  lib  ------------------------------------------------------  end  lib




//  match  ------------------------------------------------------------  match


static bool  is_arg    ()  {  return  * arg  ?  1  :  0  ;  }    //  -  is_arg
static bool  not_arg   ()  {  return  !  is_arg();  }    //  --------  not_arg
static bool  p_is_eol  ()  {  return  p_n == 0;  }    //  ----------  p_is_eol


static bool  p_is  ( str expect )  {    //  ----------------------------  p_is
  //  return true iff p equals expect.
  return  strncmp ( p, expect, p_n ) == 0  &&  expect[p_n] == '\0';  }


static int  p_len  ()  {    //  ---------------------------------------  p_len
  //  return the length of the current token.
  return  strcspn ( p, " \n" );  }


static void  arg_next  ()  {    //  --------------------------------  arg_next
  if(  * arg  )  {  arg ++;  }  }


static void  p_next  ()  {    //  ------------------------------------  p_next
  //  advance p to the next token on the current line.
  p    =  skip_to   ( p, " \n" );
  p    =  skip_past ( p, " "   );
  p_n  =  p_len();  }


static void  p_next_line  ()  {    //  --------------------------  p_next_line
  //  advance p to the beginning of the next line.
  p    =  skip_to  ( p, "\n" );
  p    =  skip_one ( p, '\n' );
  p_n  =  p_len();  }


static bool  line_match  ()  {    //  -----------------------------  line_match
  //  return true iff p accepts argv.
  for(   arg  =  argv;
	 true;
	 arg_next(), p_next()  )  {
    if(  p_is_eol()                 )  {  return  not_arg();  }
    if(  not_arg()                  )  {  return  false;  }
    if(  p_is("STAR")               )  {  return  true;  }
    if(  p_is("PLUS")               )  {  return  is_arg();  }
    if(  p_is("ARG")  &&  is_arg()  )  {  continue;  }
    if(  p_is(*arg)                 )  {  continue;  }
    return  false;  }  }


//  end  match  --------------------------------------------------  end  match




//  main  --------------------------------------------------------------  main


static void  init  ( str argv_main[] )  {    //  -----------------------  init

  p          =  ALLOW;
  p_n        =  p_len();
  argv       =  argv_main;    //  assume
  as_root    =  is_equal ( basename ( argv[0] ), "as_root" );

  if(  as_root  )
  {    argv  =  argv_main + 1;  }  }


int  main  ( cint argc, str argv_main[] )  {    //  -------------------  main

  init ( argv_main );

  if(  as_root  &&  argc == 1 )  {  usage();  exit(1);  }

  for(  ;  * p;  p_next_line()  )
  {     if(  line_match ()  )
        {    do_exec();          //  do_exec() should never return.
             return  1;  }  }    //  we should never get here.

  if(   as_root  )
  {     die("match failed.");  }

  drop();
  do_exec();       //  do_exec() should never return.

  return  1;  }    //  we should never get here.


//  end  main  ----------------------------------------------------  end  main








//  as_root.c  -  An ultra-minimalist alternative to sudo.
//
//  Copyright (c) 2024 Parke Bostrom, parke.nexus at gmail.com
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

/*
 * This file is Public Domain
 *
 * Just short example, how Fred Fish's dbug package should be used
 *
 * Tonu Samuel <tonu@spam.ee>
 *
 */
#include <stdint.h>
#include <string.h>
#include "dbug.h"
static int sub1 (void);
static void sub2 (const char *arg);

static int
sub1 (void)
{
    DBUG_ENTER ("sub1");
    sub2 ("Hello world!");
    sub2 ("Hello earth!");;
    sub2 ("Hello programmer!");

    DBUG_RETURN (0);
}


static void
sub2 (const char *arg)
{
    DBUG_ENTER ("sub2");
    DBUG_PRINT ("info", ("Got argument: '%s'", arg));

    printf ("%s\n", arg);

    DBUG_VOID_RETURN;
}


int
main (void)
{
    int ret = 0;
    DBUG_PUSH ("d:L:t:F");
    DBUG_PROCESS ("example");

    ret = sub1 ();
    /*warring you should make sure the DBUG_PRINT is between DBUG_ENTER and (DBUG_VOID_RETURN or DBUG_RETURN)*/
    DBUG_PRINT("info",("return value eq %d ",ret));
    return 0;
}


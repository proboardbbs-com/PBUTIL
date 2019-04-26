#include <string.h>
#include <stdio.h>
#include "pbutil.hpp"

typedef struct
  {
    Date     date;
    Time     timeIn;
    Time     timeOut;
    char     name[36];
    char     city[26];
    char     country[26];
    long     baud;
    word     node;
    long     kbDown;
    long     kbUp;
    word     yells;
    word     level;
    dword    uflags;
    byte     extra[81];
  } BINLOG;


void
check_binlog(int,char *[])
{
   File fi,fo;

   Log("----------------------------------------------------");
   Log("FB: Fix BINLOG");

   if(!fi.open(FileName(syspath,"BINLOG.PB"),fmode_read,10000))
   {
      printf("Can't open BINLOG.PB\n");
      Log("Can't open BINLOG.PB");
      return;
   }

   if(!fo.open(FileName(syspath,"BINLOG.$$$"),fmode_create,10000))
   {
      printf("Can't create BINLOG.$$$\n");
      Log("Can't create BINLOG.$$$");
      return;
   }


   BINLOG bl;

   for(long err_count=0;;)
   {
      if(fi.read(&bl,sizeof(bl)) != sizeof(bl)) break;

      if(   bl.timeIn[0] > 23
         || bl.timeOut[0] > 23
         || !bl.date.ok()
         || strlen(bl.name) > 35
         || strlen(bl.city) > 25
         || strlen(bl.country) > 25
         || bl.baud < 0
         || bl.baud > 115200L
         || bl.node < 1
         || bl.node > 255
        )
      {
         err_count++;
      }
      else
      {
         fo.write(&bl,sizeof(bl));

         if(fo.error())
         {
            printf("Error writing to output file (disk full?)\n");
            Log("Error writing to output file (disk full?)");
            fo.close();
            unlink(FileName(syspath,"BINLOG.$$$"));
            return;
         }
      }
   }

   fi.close();
   fo.close();

   if(err_count)
   {
      Log("%ld damaged records removed from BINLOG.PB",err_count);
      printf("%ld damaged records removed from BINLOG.PB\n",err_count);
      unlink(FileName(syspath,"BINLOG.PB"));
      rename(FileName(syspath,"BINLOG.$$$"),FileName(syspath,"BINLOG.PB"));
   }
   else
   {
      Log("BINLOG.PB is undamaged");
      printf("BINLOG.PB is undamaged\n");
      unlink(FileName(syspath,"BINLOG.$$$"));
   }
}

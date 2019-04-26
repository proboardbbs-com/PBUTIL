#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pbutil.hpp"

static char *valid="ABCDEFGHIJKLMNOPQRSTUVWXYZ_1234567890#$!";

void
fidx(int,char *[])
{
File f;
File fidx;
FilesIdx idxRec;

if(!f.open(FileName(syspath,"FILECFG.PRO"),fmode_read,4096))
  {
   Log("Unable to open FILECFG.PRO");

   printf("Can't open FILECFG.PRO\n");
   return;
  }

if(!fidx.open(FileName(syspath,"FILESIDX.PB"), fmode_create , 16384))
  {
   Log("Unable to create FILESIDX.PB");

   printf("Can't create FILESIDX.PB\n");
   return;
  }

printf("Indexing file database...");

char *str = new char[1024];

for(int a=1;;a++)
  {
   _FileArea fa;
   File tf;

   if(f.read(&fa,sizeof(fa))!=sizeof(fa)) break;

   if(fa.name[0] == '\0') continue;

   if(!tf.open(fa.listpath,fmode_read | fmode_text , 16384)) continue;

   for(;;)
     {
     if(!tf.readLine(str,1023)) break;
     strip_linefeed(str);
     if(!strchr(valid,toupper(str[0]))) continue;

     for(int i=0;str[i];i++)
      if(str[i] == ' ' || str[i] == '\t')
        {
         str[i] = '\0';
         break;
        }

     if(i > 12 || str[0] == '\0') continue;

     strcpy(idxRec.name,str);
     strupr(idxRec.name);
     idxRec.area = a;
     fidx.write(&idxRec,sizeof(idxRec));
     }
  }

delete [] str;

printf("Done.\n");
}


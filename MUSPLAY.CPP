#include <time.h>
#include <dos.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <bios.h>
#include "pbutil.hpp"

struct music_data
  {
   unsigned freq;
   unsigned dur;
  };

void
musplay(int argc,char *argv[])
{
int i;
music_data *data;

if(argc<1)
  {
   printf("No file specified!\n");
   return;
  }

FileName fn(syspath,argv[0]);

fn.changeExt("MUS");

File fp(fn,fmode_read | fmode_text);

if(!fp.opened())
  {
   printf("File not found: %s\n",(char *)fn);
   return;
  }

printf("Playing file %s (Any key stops)\n",strupr(fn));

data=new music_data[1000];

for(i=0;i<1000;)
  {
   char str[100];
   if(!fp.readLine(str,99)) break;
   char *p=strtok(str," \n");
   if(!strcmpl(p,"TONE"))
     {
      data[i].freq=atoi(strtok(NULL," \n"));
      data[i++].dur =atoi(strtok(NULL," \n"));
     }
   if(!strcmpl(p,"WAIT"))
     {
      data[i].freq=0;
      data[i++].dur =atoi(strtok(NULL," \n"));
     }
  }

fp.close();

int numnotes=i;

for(i=0;i<numnotes;i++)
  {
   if(bioskey(1))
     {
      bioskey(0);
      break;
     }

   if(data[i].freq)
     {
      sound(data[i].freq);
      msleep(data[i].dur*10);
     }
    else
     {
      nosound();
      msleep(data[i].dur*10);
     }
  }

nosound();
delete [] data;
}

#include <stdio.h>
#include <io.h>
#include <string.h>
#include "pbutil.hpp"

void
msgridx(int argc,char *[])
{
 FileName fn;
 File fh;
 int i;
 msgarea ma;

 Log("컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴");
 Log("MI: Message reindex");

 fn(syspath,"MESSAGES.PB");

   if(!fh.open(fn))
   {
      Log("Unable to open MESSAGES.PB");
      printf("Can't open MESSAGES.PB\n");
      return;
   }

   int jampacked = 0;

   if(argc >= 0)
   {
      fh.rewind();

      for(i = 0 ; i < 10000 ; i++)
      {
         if(fh.read(&ma , sizeof(msgarea)) != sizeof(msgarea))
            break;

         if(ma.msgBaseType == MSGBASE_JAM)
         {
            jampacked++;

            JamReIndex(ma,TRUE);
         }
      }
   }

   fh.close();

   if(jampacked)
      printf("\n");


 fn(cfg.msgpath,"MSGHDR.BBS");
 if(!fh.open(fn,fmode_read,64000u))
   {
    Log("Unable to open MSGHDR.BBS");

    printf("Can't open MSGHDR.BBS\n");
    return;
   }

 fn(cfg.msgpath,"MSGIDX.BBS");
 File fi;
 fi.open(fn,fmode_create,64000u);
 fn(cfg.msgpath,"MSGTOIDX.BBS");
 File ft;
 ft.open(fn,fmode_create,64000u);

 if(argc>=0) printf("Reindexing Hudson MsgBase...");

 msginfo info;
 memset(&info,0,sizeof(info));
 info.low=0x7FFF;
 int total=0;

 for(;;)
   {
    qbbs_msg hdr;
    msgidx mi;
    char name[36];

    if(fh.read(&hdr,sizeof(hdr))!=sizeof(hdr)) break;

    if(hdr.msgnum < 1) hdr.msgattr |= QMSG_DELETED;

    if(hdr.area<1 || hdr.area>200)
      {
       hdr.area = 1;
       hdr.msgattr |= QMSG_DELETED;
      }

    mi.num  = hdr.msgnum;
    mi.area = hdr.area;

    if(hdr.to[0]<0 || hdr.to[0]>35)
      {
       hdr.to[0] = 0;
       hdr.msgattr |= QMSG_DELETED;
      }

    pas2c(hdr.to);
    memset(name,0,36);
    strcpy(name,hdr.to);

    if(hdr.msgattr & QMSG_RECEIVED) strcpy(name,"* Received *");

    if(hdr.msgattr & QMSG_DELETED)
       {
        strcpy(name,"* Deleted *");
        mi.num = -1;
       }
      else
       {
        info.active[hdr.area-1]++;
        info.total++;
       }
    total++;

    fi.write(&mi,sizeof(mi));

    if(!(hdr.msgattr & QMSG_DELETED))
      {
       if(hdr.msgnum>info.high) info.high = hdr.msgnum;
       if(hdr.msgnum<info.low ) info.low  = hdr.msgnum;
      }

    c2pas(name);

    ft.write(name,36);
   }

 if(argc>=0) printf("Done.\n");

 fi.close();
 fh.close();
 ft.close();

 fn(cfg.msgpath,"MSGINFO.BBS");
 fi.open(fn,fmode_create);
 fi.write(&info,sizeof(info));
 fi.close();

 if(argc<0) return;

 Log("Total : %d messages (#%d to #%d)",total,info.low,info.high);

 printf("\nHudson Stats:  * Total messages : %d (%d to %d)\n",total,info.low,info.high);
 printf(  "               * Active messages: %d\n",info.total);

 total=0;
 for(i=0;i<200;i++) if(info.active[i]) total++;

 Log("Active: %d messages (%d areas)",info.total,total);

 printf(  "               * Active areas   : %d\n",total);
}

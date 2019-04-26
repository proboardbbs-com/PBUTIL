/*
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "pbutil.hpp"

void
msgkill(int,char *[])
{
int *idx_array=new int[32001];

for(int i=1;i<=32000;i++) idx_array[i]=-1;


printf("Message-Killer\n"
       "컴컴컴컴컴컴컴\n\n");

msgridx(-1,NULL);

msgarea *ma=new msgarea[200];

memset(ma,0,200*sizeof(msgarea));

File f(FileName(syspath,"MSG.PRO"));
if(!f.opened())
  {
   printf("Can't open MSG.PRO\n");
   return;
  }

f.read(ma,200*sizeof(msgarea));
f.close();

if(!f.open(FileName(cfg.msgpath,"MSGINFO.BBS"),fmode_rw))
  {
   printf("Can't open MSGINFO.BBS\n");
   return;
  }

msginfo minfo;

f.read(&minfo,sizeof(minfo));
f.close();

if(!f.open(FileName(cfg.msgpath,"MSGHDR.BBS"),fmode_rw,4096))
  {
   printf("Can't open MSGHDR.BBS\n");
   return;
  }

qbbs_msg msg;
int deleted[200];
memset(deleted,0,400);

printf("Processing messages...(    0 deleted)");

f.rewind();

for(i=0;;i++)
  {
   if(f.read(&msg,sizeof(msg))!=sizeof(msg)) break;

   if(msg.msgattr & QMSG_DELETED) continue;

   idx_array[msg.msgnum]=i;
  }

f.rewind();

int del=0;
for(i=0;;i++)
  {
  char date[9];
  int days;
  Date today(TODAY),d;

  f.seek(long(i) * sizeof(msg));
  if(f.read(&msg,sizeof(msg))!=sizeof(msg)) break;

  if(msg.msgattr & QMSG_DELETED) continue;

  msg.postdate[0] = 8;

  memmove(date,msg.postdate,9);

  pas2c(date);

  d[1] = atoi(strtok(date,"-"));
  d[0] = atoi(strtok(NULL,"-"));
  d[2] = atoi(strtok(NULL,"-"));

  days=today-d;

  int kill=0;

  if(ma[msg.area-1].msg_kill_days)
    if(days > ma[msg.area-1].msg_kill_days) kill = TRUE;

  if(ma[msg.area-1].rcv_kill_days)
    if(days > ma[msg.area-1].rcv_kill_days && (msg.msgattr & QMSG_RECEIVED)) kill = TRUE;

  if(ma[msg.area-1].max_msgs)
    if(minfo.active[msg.area-1]-deleted[msg.area-1]>ma[msg.area-1].max_msgs) kill = TRUE;

  if(kill)
    {
    qbbs_msg tmpmsg;

    msg.msgattr |= QMSG_DELETED;

    if(msg.area > 200 || msg.area < 1) msg.area = 1;

    deleted[msg.area-1]++;
    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b%5d deleted)",++del);
    idx_array[msg.msgnum]=-1;
    if(msg.prevmsg)
       {
       if(idx_array[msg.prevmsg]>=0)
         {
         f.seek(long(idx_array[msg.prevmsg])*sizeof(msg));
         f.read(&tmpmsg,sizeof(tmpmsg));
         if(tmpmsg.msgattr & QMSG_DELETED) tmpmsg.prevmsg=tmpmsg.nextmsg=0;
         tmpmsg.nextmsg=msg.nextmsg;
         f.seek(long(idx_array[msg.prevmsg])*sizeof(msg));
         f.write(&tmpmsg,sizeof(tmpmsg));
         if(tmpmsg.msgattr & QMSG_DELETED) msg.prevmsg=0;
         }
       }
    if(msg.nextmsg)
       {
       if(idx_array[msg.nextmsg]>=0)
         {
         f.seek(long(idx_array[msg.nextmsg])*sizeof(msg));
         f.read(&tmpmsg,sizeof(tmpmsg));
         if(tmpmsg.msgattr & QMSG_DELETED) tmpmsg.prevmsg=tmpmsg.nextmsg=0;
                                 else  tmpmsg.prevmsg=msg.prevmsg;
         f.seek(long(idx_array[msg.nextmsg])*sizeof(msg));
         f.write(&tmpmsg,sizeof(tmpmsg));
         }
       }
    msg.nextmsg=msg.prevmsg=0;
    f.seek(long(i)*sizeof(msg));
    f.write(&msg,sizeof(msg));
    }
  }

delete [] idx_array;

printf("\n\nUpdating reply chains...");

msgridx(-1,NULL);

printf("Done.\n");
}

*/

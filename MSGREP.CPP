/*
#include <stdio.h>
#include "pbutil.hpp"

void
msgrep(int,char *[])
{
File fh,ft;

FileName fn(cfg.msgpath,"MSGTXT.BBS");

if(!ft.open(fn))
  {
   printf("Can't open MSGTXT.BBS\n");
   return;
  }

fn(cfg.msgpath,"MSGHDR.BBS");

if(!fh.open(fn,fmode_rw,64000))
  {
   printf("Can't open MSGHDR.BBS\n");
   return;
  }

int numrecs=int(ft.len()/256L);

ft.close();

printf("Repairing messagebase...");

int numdel=0,numfix=0;

for(int i=0;;i++)
  {
  qbbs_msg msg;
  int kill=0,fix=0;
  Dstring err;

  if(fh.read(&msg,sizeof(msg))!=sizeof(msg)) break;

  if(msg.msgnum<1    || msg.msgnum>32000) { kill=1; err="Invalid messagenumber"; }
  if(msg.numrecs>250 || msg.numrecs<1) { kill=1; err="Invalid message-size"; }
  if(msg.startrec>=numrecs) { kill=1; err="Invalid message link"; }
  if(msg.from[0]<0   || msg.from[0]>35) { fix=1; msg.from[0]=0; err="Invalid 'from' field"; }
  if(msg.to[0]<0     || msg.to[0]>35) { fix=1; msg.to[0]=0; err="Invalid 'to' field"; }
  if(msg.subj[0]<0   || msg.subj[0]>66) { fix=1; msg.subj[0]=0; err="Invalid 'subj' field"; }
  if(msg.nextmsg<0   || msg.nextmsg>32000 || msg.prevmsg<0 || msg.prevmsg>32000) { fix=1; msg.nextmsg=msg.prevmsg=0; err="Invalid reply chain"; }
  if(msg.area<1      || msg.area>200) { kill=1; err="Invalid area"; }

  if(kill || fix)
    {
    if(numdel || numfix) printf("                        ");
    if(kill) printf("Msg #%d removed - %s\n",msg.msgnum,(char *)err);
       else  printf("Msg #%d fixed - %s\n",msg.msgnum,(char *)err);

    if(kill)
      {
       msg.msgattr |= QMSG_DELETED;
       numdel++;
      }
     else numfix++;

    fh.seek(long(i)*sizeof(msg));
    fh.write(&msg,sizeof(msg));
    fh.seek(long(i+1)*sizeof(msg));
    }
  }

fh.close();

if(!numdel && !numfix) printf("Everything seems to be ok.\n");
     else
       {
       if(numdel) printf("\n%d damaged messages removed.\n",numdel);
       if(numfix) printf("\n%d damaged messages fixed.\n",numfix);
       printf("\n");

       msgpack(0,NULL);
       printf("\n");
       msglink(0,NULL);
       printf("\n");
       }

}
*/


#include <ctype.h>
#include <string.h>
#include <stdlib.h>
#include <stdio.h>
#include <dos.h>
#include "pbutil.hpp"

class msgline
  {
  unsigned char s[256];
 public:
  void operator=(char *x) { memmove(s,x,256); }
  void clean()            { if(s[0]<255) memset(&s[0]+s[0]+1,0,255-s[0]); }
  };

struct msgAreaMax
   {
       int msgKillDays;
       int rcvKillDays;
       int maxMsgs;
   };

void
clean_header(qbbs_msg *hdr)
{
 qbbs_msg tmp;

 memset(&tmp,0,sizeof(tmp));

 tmp.msgnum   = hdr->msgnum;
 tmp.prevmsg  = hdr->prevmsg;
 tmp.nextmsg  = hdr->nextmsg;
 tmp.tread    = hdr->tread;
 tmp.startrec = hdr->startrec;
 tmp.numrecs  = hdr->numrecs;
 tmp.destnet  = hdr->destnet;
 tmp.destnode = hdr->destnode;
 tmp.orgnet   = hdr->orgnet;
 tmp.orgnode  = hdr->orgnode;
 tmp.destzone = hdr->destzone;
 tmp.orgzone  = hdr->orgzone;
 tmp.cost     = hdr->cost;
 tmp.msgattr  = hdr->msgattr;
 tmp.netattr  = hdr->netattr;
 tmp.area     = hdr->area;

 memmove(tmp.posttime , hdr->posttime , unsigned( hdr->posttime[0] ) + 1);
 memmove(tmp.postdate , hdr->postdate , unsigned( hdr->postdate[0] ) + 1);
 memmove(tmp.to       , hdr->to       , unsigned( hdr->to[0]       ) + 1);
 memmove(tmp.from     , hdr->from     , unsigned( hdr->from[0]     ) + 1);
 memmove(tmp.subj     , hdr->subj     , unsigned( hdr->subj[0]     ) + 1);

 tmp.recvdate=hdr->recvdate;
 tmp.recvtime=hdr->recvtime;

 *hdr=tmp;
}




void
msgpack(int argc,char *argv[])
{
 FileName fn,fn2;
 bool renumber = FALSE,
      force    = FALSE,
      do_kill  = FALSE,
      killbak  = FALSE,
      do_jam   = TRUE,
      do_hudson= TRUE;

 Log("컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴");
 Log("MP - Message Base Packer");

 for(int i=0;i<argc;i++)
   if(argv[i][0]=='-')
     switch(toupper(argv[i][1]))
       {
        case 'R': renumber = TRUE; break;
        case 'F': force    = TRUE; break;
        case 'K': killbak  = TRUE; break;
        case 'D': do_kill  = TRUE; break;
        case 'H': do_jam   = FALSE; break;
        case 'J': do_hudson = FALSE; break;
       }
     else if(argv[i][0]=='?')
            {
             printf("Message packer options\n"
                    "컴컴컴컴컴컴컴컴컴컴컴\n\n"
                    "   [-R]  Renumber messagebase\n"
                    "   [-D]  Delete old messages\n"
                    "   [-F]  Force pack\n"
                    "   [-K]  Kill .BAK files after pack\n"
                    "   [-H]  Hudson message base only\n"
                    "   [-J]  JAM message base only\n\n"
                    " These options can be combined.\n\n");
             return;
            }

 msgridx(-1,NULL);

 msgarea *ma=new msgarea;

 memset(ma,0,sizeof(msgarea));

 File f(FileName(syspath,"MESSAGES.PB"));
 if(!f.opened())
   {
    Log("Unable to open MESSAGES.PB");
    printf("Can't open MESSAGES.PB\n");
    return;
   }

 f.rewind();

 int jampacked = 0;

 if(do_jam)
   for(i = 0 ; i < 10000 ; i++)
   {
      if(f.read(ma , sizeof(msgarea)) != sizeof(msgarea))
         break;

      if(ma->msgBaseType == MSGBASE_JAM)
      {
         JamPack(*ma , renumber , do_kill);
         jampacked++;
      }
   }

 if(jampacked) printf("\n");

 if(!do_hudson)
   return;

 msgAreaMax ma_max[200];

 f.rewind();

 for(i=0;i<200;i++)
 {
   if(f.read(ma,sizeof(*ma)) != sizeof(*ma))
      break;
   ma_max[i].maxMsgs     = ma->maxMsgs;
   ma_max[i].rcvKillDays = ma->rcvKillDays;
   ma_max[i].msgKillDays = ma->msgKillDays;
 }
 f.close();

 if(!f.open(FileName(cfg.msgpath,"MSGINFO.BBS"),fmode_rw))
   {
    Log("Unable to open MSGINFO.BBS");
    printf("Can't open MSGINFO.BBS\n");
    return;
   }

 msginfo minfo;

 f.read(&minfo,sizeof(minfo));
 f.close();

 fn(cfg.msgpath,"MSGHDR.BBS");
 File fhi(fn,fmode_read,50000u);
 if(!fhi.opened())
   {
    Log("Unable to open MSGHDR.BBS");
    printf("Can't open MSGHDR.BBS\n");
    return;
   }

 fn(cfg.msgpath,"MSGTXT.BBS");
 File fti(fn,fmode_read,50000u);
 if(!fti.opened())
   {
    Log("Unable to open MSGTXT.BBS");
    printf("Can't open MSGTXT.BBS\n");
    return;
   }

 long num_txt_records = fti.len() / 256;

 int  msgs     = 0,
      delmsgs  = 0;
 long bytes    = 0,
      delbytes = 0;

 unsigned *offset=new unsigned[32001];

 memset(offset,0,32001*sizeof(unsigned));

 qbbs_msg hdr;

 BitArray msgtxt_used(65536L);
 BitArray msg_used(32000L,1);

 for(;;)
   {
    if(fhi.read(&hdr,sizeof(hdr))!=sizeof(hdr)) break;

    msgs++;

    bytes += long(hdr.numrecs) * 256L + long(sizeof(hdr)) + 39;

    if(hdr.msgnum > 32000 || hdr.msgnum < 1)
      {
       hdr.msgattr |= QMSG_DELETED;
      }

    if(long(hdr.numrecs) + long(hdr.startrec) > num_txt_records)
      {
       hdr.msgattr |= QMSG_DELETED;
       hdr.startrec = hdr.numrecs = 0;
      }

    if(hdr.msgattr & QMSG_DELETED)
      {
       delmsgs++;
       delbytes+=long(hdr.numrecs)*256L+long(sizeof(hdr))+39;
       continue;
      }
     else
      {
       msg_used.set(hdr.msgnum);
      }
   }

 printf("Hudson MsgBase:   * Total messages      : %5d  = %5ld Kb\n",msgs,bytes/1024);
 printf("                  * Deleted messages    : %5d  = %5ld Kb\n\n",delmsgs,delbytes/1024);
 printf("                  * Disk space required :          %5ld Kb\n",(bytes-delbytes)/1024);

 if(dos_getdiskfreespace(toupper(cfg.msgpath[0])-'A'+1)<bytes-delbytes+30000)
   {
    Log("Insufficient disk space (%ld Kb required)",(bytes-delbytes+30000)/1024);
    printf("\nInsufficient free disk space to pack!\n");
    return;
   }

 if(minfo.high > 25000)
   {
    Log("Highest msg# > 25000 -> Renumber");
    renumber=1;
   }

 if(!delmsgs && !force && !renumber && !do_kill)
    {
     printf("\nNothing to pack!\n\n");
     Log("No messages to pack");
     return;
    }

 Log("%d messages are marked as deleted",delmsgs);

 fhi.rewind();
 fti.rewind();

 printf("\nPacking...");

 if(renumber) printf("(RENUMBER)...");

 printf("0    ");

 File fho,fto;

 fn(cfg.msgpath,"MSGHDR.$$$");
 fho.open(fn,fmode_create,50000u);
 fn(cfg.msgpath,"MSGTXT.$$$");
 fto.open(fn,fmode_create,50000u);

 int extra_killed = 0;

 unsigned rec=0;

 i=0;

 for(int msgnum=1 , prevmsg = 0;;i++)
   {
    int l;
    msgline line;

    if(fhi.read(&hdr,sizeof(hdr))!=sizeof(hdr)) break;

    if(!(i%25)) printf("\b\b\b\b\b%-5d",i);

    if(hdr.msgattr & QMSG_DELETED) continue;

    if(!hdr.fix())
      {
       hdr.msgattr |= QMSG_DELETED;

       Log("Msg #%d damaged (Deleted)",hdr.msgnum);

       continue;
      }

    for(l=0 ; l<hdr.numrecs ; l++)
      {
       if(   long(hdr.startrec)+l > num_txt_records
          || msgtxt_used[long(hdr.startrec)+l])
         {
          hdr.msgattr |= QMSG_DELETED;

          Log("Msg #%d crosslinked (Deleted)",hdr.msgnum);

          break;
         }

       msgtxt_used.set(long(hdr.startrec)+l);
      }

    if( hdr.nextmsg
        &&
       (   hdr.nextmsg < 0
        || hdr.nextmsg > minfo.high
        || !msg_used[hdr.nextmsg]
       ) )
         {
          Log("Msg #%d has bad forward reply link -> #%d (Fixed)",hdr.msgnum,hdr.nextmsg);
          hdr.nextmsg = 0;
         }

    if( hdr.prevmsg
       &&
       (   hdr.prevmsg < 0
        || hdr.prevmsg > minfo.high
        || !msg_used[hdr.prevmsg]
       ) )
         {
          Log("Msg #%d has bad backward reply link -> #%d (Fixed)",hdr.msgnum,hdr.prevmsg);
          hdr.prevmsg = 0;
         }

    if(hdr.msgnum < 1 || hdr.msgnum > 32000)
      {
       Log("Msg #%d has bad message number (Deleted)",hdr.msgnum);

       hdr.msgattr |= QMSG_DELETED;

       hdr.msgnum = 32000;
      }

    if(hdr.area < 1 || hdr.area > 200)
      {
       Log("Msg #%d has bad area number (Deleted)",hdr.msgnum);

       hdr.msgattr |= QMSG_DELETED;
      }

    if(hdr.msgattr & QMSG_DELETED) continue;

    if(do_kill)
      {
       char date[9];
       Date today(TODAY),d;

       hdr.nextmsg = 0;
       hdr.prevmsg = 0;

       memmove(date,hdr.postdate,9);

       pas2c(date);

       d[1] = atoi(strtok(date,"-"));
       d[0] = atoi(strtok(NULL,"-"));
       d[2] = atoi(strtok(NULL,"-"));

       int days = today - d;

       bool kill = FALSE;

       if(ma_max[hdr.area-1].msgKillDays)
         if(days > ma_max[hdr.area-1].msgKillDays) kill = TRUE;

       if(ma_max[hdr.area-1].rcvKillDays)
         if(days > ma_max[hdr.area-1].rcvKillDays && (hdr.msgattr & QMSG_RECEIVED)) kill = TRUE;

       if(ma_max[hdr.area-1].maxMsgs)
         if(minfo.active[hdr.area-1] > ma_max[hdr.area-1].maxMsgs) kill = TRUE;

       if(kill)
         {
          extra_killed++;
          minfo.active[hdr.area - 1]--;
          continue;
         }
      }

    {
     for(int i = prevmsg+1 ; i < hdr.msgnum ; i++) offset[i] = offset[i-1] + 1;
     offset[i] = offset[i-1];
     prevmsg = hdr.msgnum;
    }

    if(renumber)
      {
       hdr.msgnum  = msgnum++;
       hdr.nextmsg = hdr.prevmsg = 0;
      }

    fti.seek(256L*long(hdr.startrec));

    for(l=0;l<hdr.numrecs;l++)
      {
       fti.read(&line,sizeof(line));
       line.clean();
       fto.write(&line,sizeof(line));
      }

    hdr.startrec = rec;
    rec += hdr.numrecs;

    clean_header(&hdr);

    fho.write(&hdr,sizeof(hdr));
   }

 fhi.close();
 fti.close();

 fho.close();
 fto.close();

 unlink(FileName(cfg.msgpath,"MSGTXT.BAK"));
 unlink(FileName(cfg.msgpath,"MSGHDR.BAK"));

 fn(cfg.msgpath,"MSGHDR.BBS");
 fn2(cfg.msgpath,"MSGHDR.BAK");
 rename(fn,fn2);
 fn(cfg.msgpath,"MSGTXT.BBS");
 fn2(cfg.msgpath,"MSGTXT.BAK");
 rename(fn,fn2);
 fn(cfg.msgpath,"MSGHDR.$$$");
 fn2(cfg.msgpath,"MSGHDR.BBS");
 rename(fn,fn2);
 fn(cfg.msgpath,"MSGTXT.$$$");
 fn2(cfg.msgpath,"MSGTXT.BBS");
 rename(fn,fn2);

 if(killbak)
   {
    unlink(FileName(cfg.msgpath,"MSGTXT.BAK"));
    unlink(FileName(cfg.msgpath,"MSGHDR.BAK"));
   }

 printf("\b\b\b\b\bDone.\n\n");

 if(do_kill)
   if(extra_killed)
     Log("%d additional messages deleted",extra_killed);
    else
     Log("No additional messages deleted",extra_killed);

 if(renumber)
   {
    unsigned *lr = new unsigned[200];

    Log("Adjusting lastread-pointers");

    printf("Adjusting LASTREAD.BBS...");
    fn(cfg.msgpath,"LASTREAD.BBS");
    File f(fn,fmode_rw);
    if(!f.opened())
      {
       Log("Unable to open LASTREAD.BBS");
       printf("Can't open LASTREAD.BBS!\n\n");
       return;
      }
    for(i=0;;i++)
      {
       f.seek(long(i)*400);
       if(f.read(lr,400) != 400) break;
       for(int j=0;j<200;j++) lr[j] -= offset[lr[j]];
       f.seek(long(i)*400);
       f.write(lr,400);
      }
    f.close();
    printf("Done.\n\n");

    delete [] lr;

    RA2_UsersBBS *ubbs = new RA2_UsersBBS;

    printf("Adjusting USERS.BBS...");
    fn(cfg.msgpath,"USERS.BBS");
    if(!f.open(fn,fmode_rw))
      {
       Log("Unable to open USERS.BBS");
       printf("Can't open USERS.BBS!\n\n");
       return;
      }
    for(i=0;;i++)
      {
       f.seek(long(i)*sizeof(*ubbs));
       if(f.read(ubbs,sizeof(*ubbs)) != sizeof(*ubbs)) break;

       if(ubbs->LastRead > (unsigned)minfo.high) ubbs->LastRead = minfo.high;

       ubbs->LastRead -= offset[ ubbs->LastRead ];

       f.seek(long(i)*sizeof(*ubbs));
       f.write(ubbs,sizeof(*ubbs));
      }
    f.close();
    printf("Done.\n\n");

    delete ubbs;
   }

 delete offset;

 Log("Pack completed - Starting reindex");

 msgridx (0,NULL);

 if(do_kill || renumber)
   {
    printf("\nRun PBUTIL ML to recreate reply-links\n");
   }
}


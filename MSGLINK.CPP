#include <string.h>
#include <stdio.h>
#include "pbutil.hpp"

struct entry
  {
  unsigned char area;
  unsigned crc;
  int prev,next;
  };

class entry_array
  {
  int numchunks;
  entry *list[10];
 public:
  entry_array(int n)
     {
     numchunks=n/9000+1;
     for(int i=0;i<numchunks;i++)
        {
        list[i]=new entry[9000];
        memset(list[i],0,63000u);
        }
     }
  ~entry_array()
     {
     for(int i=0;i<numchunks;i++) delete [] list[i];
     }
  entry& operator[](int n)
     {
     n--;
     return list[n/9000][n%9000];
     }
  };

void
strip(char *s)
{
String x(s);

x.upperCase();

if(!strncmp(x,"RE:",3) || !strncmp(x,"(R)",3))
   {
   char *p=s+3;
   for(;*p;p++) if(*p!=' ') break;
   memmove(s,p,strlen(p)+1);
   }

int l=strlen(s);
for(int i=l-1;i>=0;i--) if(s[i]!=' ') { s[i+1]=0; break; }
if(i<0) *s=0;
}

void
msglink(int,char *[])
{
entry_array linklist(32000);
File f;
FileName fn(cfg.msgpath,"MSGHDR.BBS");
qbbs_msg msg;
unsigned *msgnum=new unsigned[32000];

Log("----------------------------------------------------");
Log("ML: Message link");

if(!f.open(fn,fmode_rw,4000))
  {
   Log("Unable to open MSGHDR.BBS");

   printf("Can't open MSGHDR.BBS\n");
   return;
  }

printf("Reading messages & cleaning subjects...");

for(int nummsg=0,highmsg=0;;nummsg++)
  {
  entry e;
  unsigned test;

  if(f.read(&msg,sizeof(msg))!=sizeof(msg)) break;

  msgnum[nummsg]=msg.msgnum;
  if(msg.msgnum>highmsg) highmsg=msg.msgnum;

  if(msg.msgattr & QMSG_DELETED) continue;

  e.area=msg.area;
  pas2c(msg.subj);
  test=upcrc(msg.subj);
  strip(msg.subj);
  e.crc=upcrc(msg.subj);
  e.next=e.prev=0;
  c2pas(msg.subj);

  linklist[msg.msgnum]=e;

  if(test!=e.crc)
    {
    f.seek(long(nummsg)*sizeof(msg));
    f.write(&msg,sizeof(msg));
    f.seek(long(nummsg+1)*sizeof(msg));
    }
  }

printf("Done.\n\nLinking %d messages...0    ",highmsg);

for(int i=1;i<=highmsg;i++)
  {
  entry *e=&linklist[i];

  if(!(i%10)) printf("\b\b\b\b\b%-5d",i);

  if(!e->area && !e->crc) continue;
  if(e->next || e->prev) continue;

  int m=i;

  for(int j=i+1;j<=highmsg;j++)
    {
    entry *f=&linklist[j];

    if(f->next || f->prev || !f->area) continue;
    if(e->crc==f->crc && e->area==f->area)
      {
       e->next=j;
       f->prev=m;
       m=j;
       e=f;
      }
    }
  }

Log("%d messages linked",nummsg);

printf("\b\b\b\b\bDone.\n\nRewriting messagebase...");

f.rewind();

for(i=0;i<nummsg;i++)
  {
   f.seek(long(i)*sizeof(msg)+2);
   f.write(&linklist[msgnum[i]].prev,4);
  }

printf("Done.\n");
}


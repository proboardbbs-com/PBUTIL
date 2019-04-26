#include <io.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <sys\stat.h>
#include "pbutil.hpp"

void
userkill(int argc,char *argv[])
{
RA2_UsersBBS ubbs;
int  killdays  = 0;
int  killcalls = 0;
word killlevel = 0;

if(argc<1) return;

for(int i=0;i<argc;i++)
  if(argv[i][0]=='-')
    switch(toupper(argv[i][1]))
      {
      case 'D': killdays = atoi(&argv[i][2]);
                break;
      case 'C': killcalls=atoi(&argv[i][2]);
                break;
      case 'L': killlevel=atoi(&argv[i][2]);
                break;
      }
    else if(argv[i][0]=='?')
           {
           printf("User Killer Options\n"
                  "컴컴컴컴컴컴컴컴컴�\n\n"
                  "   [-C<n>]     Delete if # calls < n\n"
                  "   [-D<d>]     Delete if not called for <d> days\n"
                  "   [-L<level>] Only work on records with this level\n\n"
                  " These options can be combined (see manual)\n\n");
           return;
           }

 Log("----------------------------------------------------");
 Log("UK: User Delete");

 File f;

 if(!f.open(FileName(cfg.msgpath,"USERS.BBS"),fmode_rw,4096))
   {
    Log("Unable to open USERS.BBS");
    printf("Can't open USERS.BBS\n");
    return;
   }

printf("Cleaning up user file...");

for(int n=0,del=0;;n++)
  {
  if(f.read(&ubbs,sizeof(ubbs))!=sizeof(ubbs)) break;

  if(killlevel && ubbs.Security != killlevel) continue;

  if(ubbs.Attribute & RA_UFLAG_DELETED) continue;

  Date date,today(TODAY);

  pas2c(ubbs.LastDate);
  String datestr(ubbs.LastDate);
  c2pas(ubbs.LastDate);

  date[1] = atoi(strtok(datestr,"-"));
  date[0] = atoi(strtok(NULL,"-"));
  date[2] = atoi(strtok(NULL,"-"));

  if(killdays && date.ok())
    {
     if(killcalls)
       {
        if((today-date)>killdays && ubbs.NoCalls<killcalls)
          {
           ubbs.Attribute |= RA_UFLAG_DELETED;
          }
       }
      else
       {
        if((today-date)>killdays) ubbs.Attribute |= RA_UFLAG_DELETED;
       }
    }

  if(ubbs.NoCalls<killcalls && !killdays) ubbs.Attribute |= RA_UFLAG_DELETED;

  if((ubbs.Attribute & RA_UFLAG_DELETED) && !(ubbs.Attribute & RA_UFLAG_NOKILL))
    {
     f.seek(long(n)*sizeof(ubbs));
     f.write(&ubbs,sizeof(ubbs));
     f.seek(long(n+1)*sizeof(ubbs));
     del++;

     pas2c(ubbs.Name);

     Log("User record %d deleted (%s)",n+1,ubbs.Name);
    }
  }

 Log("Done - %d users deleted",del);

 printf(" %d users deleted\n",del);
}



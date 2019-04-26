#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "pbutil.hpp"

static void gentops(bool aliases = FALSE);
static void clean_tagfile();

void maintenance(int c,char *v[])
{
   bool aliases = FALSE;

   Log("컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴");
   Log("DM - Daily maintenance");

   for(int i=0;i<c;i++)
   {
      if(toupper(v[i][1]) == 'H')
         aliases = TRUE;

      if(v[i][0] == '?')
      {
         printf("Daily Maintenace options\n"
                "컴컴컴컴컴컴컴컴컴컴컴컴\n\n"
                "   [-H]  Write users' aliases in TOPS.PB\n\n");
         return;
      }

   }

   gentops(aliases);
   clean_tagfile();
}

struct topentry
   {
      char name[36];
      dword n;
   };

static void near
insert_top(topentry *tp,char *name,dword n)
{
   if(n<tp[19].n) return;

   for(int i=0;i<20;i++)
      if(n>tp[i].n)
      {
         for(int j=18;j>=i;j--)
            tp[j+1] = tp[j];

         strcpy(tp[i].name,name);

         tp[i].n = n;
         break;
      }
}

static void
gentops(bool aliases)
{
   File f_users,
        f_userspb;

   Log("Creating TOPS.PB");

   if(!f_users  .open(FileName(cfg.msgpath,"USERS.BBS")  ,fmode_read,30000))
   {
      Log("Unable to open USERS.BBS");
      printf("Can't open userfile!\n");
      return;
   }

   if(!f_userspb.open(FileName(cfg.msgpath,"USERSPB.BBS")  ,fmode_read,30000))
   {
      Log("Unable to open USERSPB.BBS");
      printf("Can't open userfile!\n");
      return;
   }

   printf("Creating TOPS.PB...");

   topentry *top_callers    = new topentry[20];
   topentry *top_downk      = new topentry[20];
   topentry *top_downtimes  = new topentry[20];
   topentry *top_upk        = new topentry[20];
   topentry *top_uptimes    = new topentry[20];
   topentry *top_msgwriters = new topentry[20];
   topentry *top_online     = new topentry[20];

   memset(top_callers    ,0, sizeof(topentry)*20);
   memset(top_downk      ,0, sizeof(topentry)*20);
   memset(top_downtimes  ,0, sizeof(topentry)*20);
   memset(top_upk        ,0, sizeof(topentry)*20);
   memset(top_uptimes    ,0, sizeof(topentry)*20);
   memset(top_msgwriters ,0, sizeof(topentry)*20);
   memset(top_online     ,0, sizeof(topentry)*20);

   for(int n=0;;n++)
   {
      RA2_UsersBBS   u;
      RA2_UsersPbBBS upb;

      if(f_users.read(&u,sizeof(u)) != sizeof(u)) break;
      if(f_userspb.read(&upb,sizeof(upb)) != sizeof(upb)) break;

      if(u.Attribute & (RA_UFLAG_DELETED)) continue;
      if(upb.uFlags  & (RA_UFLAG3_NOTOPS)) continue;

      pas2c(u.Name);
      pas2c(u.Handle);

      if(u.Handle[0] == '\0')
         strcpy(u.Handle,u.Name);

      insert_top(top_callers   , (aliases ? u.Handle:u.Name) , dword(u.NoCalls));
      insert_top(top_downk     , (aliases ? u.Handle:u.Name) , dword(u.DownloadsK));
      insert_top(top_downtimes , (aliases ? u.Handle:u.Name) , dword(u.Downloads));
      insert_top(top_upk       , (aliases ? u.Handle:u.Name) , dword(u.UploadsK));
      insert_top(top_uptimes   , (aliases ? u.Handle:u.Name) , dword(u.Uploads));
      insert_top(top_msgwriters, (aliases ? u.Handle:u.Name) , dword(u.MsgsPosted));
      insert_top(top_online    , (aliases ? u.Handle:u.Name) , dword(upb.totalTimeUsed));
   }

   File f(FileName(syspath,"TOPS.PB"),fmode_create);

   f.write(top_callers,sizeof(topentry)*20);
   f.write(top_downk,sizeof(topentry)*20);
   f.write(top_downtimes,sizeof(topentry)*20);
   f.write(top_upk,sizeof(topentry)*20);
   f.write(top_uptimes,sizeof(topentry)*20);
   f.write(top_msgwriters,sizeof(topentry)*20);
   f.write(top_online,sizeof(topentry)*20);

   delete [] top_callers;
   delete [] top_downk;
   delete [] top_downtimes;
   delete [] top_upk;
   delete [] top_uptimes;
   delete [] top_msgwriters;
   delete [] top_online;

   printf("Done\n");
}

static
void
clean_tagfile()
{
   FileName fn1(syspath,"TAGLIST.PB");
   FileName fn2(syspath,"TAGLIST.$$$");

   File fi,fo;

   Log("Cleaning up TAGLIST.PB");

   if(fi.open(fn1,fmode_read))
      if(fo.open(fn2,fmode_create))
      {
         printf("Cleaning up TAGLIST.PB...");

         for(;;)
         {
            char name[36];
            word n;

            if(fi.read(name,36) != 36)
               break;

            fi >> n;

            if(!name[0])
            {
               fi.seek(n * sizeof(FilesIdx),seek_cur);
               continue;
            }


            word bufsize = n * sizeof(FilesIdx);

            byte *buf = new byte[bufsize];

            if(fi.read(buf,bufsize) != bufsize)
               break;

            fo.write(name,36);
            fo << n;
            fo.write(buf,bufsize);

            delete [] buf;
         }

         fi.close();
         fo.close();

         unlink(fn1);
         rename(fn2,fn1);

         printf("Done\n");
      }
}

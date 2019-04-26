#include <stdio.h>
#include <string.h>
#include "pbutil.hpp"

void
userfix(int,char *[])
{
 File f_users,f_userspb,f_usersxi,
      fo_userspb,fo_usersxi,fo_users;

 Log("----------------------------------------------------");
 Log("UF: User fix");

 if(!f_users.open(FileName(cfg.msgpath,"USERS.BBS"),fmode_read,30000) < 0)
   {
    Log("Unable to open USERS.BBS");
    printf("Can't open userfile!\n");
    return;
   }

 f_userspb.open(FileName(cfg.msgpath,"USERSPB.BBS"),fmode_read,30000);
 f_usersxi.open(FileName(cfg.msgpath,"USERSXI.BBS"),fmode_read,30000);

if(   !fo_userspb.open(FileName(cfg.msgpath,"USERSPB.$$$"),fmode_create,5000)
   || !fo_usersxi.open(FileName(cfg.msgpath,"USERSXI.$$$"),fmode_create,5000)
   || !fo_users  .open(FileName(cfg.msgpath,"USERS.$$$")  ,fmode_create,5000))
  {
   Log("Unable to create output files");
   printf("Can't create output files\n");
   return;
  }

 printf("Fixing userfile...");

 RA2_UsersBBS   *ubbs   = new RA2_UsersBBS;
 RA2_UsersPbBBS *upbbbs = new RA2_UsersPbBBS;
 RA2_UsersXiBBS *uxibbs = new RA2_UsersXiBBS;

 for(int rec=0;;rec++)
   {
    bool found = FALSE;

    if(f_users.read(ubbs,sizeof(*ubbs)) != sizeof(*ubbs)) break;

    pas2c(ubbs->Name);

    if(f_userspb.opened())
      {
       f_userspb.seek(long(rec) * sizeof(*upbbbs));
       f_userspb.read(upbbbs,sizeof(*upbbbs));

       if(!strcmpl(ubbs->Name,upbbbs->name)) found = TRUE;

       if(!found)
         {
          f_userspb.rewind();

          for(;;)
            {
             if(f_userspb.read(upbbbs,sizeof(*upbbbs)) != sizeof(*upbbbs)) break;

             if(!strcmpl(ubbs->Name,upbbbs->name))
               {
                found = TRUE;
                break;
               }
            }
         }
      }

    if(!found)
      {
       CLEAR_OBJECT(*upbbbs);
       strcpy(upbbbs->name,ubbs->Name);
       upbbbs->logLevel = 1;
       memset(&upbbbs->mailCheckBoards,0xFF,125);
      }

//    for(int i=1;i<=200;i++) if(upbbbs->mailcheckboards.connected(i)) break;

//    if(i>200) memset(&upbbbs->mailcheckboards,0xFF,25);

    found = FALSE;

    if(f_usersxi.opened())
      {
       f_usersxi.seek(long(ubbs->XIrecord) * sizeof(*uxibbs));
       if(f_usersxi.read(uxibbs,sizeof(*uxibbs)) == sizeof(*uxibbs)) found = TRUE;
      }

    if(!found)
      {
       CLEAR_OBJECT(*uxibbs);
      }

    ubbs->XIrecord = rec;
    c2pas(ubbs->Name);

    fo_users  .write(ubbs  ,sizeof(*ubbs));
    fo_userspb.write(upbbbs,sizeof(*upbbbs));
    fo_usersxi.write(uxibbs,sizeof(*uxibbs));
   }

 Log("Userfile fixed - Running reindex");

 printf("Userfile fixed\n\n");

 FileName fn1,fn2;

 f_users.close();
 f_userspb.close();
 f_usersxi.close();
 fo_users.close();
 fo_userspb.close();
 fo_usersxi.close();

 fn1(cfg.msgpath,"USERSPB.BAK");
 fn2(cfg.msgpath,"USERSPB.BBS");
 unlink(fn1);
 rename(fn2,fn1);
 fn1(cfg.msgpath,"USERSPB.$$$");
 rename(fn1,fn2);

 fn1(cfg.msgpath,"USERS.BAK");
 fn2(cfg.msgpath,"USERS.BBS");
 unlink(fn1);
 rename(fn2,fn1);
 fn1(cfg.msgpath,"USERS.$$$");
 rename(fn1,fn2);

 fn1(cfg.msgpath,"USERSXI.BAK");
 fn2(cfg.msgpath,"USERSXI.BBS");
 unlink(fn1);
 rename(fn2,fn1);
 fn1(cfg.msgpath,"USERSXI.$$$");
 rename(fn1,fn2);

 useridx(0,NULL);
}

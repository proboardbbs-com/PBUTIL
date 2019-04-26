#include <stdio.h>
#include "pbutil.hpp"

void
useridx(int,char *[])
{
File f_users,
     fo;

Log("----------------------------------------------------");
Log("UI: Userfile reindex");

if(!f_users  .open(FileName(cfg.msgpath,"USERS.BBS")  ,fmode_read,30000))
   {
    Log("Unable to open USERS.BBS");
    printf("Can't open userfile!\n");
    return;
   }

if(!fo.open(FileName(cfg.msgpath,"USERSIDX.BBS"),fmode_create,2000))
  {
   Log("Unable to create USERSIDX.BBS");

   printf("Can't create USERSIDX.BBS\n");
   return;
  }

printf("Reindexing userfile...");

for(int n=0;;n++)
  {
   RA2_UsersBBS   u;

   if(f_users.read(&u,sizeof(u)) != sizeof(u)) break;

   pas2c(u.Name);
   pas2c(u.Handle);

   dword crc1 = RaCrc(u.Name);
   dword crc2 = RaCrc(u.Handle);

   fo.write(&crc1,4);
   fo.write(&crc2,4);
  }

Log("%d user records reindexed",n);
printf(" %d user records indexed.\n",n);
}


#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "pbutil.hpp"

static void pack_squish(BitArray&);

void
userpack(int argc,char *argv[])
{
bool reset = FALSE;
bool killbak = FALSE;

File f_users,
     f_usersxi,
     f_userspb,
     f_lastread,
     fo_users,
     fo_usersxi,
     fo_userspb,
     fo_lastread;

 for(int i=0;i<argc;i++)
   {
    if(argv[i][0] == '?')
      {
       printf("Userfile packer options\n"
              "컴컴컴컴컴컴컴컴컴컴컴�\n\n"
              "   [-R]  Reset LASTREAD pointers to 0\n"
              "   [-K]  Kill .BAK files after pack\n\n"
              " These options can be combined.\n\n");
       return;
      }
    if(argv[i][0]=='-')
      switch(toupper(argv[i][1]))
        {
         case 'R': reset = TRUE;
                   break;
         case 'K': killbak = TRUE;
                   break;
        }
   }

 Log("----------------------------------------------------");
 Log("UP: Userfile pack");

 if(   !f_users   .open(FileName(cfg.msgpath,"USERS.BBS"   ),fmode_read,10000)
    || !f_usersxi .open(FileName(cfg.msgpath,"USERSXI.BBS" ),fmode_read,10000)
    || !f_userspb .open(FileName(cfg.msgpath,"USERSPB.BBS" ),fmode_read,10000))
     {
      printf("Can't open userfile!\n");
      return;
     }

 f_lastread.open(FileName(cfg.msgpath,"LASTREAD.BBS"),fmode_read,10000);

 if(   !fo_users   .open(FileName(cfg.msgpath,"USERS.$$$"   ),fmode_create,10000)
    || !fo_usersxi .open(FileName(cfg.msgpath,"USERSXI.$$$" ),fmode_create,10000)
    || !fo_userspb .open(FileName(cfg.msgpath,"USERSPB.$$$" ),fmode_create,10000)
    || !fo_lastread.open(FileName(cfg.msgpath,"LASTREAD.$$$"),fmode_create,10000))
     {
      Log("Unable to create new user files");
      printf("Can't create new user file!\n");
      return;
     }

 printf("Packing user file...");
 if(reset) printf("(RESET)...");

 RA2_UsersBBS       *ra_u   = new RA2_UsersBBS;
 RA2_UsersXiBBS     *ra_uxi = new RA2_UsersXiBBS;
 RA2_UsersPbBBS     *ra_upb = new RA2_UsersPbBBS;
 word               *ra_lr  = new word[200];

 BitArray users_deleted(60000L);

 for(int del=0,rec=0,dest_rec=0;;rec++)
   {
    f_users.seek(long(rec) * sizeof(*ra_u));

    if(f_users.read(ra_u,sizeof(*ra_u)) != sizeof(*ra_u)) break;

    if(!isalpha(ra_u->Name[1]) || ra_u->Name[0]>35 || ra_u->Name[0]<3)
      {
       del++;
       continue;
      }

    if((ra_u->Attribute & RA_UFLAG_DELETED) && !(ra_u->Attribute & RA_UFLAG_NOKILL))
      {
       del++;
       continue;
      }


    users_deleted.set(rec);

    f_usersxi.seek(long(ra_u->XIrecord) * sizeof(*ra_uxi));
    f_userspb.seek(long(rec) * sizeof(*ra_upb));

    if(   f_usersxi .read(ra_uxi,sizeof(*ra_uxi)) != sizeof(*ra_uxi)
       || f_userspb .read(ra_upb,sizeof(*ra_upb)) != sizeof(*ra_upb) )
      {
       Log("User file damaged (XIrecord = %ld)",ra_u->XIrecord);
       printf("Userfile damaged!\n");
       killbak = FALSE;
       break;
      }

    ra_u->XIrecord = dest_rec;

    memset(ra_lr,0,400);

    if(f_lastread.opened())
      {
       f_lastread.seek(long(rec) * 400);
       f_lastread.read(ra_lr,400);
      }

    if(reset)
      {
       for(int i=0;i<200;i++) ra_lr[i] = 0;
      }

    fo_users   .write(ra_u  ,sizeof(*ra_u));
    fo_usersxi .write(ra_uxi,sizeof(*ra_uxi));
    fo_userspb .write(ra_upb,sizeof(*ra_upb));
    fo_lastread.write(ra_lr,400);

    if(   fo_users   .error()
       || fo_usersxi .error()
       || fo_userspb .error()
       || fo_lastread.error()
      )
    {
      Log("Error writing to output files! (disk full?)");
      printf("Error writing to output files! (disk full?)\n");
      delete ra_u;
      delete ra_uxi;
      delete ra_upb;
      delete [] ra_lr;
      return;
    }

    dest_rec++;
   }

 f_users.close();
 f_usersxi.close();
 f_userspb.close();
 f_lastread.close();
 fo_users.close();
 fo_usersxi.close();
 fo_userspb.close();
 fo_lastread.close();

 delete ra_u;
 delete ra_uxi;
 delete ra_upb;
 delete [] ra_lr;


 FileName fn1,fn2;

 fn1(cfg.msgpath,"USERS.BAK");
 fn2(cfg.msgpath,"USERS.BBS");
 unlink(fn1);
 if(killbak) unlink(fn2);
        else rename(fn2,fn1);
 fn1(cfg.msgpath,"USERS.$$$");
 rename(fn1,fn2);

 fn1(cfg.msgpath,"USERSXI.BAK");
 fn2(cfg.msgpath,"USERSXI.BBS");
 unlink(fn1);
 if(killbak) unlink(fn2);
        else rename(fn2,fn1);
 fn1(cfg.msgpath,"USERSXI.$$$");
 rename(fn1,fn2);

 fn1(cfg.msgpath,"USERSPB.BAK");
 fn2(cfg.msgpath,"USERSPB.BBS");
 unlink(fn1);
 if(killbak) unlink(fn2);
        else rename(fn2,fn1);
 fn1(cfg.msgpath,"USERSPB.$$$");
 rename(fn1,fn2);

 fn1(cfg.msgpath,"LASTREAD.BAK");
 fn2(cfg.msgpath,"LASTREAD.BBS");
 unlink(fn1);
 if(killbak) unlink(fn2);
        else rename(fn2,fn1);
 fn1(cfg.msgpath,"LASTREAD.$$$");
 rename(fn1,fn2);

 if(!del) printf(" No");
     else printf(" %d",del);

 printf(" users deleted\n\n");

 if(del) Log("%d deleted users removed",del);

 pack_squish(users_deleted);

 useridx(0,NULL);
}


static void
pack_squish(BitArray& users_deleted)
{
 msgarea ma;
 File f_msgareas;
 bool found_one = FALSE;

 if(!f_msgareas.open(FileName(syspath,"MESSAGES.PB")))
   {
    Log("Unable to open MESSAGES.PB");

    return;
   }

 for(int area = 0 ;; area++)
   {
    if(f_msgareas.read(&ma,sizeof(ma)) != sizeof(ma)) break;

    if(ma.msgBaseType != MSGBASE_SQUISH) continue;

    if(!found_one)
      {
       printf("Packing Squish lastread pointers...%-30s","");

       found_one = TRUE;
      }

    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%-30s",ma.name);

    FileName fni(ma.path),
             fno(ma.path);

    fni.changeExt("SQL");
    fno.changeExt("$$$");

    File fi(fni , fmode_read   , 1024);
    File fo(fno , fmode_create , 1024);

    for(long rec = 0 , del_rec = 0 ; ; rec++)
      {
       long n;

       if(fi.read(&n,4) != 4) break;

       if(!users_deleted[rec]) continue;

       fo << n;

       del_rec++;
      }

    fi.close();
    fo.close();

    unlink(fni);
    rename(fno,fni);

    Log("Packed file %s",(char *)fni);
   }

 if(found_one) printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%-30s\n\n","Done.");
}

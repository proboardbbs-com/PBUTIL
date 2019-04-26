#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "pbutil.hpp"

struct username
  {
  char name[36];
  word level;
  int pos;
  };

class user_array
  {
  username *x[10];
  int num_chunks;
 public:
  user_array(int size)
    {
    num_chunks=size/1500+1;
    for(int i=0;i<num_chunks;i++) x[i]=new username[1500];
    }
  ~user_array()
    {
    for(int i=0;i<num_chunks;i++) delete [] x[i];
    }
  username& operator[](int i)
    {
    if(num_chunks>1) return x[i/1500][i%1500];
                else return x[0][i];
    }
  void swap(int i,int j)
    {
    username tmp=(*this)[i];
    (*this)[i]=(*this)[j];
    (*this)[j]=tmp;
    }
  };

void sort_squish(user_array& un , int n);

void
usersort(int,char *[])
{
   bool killbak = FALSE;

   File f_users,
        f_usersxi,
        f_userspb,
        f_lastread,
        fo_users,
        fo_usersxi,
        fo_userspb,
        fo_lastread;

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


    RA2_UsersBBS   *ra_u   = new RA2_UsersBBS;
    RA2_UsersXiBBS *ra_uxi = new RA2_UsersXiBBS;
    RA2_UsersPbBBS *ra_upb = new RA2_UsersPbBBS;
    word           *ra_lr  = new word[200];

   int n = int(f_users.len() / sizeof(RA2_UsersBBS));

   user_array un(n+1);

   for(n=0;;n++)
   {
      if(f_users.read(ra_u,sizeof(*ra_u)) != sizeof(*ra_u)) break;
      pas2c(ra_u->Name);
      strcpy(un[n].name,ra_u->Name);
      un[n].level = ra_u->Security;
      un[n].pos   = n;
   }

   printf("Sorting userfile...   0");

   for(int i=0;i<n-1;i++)
   {
      for(int j=i+1;j<n;j++)
      {
         if(un[i].level==un[j].level && strcmpl(un[i].name,un[j].name)>0) un.swap(i,j);
         if(un[i].level<un[j].level) un.swap(i,j);
      }

      if(!(i%5))
      {
         printf("\b\b\b\b%4d",i);
      }
   }

   printf("\b\b\b\b%4d users sorted.\n",n);
   printf("\nWriting user file...   0");

   bool error = FALSE;
   long dest_rec = 0;

   for(i=0;i<n;i++,dest_rec++)
   {
      f_users.seek(long(un[i].pos)*sizeof(*ra_u));
      f_users.read(ra_u,sizeof(*ra_u));

      f_usersxi.seek(long(ra_u->XIrecord) * sizeof(*ra_uxi));
      f_userspb.seek(long(un[i].pos) * sizeof(*ra_upb));

      if(   f_usersxi .read(ra_uxi,sizeof(*ra_uxi)) != sizeof(*ra_uxi)
         || f_userspb .read(ra_upb,sizeof(*ra_upb)) != sizeof(*ra_upb) )
        {
         Log("User file damaged");
         printf("Userfile damaged!\n");
         error = TRUE;
         break;
        }

      ra_u->XIrecord = dest_rec;

      memset(ra_lr,0,400);

      f_lastread.seek(long(un[i].pos) * 400);
      f_lastread.read(ra_lr,400);

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

         error = TRUE;
         break;
      }

      if(!(i%5)) printf("\b\b\b\b%4d",i);
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

 if(error)
   return;

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

 printf("\b\b\b\bDone.\n\n");

 sort_squish(un,n);

 useridx(0,NULL);
}

static void
sort_squish(user_array& un , int n)
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
       printf("Sorting Squish lastread pointers...%-30s","");

       found_one = TRUE;
      }

    printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%-30s",ma.name);

    FileName fni(ma.path),
             fno(ma.path);

    fni.changeExt("SQL");
    fno.changeExt("$$$");

    File fi(fni , fmode_read   , 1024);
    File fo(fno , fmode_create , 1024);

    for(int rec = 0 ; rec < n ; rec++)
      {
       long n;

       fi.seek(long(un[rec].pos) * 4L);
       if(fi.read(&n,4) != 4)
         fo << long(0);
       else
         fo << n;
      }

    fi.close();
    fo.close();

    unlink(fni);
    rename(fno,fni);

    Log("Sorted file %s",(char *)fni);
   }

 if(found_one) printf("\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b%-30s\n\n","Done.");
}


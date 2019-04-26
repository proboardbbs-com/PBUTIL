#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "pbutil.hpp"

static char *valid="ABCDEFGHIJKLMNOPQRSTUVWXYZ_1234567890#$";
static void make_topfiles();

static char *months[]={ "","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec" };

inline bool
is_whitespace(char c)
{
   return (c==' ' || c=='\t');
}

static int
DescStart( char *str )
{
   char *s = str;
   char *prev_s;
   String token;

   while(!is_whitespace(*s) && *s)
      s++;

   prev_s = s;

   while(*s)
   {
      token.clear();

      prev_s = s;

      while(is_whitespace(*s) && *s) s++;                   // Skip whitespace
      while(!is_whitespace(*s) && *s) token << (*s++);

      bool non_num_found  = FALSE;

      for(int j=0;token[j];j++)
      {
         char c = token[j];

         if(!strchr("/-.",c) && !isdigit(c))
            non_num_found = TRUE;
      }

      if(non_num_found)
         break;

      if(!*s)
         prev_s = s;
   }

   s = prev_s;

   while(is_whitespace(*s) && *s) s++;                   // Skip whitespace

   return int(s-str);
}


static bool
update(char *listfile,char *filename)
{
char orgfile[80];
File tf,temp;
bool found = FALSE;

strcpy(orgfile,listfile);
if(!tf.open(listfile,fmode_read | fmode_text,8192))
{
   Log("Unable to open file '%s'",(char *)listfile);
   return FALSE;
}

strcpy(&orgfile[strlen(orgfile)-3],"TMP");

if(!temp.open(orgfile,fmode_create | fmode_text,8192)) return FALSE;

char line[501];
while(tf.readLine(line,500))
   {
   if(strchr(valid,toupper (*line)))
      {
      int count=0;
      char name[50]="";
      char name_date_size[50]="";

      int desc_start = DescStart(line);

      strncpy(name_date_size,line,desc_start);
      name_date_size[desc_start] = '\0';

      for(int i=0;name_date_size[i];i++)
         if(is_whitespace(name_date_size[i]))
            break;

      strncpy(name,name_date_size,i);
      name[i] = '\0';

      i = desc_start;

      for(;line[i]==' ';i++) {}
      if(line[i]=='[')
        {
         char *ptr=&line[i+1];
         for(;line[i]!=']' && line[i];i++) {}
         line[i++]='\0';
         count=atoi(ptr);
         for(;line[i]==' ';i++){}
        }

      if(!strcmpl(name,filename))
        {
         found = TRUE;
         count++;
        }
      temp << form("%s[%02d] %s",name_date_size,count,(char *)&line[i]);
      }
     else
      {
       temp << line;
      }
   }

tf.close();
temp.close();

unlink(listfile);
rename(orgfile,listfile);

return found;
}

static int numtops=10;

void
fc(int argc,char *argv[])
{
File fah;
File downlog;
filearea fa;
char string[80];
FileName fn;
int force=0,rewrite=0;

for(int i=0;i<argc;i++)
  if(argv[i][0]=='-' || argv[i][0]=='/')
    switch(toupper(argv[i][1]))
      {
      case 'N': numtops=atoi(&argv[i][2]);
                break;
      case 'F': force=1;
                break;
      case 'R': rewrite=1;
                break;
      }
   else if(argv[i][0]=='?')
           {
           printf("File Counters Options\n"
                  "ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n\n"
                  "   [-N<x>]   Write Top x (default=10)\n"
                  "   [-F]      Force writing of Top xx\n"
                  "   [-R]      Rewrite all file areas\n\n"
                  " These options can be combined.\n");
           return;
           }

Log("----------------------------------------------------");
Log("FC: File counters update");

printf("File counters update\n"
       "ÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ\n\n");

fn(syspath,"DOWNLOAD.LOG");
downlog.open(fn,fmode_read | fmode_text);

fn(syspath,"FILECFG.PRO");
if(!fah.open(fn,fmode_read,8192))
  {
   Log("Unable to open FILECFG.PRO");

   printf("Can't open FILECFG.PRO\n");
   return;
  }

int files=0;

   for(;;)
   {
      int area = 0;

      if(!downlog.opened()) break;
      if(!downlog.readLine(string,79)) break;

      strip_linefeed(string);

      FileName path,name;

      if(isdigit(string[0]))
      {
         area = atoi(string);
         path = &string[6];
      }
      else
      {
         path = string;
      }

      name = path;

      path.stripName();
      name.stripPath();

      if(area)
         fah.seek((long(area)-1) * sizeof(fa));
      else
         fah.seek(0);

      for(;;)
      {
         if(fah.read(&fa,sizeof(fa))!=sizeof(fa)) break;

         if(fa.name[0] == '\0') continue;

         append_backspace(fa.filepath);

         if(area || !strcmpl(path,fa.filepath))
         {
            if(update(fa.listpath,name))
            {
               Log("File '%s' (%s) updated",(char *)name,fa.name);
               printf("Updating %-12s\r",(char *)name);
               files++;
               break;
            }

            if(area)
               break;
         }
      }
   }

fah.seek(0L);
if(rewrite)
 for(;;)
  {
  if(fah.read(&fa,sizeof(fa))!=sizeof(fa)) break;

  if(fa.name[0] == '\0') continue;

  Log("Rewriting area %s",fa.name);
  printf("Rewriting area %-40s\r",fa.name);
  update(fa.listpath,"*********");
  }

fah.close();
downlog.close();

fn(syspath,"DOWNLOAD.LOG");
unlink(fn);

if(!files && !force) return;

Log("%d file counters updated",files);

printf("%d files updated.                                       \n\n",files);

Log("Creating files top %d",numtops);

printf("Writing top %d...",numtops);
make_topfiles();
printf("Done.\n");
}

static
 struct top
  {
   char name[13];
   char omschr[80];
   int n;
  } *tp;

static void addit(char *,int);

static void
make_topfiles()
{
int i;
File f;
FileName fn;

tp=new top[numtops+1];
memset(tp,0,numtops*sizeof(top));

fn(syspath,"FILECFG.PRO");
if(!f.open(fn,fmode_read,4096))
  {
   Log("Unable to open FILECFG.PRO");

   printf("Can't open FILECFG.PRO\n");
   return;
  }

long totaldl = 0;

for(;;)
   {
   filearea fa;
   File tf;

   if(f.read(&fa,sizeof(fa))!=sizeof(fa)) break;

   if(fa.name[0] == '\0' || fa.notops) continue;

   if(!tf.open(fa.listpath,fmode_read | fmode_text,8192)) continue;

   char *str = new char[1024];

   for(;;)
     {
     if(!tf.readLine(str,1023)) break;
     strip_linefeed(str);
     if(!strchr(valid,toupper(str[0]))) continue;
     for(int i=0;str[i]!='[' && str[i];i++) {}
     if(str[i]=='[')
       {
        int num;

        sscanf(&(str[i+1]),"%d",&num);
        addit(str,num);
        totaldl += num;
       }
     }

     delete [] str;
   }
f.close();

Log("Total # downloads = %ld",totaldl);

fn(cfg.txtpath,"TOPFILES.ANS");

Date date(TODAY);
Time timenow(NOW);

File fp(fn,fmode_create | fmode_text);

fp.printf("[2J[1;37mÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\n");
fp.printf("[1;37m³[36m Top %2d files [37m³[36m    Last updated on [33m%2d %s %4d  at  %02d:%02d[33m                   [37m³\n",numtops,date[0],months[date[1]],1900+date[2],timenow[0],timenow[1]);
fp.printf("[1;37mÃÄÄÄÄÂÄÄÄÄÄÄÄÄÄÁÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´\n");
fp.printf("[1;37m³[33m Nr [37m³[33m  File        [37m³[33m #DL [37m³[33m  Description                                     [37m³[40m\n");
fp.printf("[1;37mÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´\n");

for(i=0;i<numtops;i++)
   fp.printf("[1;37m³[31m %2d [37m³[36m %-12s [37m³%4d ³[36m %-48.48s [37m³\n",i+1,tp[i].name,tp[i].n,tp[i].omschr);

fp.printf("[1;37mÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n");
fp.printf("\nPress [33m[Enter][37m to continue\x01[0m");

fp.close();

fn(cfg.txtpath,"TOPFILES.AVT");

fp.open(fn,fmode_create | fmode_text);

fp.printf("\f\x0FÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\n");
fp.printf("\x0F³\x0B Top %2d files \x0F³\x0B    Last updated on \x0E%2d %s %4d  at  %02d:%02d                   \x0F³\n",numtops,date[0],months[date[1]],1900+date[2],timenow[0],timenow[1]);
fp.printf("\x0FÃÄÄÄÄÂÄÄÄÄÄÄÄÄÄÁÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´\n");
fp.printf("\x0F³\x0E Nr \x0F³\x0E  File        \x0F³\x0E #DL \x0F³\x0E  Description                                     \x0F³\n");
fp.printf("\x0FÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´\n");

for(i=0;i<numtops;i++)
   fp.printf("\x0F³\x0C %2d \x0F³\x0B %-12s \x0F³%4d ³\x0B %-48.48s \x0F³\n",i+1,tp[i].name,tp[i].n,tp[i].omschr);

fp.printf("\x0FÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n");
fp.printf("\nPress \x0E[Enter]\x0F to continue\x01\x07");

fp.close();

fn(cfg.txtpath,"TOPFILES.AVP");

fp.open(fn,fmode_create | fmode_text);

fp.printf("\f\x0FÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\n");
fp.printf("\x0F³\x0B Top %2d files \x0F³\x0B    Last updated on \x0E%2d %s %4d  at  %02d:%02d                   \x0F³\n",numtops,date[0],months[date[1]],1900+date[2],timenow[0],timenow[1]);
fp.printf("\x0FÃÄÄÄÄÂÄÄÄÄÄÄÄÄÄÁÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´\n");
fp.printf("\x0F³\x0E Nr \x0F³\x0E  File        \x0F³\x0E #DL \x0F³\x0E  Description                                     \x0F³\n");
fp.printf("\x0FÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´\n");

for(i=0;i<numtops;i++)
   fp.printf("\x0F³\x0C %2d \x0F³\x0B %-12s \x0F³%4d ³\x0B %-48.48s \x0F³\n",i+1,tp[i].name,tp[i].n,tp[i].omschr);

fp.printf("\x0FÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n");
fp.printf("\nPress \x0E[Enter]\x0F to continue\x01\x07");

fp.close();


fn(cfg.txtpath,"TOPFILES.ASC");

fp.open(fn,fmode_create | fmode_text);

fp.printf("ÚÄÄÄÄÄÄÄÄÄÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ¿\n");
fp.printf("³ Top %2d files ³   Last updated on %2d %s %4d  at  %02d:%02d                    ³\n",numtops,date[0],months[date[1]],1900+date[2],timenow[0],timenow[1]);
fp.printf("ÃÄÄÄÄÂÄÄÄÄÄÄÄÄÄÁÄÄÄÄÂÄÄÄÄÄÂÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´\n");
fp.printf("³ Nr ³  File        ³ #DL ³  Description                                     ³\n");
fp.printf("ÃÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÅÄÄÄÄÄÅÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄ´\n");

for(i=0;i<numtops;i++)
   fp.printf("³ %2d ³ %-12s ³%4d ³ %-48.48s ³\n",i+1,tp[i].name,tp[i].n,tp[i].omschr);

fp.printf("ÀÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÁÄÄÄÄÄÁÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÄÙ\n");
fp.printf("\nPress [Enter] to continue\x01");
fp.close();

delete [] tp;
}

static void
addit(char *str,int n)
{
int i,j;
static bool excl_read = FALSE;
static String excl_files[100];

if(!excl_read)
  {
   excl_read = TRUE;

   File tf;
   if(tf.open(FileName(syspath,"NOTOPS.CTL"),fmode_read | fmode_text))
     {
      for(i=0;i<100;i++)
        {
         char s[100];
         if(!tf.readLine(s,99)) break;
         strip_linefeed(s);
         excl_files[i] = s;
        }
     }
  }

for(i=0;i<100;i++) if(excl_files[i][0] && !memicmp(excl_files[i],str,strlen(excl_files[i]))) return;

for(i=0;i<numtops;i++)
  {
  if(n>tp[i].n)
     {
     memmove(&tp[i+1],&tp[i],(numtops-1-i)*sizeof(top));
     str[12]=0;
     strcpy(tp[i].name,str);
     for(j=14;j<80;j++) if(str[j]==']') break;
     for(j++;j<80;j++) if(str[j]!=' ') break;
     strncpy(tp[i].omschr,&str[j],79);
     tp[i].omschr[79] = '\0';
     tp[i].n=n;
     break;
     }
  }
}


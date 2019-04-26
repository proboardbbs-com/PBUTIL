#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include "pbutil.hpp"

char *months_short[] = { "???","Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};

Config cfg;
char syspath[80];

void nomemory();

extern void (*_new_handler)();

extern unsigned _stklen = 0x3000;

main(int argc,char *argv[])
{
_new_handler=nomemory;

char *modes[]={ "UP" , "US" , "MP" , "NC" , "FC" , "MI" , "UK",
                "UI" , "ML" , "MU" , "UF" , "FI" , "FB" , "DM",
                "HF" , NULL};

getsyspath(syspath,argv[0]);

printf( "\nPBUtil v2.25.2019.03.01 þ The ProBoard Maintenance Utility\n" );
printf( "Copyright (c) 1997-2019 rileypcmd.com  All Rights Reserved\n\n" );

if(argc<=1 || argv[1][0]=='?')
  {
   printf("Usage:  PBUTIL <mode> [options]\n\n"
          "<mode>:  UP+ = User Packer                 MI  = Messagebase Reindexer\n"
          "         UF  = Userfile fix                UK+ = User Killer\n"
          "         US  = Userfile sort\n"
          "         ML  = Message Linker              UI  = Userfile Indexer\n"
          "         FC+ = File Counters               MP+ = Messagebase Packer\n"
          "         NC  = Nodelist Compiler           MU  = Music Player\n"
          "         FI  = FILES.BBS reindex           FB  = Fix BINLOG.PB\n"
          "         DM+ = Daily maintenance           HF+ = Hatch Personal File\n\n"
          "  [+] Ä> Use 'PBUTIL [mode] ?' for help on [options].\n");
   return 1;
  }

File f(FileName(syspath,"CONFIG.PRO"));
if(!f.opened())
  {
   Log("Unable to open CONFIG.PRO");
   printf("Can't find CONFIG.PRO!\n");
   return 1;
  }
if(f.read(&cfg,sizeof(cfg)) != sizeof(cfg))
  {
   Log("Invalid config file");
   printf("Invalid config file!\n");
   return 1;
  }

f.close();

int i;

 for(i=0;modes[i];i++) if(!strcmpl(argv[1],modes[i])) break;

 switch(i)
   {
    case  0: userpack(argc-2,&argv[2]); break;
    case  1: usersort(argc-2,&argv[2]); break;
    case  2: msgpack (argc-2,&argv[2]); break;
    case  3: nodelist(argc-2,&argv[2]); break;
    case  4: fc      (argc-2,&argv[2]); break;
    case  5: msgridx (argc-2,&argv[2]); break;
    case  6: userkill(argc-2,&argv[2]); break;
    case  7: useridx (argc-2,&argv[2]); break;
    case  8: msglink (argc-2,&argv[2]); break;
    case  9: musplay (argc-2,&argv[2]); break;
    case 10: userfix (argc-2,&argv[2]); break;
    case 11: fidx    (argc-2,&argv[2]); break;
    case 12: check_binlog(argc-2,&argv[2]); break;
    case 13: maintenance(argc-2,&argv[2]); break;
    case 14: hatch_file(argc-2,&argv[2]); break;
    default: strupr(argv[1]);
             printf("Unknown mode '%s' . Use PBUTIL ? for help.\n",argv[1]);
             return 1;
   }

 return 0;
}

void
nomemory()
{
 Log("----------------------------------------------------");
 Log(" OUT OF MEMORY !!");
 Log("----------------------------------------------------");
 printf("\nNot enough memory to run.\n");
 exit(1);
}

static void near
dolog(char *str)
{
Date date(TODAY);
Time time(NOW);

File fp(FileName(syspath,"PBUTIL.LOG"),fmode_write | fmode_text | fmode_copen | fmode_append);

fp.printf("%02d-%s-%02d %02d:%02d:%02d %s\n",date[0],
                                             months_short[date[1]],
                                             date[2],
                                             time[0],time[1],time[2],
                                             str);
}


void
Log(char *str ...)
{
char s[100];
va_list va;
va_start(va,str);

vsprintf(s,str,va);

dolog(s);
}

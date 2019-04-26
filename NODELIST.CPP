#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dos.h>
#include <io.h>
#include "pbutil.hpp"

#define NODE_ZONE 1
#define NODE_REGION 2
#define NODE_HOST 3
#define NODE_HUB 4
#define NODE_PVT 5
#define NODE_DOWN 6
#define NODE_HOLD 7
#define NODE_NODE 7

struct node_entry
   {
   byte type;
   int zone,region,net,node;
   int cost;
   };

static char xor_string[13]={ 234,123,76,245,87,12,43,56,23,12,98,55,34 };

struct node_index
   {
   byte type;
   long offset;
   union
     {
     int zone,net;
     };
   int cost;
   long fill;
   };

struct cost_pro
   {
   int zone,region,net,cost;
   };

static String
find_nodelist(char *name)
{
 char nl_name[13];
 int nl_version=0;

 if(strchr(name,'.')) return String(name);

 FileName fn(cfg.nodelistdir,name,".*");

 strcpy(nl_name,name);
 strcat(nl_name,".*");

 DirScan srch(fn);
 while(bool(srch))
   {
    char *x=strchr(srch.name(),'.')+1;
    if(atoi(x)>nl_version)
      {
       strcpy(nl_name,srch.name());
       nl_version=atoi(x);
      }
    srch++;
   }

 return String(nl_name);
}

static int num_cost;
static int default_cost;
static cost_pro ar[100];

static int
cost(int zone,int region,int net)
{
for(int i=0;i<num_cost;i++)
  if(ar[i].zone==zone)
    {
    if(ar[i].net==net)       return ar[i].cost;
    if(ar[i].region==region) return ar[i].cost;
    if(ar[i].region==0 && ar[i].net==0) return ar[i].cost;
    }
return default_cost;
}


static int
getnode(char *temp,node_entry& n)
{
char *options[]={"ZONE","REGION","HOST","HUB","PVT","DOWN","HOLD"};
char slices[6][100];

for(int i=0;i<6;i++)
    {
    int y=0;
    while (*temp!=',') if (*temp!='_') slices[i][y++]=*temp++;
                          else {slices[i][y++]=' '; temp++;}
    temp++;
    slices[i][y]='\0';
    }

for(i=0;i<7;i++) if(!strcmpl(slices[0],options[i])) break;

n.type=i+1;

switch(i+1)
  {
  case NODE_ZONE   : n.zone=atoi(slices[1]);
  case NODE_REGION : n.region=atoi(slices[1]);
  case NODE_HOST   : {
                     n.net=atoi(slices[1]);
                     n.node=0;
                     printf("Zone %-3d Reg %-5d Net %-5d\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b\b",n.zone,n.region,n.net);
                     }
                     break;
  default          : n.node=atoi(slices[1]);
                     break;
  }

n.cost=cost(n.zone,n.region,n.net);

return 0;
}

void
nodelist(int argc,char *argv[])
{
int i,y=0;
char line[300];
int curzone=2;
char *cost_options[]={"zone","region","net","default","myzone"};
FileName fn;
File tf;

Log("----------------------------------------------------");
Log("NC: Nodelist compiler");

fn(syspath,"COST.PRO");
if(tf.open(fn,fmode_read | fmode_text))
 for(;;)
    {
    char temp[300];

    if(!tf.readLine(line,299)) break;
    strcpy(temp,strtok(line," "));
    for(i=0;i<5;i++)
      if(!strcmpl(temp,cost_options[i]))
         {
         strcpy(temp,strtok(NULL," "));
         switch(i)
           {
           case 0: {
                   ar[y].zone=atoi(temp);
                   ar[y].region=0;
                   ar[y].net=0;
                   } break;
           case 1: {
                   ar[y].zone=curzone;
                   ar[y].region=atoi(temp);
                   ar[y].net=0;
                   } break;
           case 2: {
                   ar[y].zone=curzone;
                   ar[y].region=0;
                   ar[y].net=atoi(temp);
                   } break;
           case 3: {
                   default_cost=atoi(temp);
                   } break;
           case 4: {
                   curzone=atoi(temp);
                   } break;
           }

         if(i!=3 && i!=4) ar[y++].cost=atoi(strtok(NULL,"\r"));
         break;
         }
    }
 else Log("Warning: file COST.PRO not found");

num_cost=y;

for(i=0;i<num_cost-1;i++)
 for(y=i;y<num_cost;y++)
  if(ar[i].net<ar[y].net)
     {
     cost_pro tmp=ar[i];
     ar[i]=ar[y];
     ar[y]=tmp;
     }

tf.close();

char nl_name[13];

strcpy(nl_name,find_nodelist("NODELIST"));

fn(cfg.nodelistdir,nl_name);

tf.open(fn,fmode_read | fmode_text,40000u);

node_index *index = new node_index[4000];
int idx=0;

node_entry node;

for(i=0;i<=argc;i++)
  {
  strupr(nl_name);
  if(!tf.opened())
    {
     Log("Unable to open nodelist file %s",nl_name);
     printf("Can't find nodelist %s.",nl_name);
    }
          else
           {
           Log("Compiling nodelist %s",nl_name);

           printf("Processing nodelist %-13s... ",nl_name);
           memcpy(&index[idx++],nl_name,13);
           long num_nodes=0;

           for(;;)
            {
            long off=tf.pos();

            if(!tf.readLine(line,256)) break;

            if(line[0]==';' || strlen(line)<6) continue;
               else
                 {
                 getnode(line,node);

                 num_nodes++;
                 if(node.type<NODE_HUB)
                   {
                   index[idx].type=node.type;
                   switch(node.type)
                     {
                     case NODE_ZONE  : index[idx].net=node.zone;   break;
                     case NODE_REGION: index[idx].net=node.region; break;
                     case NODE_HOST  : index[idx].net=node.net;    break;
                     }
                   index[idx].cost=node.cost;
                   index[idx++].offset=off;
                   }
                 }
             }
            Log("%ld nodes compiled",num_nodes);
            printf("%ld nodes compiled          ",num_nodes);
           }
  printf("\n");
  tf.close();
  strcpy(nl_name,find_nodelist(argv[i]));

  fn(cfg.nodelistdir,nl_name);
  tf.open(fn,fmode_read | fmode_text,40000u);
  }

File f;
fn(syspath,"NODE_IDX.PRO");
if(!f.open(fn,fmode_create,1024))
  {
   Log("Unable to create nodelist index file");

   printf("\nCan't create index file!\n");
   return;
  }

for(i=0;i<idx;i++)
  {
   char *ptr=(char *)&index[i];

   for(int j=0;j<13;j++) ptr[j]^=xor_string[j];
  }

f.write(index,idx*sizeof(node_index));

f.close();
}


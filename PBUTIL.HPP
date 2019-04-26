#include <tslib.hpp>
#include <pb_lib.hpp>

struct User     : _User     {};
struct Config   : _Config   {};
struct msgarea  : _MsgArea  {};
struct filearea : _FileArea {};

#define MSG_BOTH 0
#define MSG_PVT  1
#define MSG_PUB  2

#define MSG_LOCAL 0
#define MSG_NET   1
#define MSG_ECHO  2

/*
#define MA_DELETED 1
#define MA_UNMOVED_NET 2
#define MA_NET 4
#define MA_PVT 8
#define MA_RECEIVED 16
#define MA_UNMOVED_ECHO 32
#define MA_LOCAL 64
#define MA_RESERVED 128
*/


struct msgidx
  {
  int num;
  unsigned char area;
  };

struct msgtoidx
  {
  char name[36];
  };

struct msginfo
  {
  int low,high,total;
  int active[200];
  };

/*
struct qbbs_msg
  {
  int msgnum,prevmsg,nextmsg,tread;
  unsigned startrec;
  int numrecs,destnet,destnode,orgnet,orgnode;
  char destzone,orgzone;
  int cost;
  unsigned char msgattr,netattr,area;
  char posttime[6];
  char postdate[9];
  char to[36];
  char from[36];
  char subj[67];
  Date recvdate;
  Time recvtime;
  };
*/

struct qbbs_msg : public _QbbsMsgHdr {};

extern Config cfg;
extern char syspath[];

void userpack(int,char *argv[]);
void usersort(int,char *argv[]);
void userkill(int,char *argv[]);
void useridx (int,char *argv[]);
void msgpack (int,char *argv[]);
void msgkill (int,char *argv[]);
void msglink (int,char *argv[]);
void nodelist(int,char *argv[]);
void fc      (int,char *argv[]);
void msgridx (int,char *argv[]);
void msgrep  (int,char *argv[]);
void lastread(int,char *argv[]);
void musplay (int,char *argv[]);
void userfix (int,char *argv[]);
void fidx    (int,char *argv[]);
void check_binlog(int,char *[]);
void maintenance(int,char *[]);
void hatch_file(int,char *[]);

bool JamPack (msgarea&,bool,bool);
bool JamReIndex(msgarea&,bool show);

void Log(char * ...);


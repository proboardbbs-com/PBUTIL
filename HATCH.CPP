#define Use_LinkedList
#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <dir.h>
#include "pbutil.hpp"

void
hatch_file(int argc,char *argv[])
{
   int i;
   LinkedList<String> hatch_list;
   LinkedList<String> wc_list;
   String from,to;
   bool keep_file = TRUE;
   bool copy_file = FALSE;

   from = cfg.sysopname;

   for(i = 0 ; i < argc ; i++)
   {
      if(argv[i][0] == '?')
      {
         printf("Usage: PBUTIL HF <files> [files...] [options]\n\n"
                "   Options\n"
                "컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴컴�\n\n"
                "   <files>     List of files to be hatched\n"
                "   -T=Name     Send files to <name> (required)\n"
                "   -F=Name     Sender's name (default = sysop name)\n"
                "   -D          Delete file(s) after download\n"
                "   -C          Copy file(s) to personal files directory\n"
                "\n"
                " * Underscores can be used in user names instead of spaces.\n"
                " * File specifications can included paths and/or wildcards\n\n");
         return;
      }

      if(!strchr("-/",argv[i][0]))
      {
         wc_list.add(String(argv[i]));
      }
      else
      {
         switch(toupper(argv[i][1]))
         {
            case 'T': to = &argv[i][2];
                      to.replace("_"," ");
                      break;
            case 'F': from = &argv[i][2];
                      from.replace("_"," ");
                      break;
            case 'D': keep_file = FALSE;
                      break;
            case 'C': copy_file = TRUE;
                      break;
         }
      }
   }

   Log("----------------------------------------------------");
   Log("HF: Hatch File");

   if(from[0] == '=')
      from.del(0,1);

   if(to[0] == '=')
      to.del(0,1);

   if(!to.len())
   {
      printf("No destination specified!\n");
      Log("No destination specified");
      return;
   }

   for(wc_list.rewind() ; !wc_list.eol() ; wc_list++)
   {
      String cur_wc = wc_list.get();
      FileName path(cur_wc);

      if(strpbrk(path,"\\:"))
      {
         path.stripName();
         path = ts_ResolveDir(path);
      }
      else
         path.clear();

      if(path[0] && !ts_DirExists(path))
         continue;

      LinkedList<String> files;
      bool in_curdir = TRUE;

      DirScan scan(cur_wc);

      for(;int(scan);scan++)
         files.add(String(scan.name()));

      if(!files.count() && !path[0])
      {
         in_curdir = FALSE;

         scan.first(FileName(cfg.pvtuploadpath,cur_wc));

         for(;int(scan);scan++)
            files.add(String(scan.name()));
      }

      for(files.rewind() ; !files.eol() ; files++)
      {
         if(!path[0] && !in_curdir)
         {
            hatch_list.add(files.get());
            continue;
         }

         if(!path[0])
         {
            char s[80];
            getcwd(s,78);
            append_backspace(s);
            path = s;
         }

         if(copy_file)
         {
            if(ts_ResolveDir(cfg.pvtuploadpath) != path)
               ts_CopyFile(FileName(path,files.get()),cfg.pvtuploadpath,8192);

            hatch_list.add(files.get());
         }
         else
         {
            hatch_list.add(FileName(path,files.get()));
         }
      }
   }

   if(!hatch_list.count())
   {
      printf("No files found!\n");
      Log("No files found");
      return;
   }

   File f;

   if(!f.open(FileName(syspath,"PVTFILES.PB"),fmode_rw | fmode_excl | fmode_copen))
   {
      printf("Can't open PVTFILES.PB!\n");
      Log("Can't open PVTFILES.PB!");
      return;
   }

   for(hatch_list.rewind() ; !hatch_list.eol() ; hatch_list++)
   {
      _PrivateFile pvt_r,pvt_w;

      CLEAR_OBJECT(pvt_w);

      strcpy(pvt_w.fname,hatch_list.get());
      strcpy(pvt_w.from,from);
      strcpy(pvt_w.to,to);
      pvt_w.date.today();
      pvt_w.attr = (keep_file ? PVTFILE_KEEP : 0);

      for(;;)
      {
         long pos = f.pos();

         if(f.read(&pvt_r,sizeof(pvt_r)) != sizeof(pvt_r) || !pvt_r.fname[0])
         {
            f.seek(pos);
            f.write(&pvt_w,sizeof(pvt_w));
            f.seek(f.pos());

            break;
         }
      }

      Log("Hatching file '%s' to %s",pvt_w.fname,pvt_w.to);
      printf("Hatching file '%s' to %s\n",pvt_w.fname,pvt_w.to);
   }
}



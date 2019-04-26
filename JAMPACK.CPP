#include <stdio.h>
#include <string.h>
#include "pbutil.hpp"
#include "..\proboard\jam.hpp"

static dword
JamDate( const Date& pbd , const Time& pbt)
{
   int years = pbd.year() - 70;

   dword unixtime = dword(years) * 365L + (dword(years + 1) / 4L);

   return ((unixtime + pbd.dayNum() - 1) * 86400L + long(pbt));
}

bool
JamPack(msgarea& ma , bool do_renum , bool do_kill)
{
   File fi_jhr,
        fi_jdt,
        fi_jlr,
        fo_jhr,
        fo_jdt;

   FileName fn_jhr ( ma.path , ".JHR" );
   FileName fn_jdt ( ma.path , ".JDT" );
   FileName fn_jlr ( ma.path , ".JLR" );

   FileName fno_jhr( ma.path , ".$HR" );
   FileName fno_jdt( ma.path , ".$DT" );
   FileName fno_jlr( ma.path , ".$LR" );

   if(!fi_jhr.open( fn_jhr , fmode_read , 8192 ))
      return FALSE;
   if(!fi_jlr.open( fn_jlr , fmode_rw ))
      return FALSE;
   if(!fi_jdt.open( fn_jdt , fmode_read , 8192 ))
      return FALSE;

   if(!fo_jhr.open( fno_jhr , fmode_create , 8192 ))
      return FALSE;
   if(!fo_jdt.open( fno_jdt , fmode_create , 8192 ))
      return FALSE;

   Log("Packing JAM area %s%s%s",ma.path,do_renum ? " (Renumber)":"",do_kill ? " (Purge)":"");

   printf("Packing JAM area %s%s%s...",ma.path,do_renum ? " (Renumber)":"",do_kill ? " (Purge)":"");

   byte *subfields = new byte[16384u];
   byte *textbuf   = new byte[32768u];
   word *offset    = new word[32000u];

   memset(offset , 0 , 64000u);

   JamFileHeader  *j_fhdr = new JamFileHeader;
   JamHeader      *j_hdr  = new JamHeader;

   bool error        = FALSE;
   long extra_killed = 0;

   if(fi_jhr.read( j_fhdr , sizeof(JamFileHeader) ) != sizeof(JamFileHeader))
   {
      Log("%s: Error reading file header" , (char *)fn_jhr );
      error = TRUE;
   }

   if(j_fhdr->signature != JAM_SIGNATURE)
   {
      Log("%s: Bad signature for file header", (char *)fn_jhr );
      error = TRUE;
   }

   fo_jhr.seek( 1024L );

   long today = JamDate(Date(TODAY) , Time(23,59,59));
   int msgnum = 1;
   int prevmsg = 0;

   j_fhdr->baseMsgNum = 0x8FFFFFFFL;
//   j_fhdr->activeMsgs = 0;

   while(!error)
   {
      if(fi_jhr.read( j_hdr , sizeof(JamHeader) ) != sizeof(JamHeader) )
         break;

      j_fhdr->activeMsgs--;

      if(j_hdr->signature != JAM_SIGNATURE)
      {
         Log("%s: Bad signature at offset %08lX", (char *)fn_jhr , fi_jhr.pos() - sizeof(JamHeader));
         error = TRUE;
         break;
      }

      if(fi_jhr.read( subfields , word(j_hdr->subFieldLen) ) != word(j_hdr->subFieldLen) )
      {
         Log("%s: Premature EOF" , (char *)fn_jhr);
         error = TRUE;
         break;
      }

      if(j_hdr->attribute & JAM_MSG_DELETED)
         continue;

      long w_days = (today - j_hdr->dateWritten) / 86400L;
      long r_days = j_hdr->dateReceived ? ((today - j_hdr->dateReceived) / 86400L) : 0;

      if(w_days < 0)
         w_days = 0;
      if(r_days < 0)
         r_days = 0;

      if(do_kill)
      {
         bool kill = FALSE;

         if(ma.msgKillDays)
            if(w_days > ma.msgKillDays)
            {
               kill = TRUE;
            }

         if(ma.rcvKillDays)
            if(r_days > ma.rcvKillDays && (j_hdr->attribute & JAM_MSG_READ))
            {
               kill = TRUE;
            }

         if(ma.maxMsgs)
            if(j_fhdr->activeMsgs >= ma.maxMsgs)
            {
               kill = TRUE;
            }

         if(kill)
         {
            extra_killed++;

            continue;
         }
      }

      if(do_renum)
      {
         for(int i = prevmsg+1 ; i < int(j_hdr->messageNumber) ; i++)
            offset[i] = offset[i-1] + 1;

         offset[i] = offset[i-1];

         prevmsg = int(j_hdr->messageNumber);

         j_hdr->messageNumber = msgnum++;
      }

      j_fhdr->activeMsgs++;

      if(j_hdr->messageNumber < j_fhdr->baseMsgNum)
         j_fhdr->baseMsgNum = j_hdr->messageNumber;

      if(j_hdr->txtLen > 32768L)
         j_hdr->txtLen = 32768L;

      fi_jdt.seek( j_hdr->offset );

      j_hdr->txtLen = fi_jdt.read(textbuf , word(j_hdr->txtLen));
      j_hdr->offset = fo_jdt.pos();

      fo_jhr.write( j_hdr , sizeof(JamHeader) );
      fo_jhr.write( subfields , word(j_hdr->subFieldLen) );

      fo_jdt.write( textbuf , j_hdr->txtLen );
   }

   if(j_fhdr->baseMsgNum >= 0x8FFFFFFFL)
      j_fhdr->baseMsgNum = 1;

   if(!error)
   {
      fo_jhr.rewind();
      fo_jhr.write( j_fhdr , sizeof(JamFileHeader)) ;

      if(do_renum)
      {
         //for(int x=0;x<40;x++) Log("offset[%d]=%d",x,offset[x]);

         fi_jlr.rewind();

         for(long r=0;;r++)
         {
            JamLastRead j_lr;

            if(fi_jlr.read(&j_lr, sizeof(JamLastRead)) != sizeof(JamLastRead))
               break;

            j_lr.highReadMsg -= offset[ word(j_lr.highReadMsg) ];
            j_lr.lastReadMsg -= offset[ word(j_lr.lastReadMsg) ];

            fi_jlr.seek ( r * sizeof(JamLastRead) );
            fi_jlr.write( &j_lr , sizeof(JamLastRead) );
            fi_jlr.seek ( fi_jlr.pos() );
         }
      }
   }

   delete [] subfields;
   delete [] textbuf;
   delete [] offset;

   delete j_fhdr;
   delete j_hdr;

   if(extra_killed)
      Log("%ld messages purged" , extra_killed);

   if(error)
      printf("Error!\n");
   else
      printf("Done.\n");

   fi_jhr.close();
   fi_jdt.close();
   fi_jlr.close();
   fo_jhr.close();
   fo_jdt.close();

   if(!error)
   {
      unlink(fn_jhr);
      unlink(fn_jdt);

      rename(fno_jhr , fn_jhr);
      rename(fno_jdt , fn_jdt);
   }
   else
   {
      unlink(fno_jhr);
      unlink(fno_jdt);
   }

   if(!error)
      JamReIndex(ma,FALSE);

   return !error;
}

static String
JamRecipient( byte *subfields , word size )
{
   String recip;
   JamSubField *sub;
   word pos;

   for(pos = 0 ; pos < size ; )
   {
      sub = (JamSubField *)(&subfields[pos]);

      pos += sizeof(JamSubField);

      byte *buf = new byte[word(sub->datLen)+1];
      memcpy(buf , &subfields[pos] , word(sub->datLen));
      buf[word(sub->datLen)] = '\0';

      pos += sub->datLen;

      if(sub->loId == JAMID_RECEIVERNAME)
      {
         char x[36];
         strncpy( x , buf , 35 );
         x[35] = '\0';

         recip = x;
      }

      delete [] buf;
   }

   return recip;
}


bool
JamReIndex( msgarea& ma , bool show)
{
   File fi_jhr,
        fo_jdx;

   FileName fn_jhr ( ma.path , ".JHR" );
   FileName fn_jdx ( ma.path , ".JDX" );

   if(!fi_jhr.open( fn_jhr , fmode_rw | fmode_excl , 8192 ))
      return FALSE;

   if(!fo_jdx.open( fn_jdx , fmode_create , 8192 ))
      return FALSE;

   long high_index = 0;
   bool error      = FALSE;

   JamFileHeader *fhdr = new JamFileHeader;
   byte *subfields = new byte[8192];

   if(show)
      printf("Reindexing JAM area %s...", ma.path);

   Log("Indexing JAM area %s" , ma.path);

   if(fi_jhr.read( fhdr , sizeof(JamFileHeader) ) != sizeof(JamFileHeader))
   {
      Log("%s: Unable to read file" , (char *)fn_jhr);
      error = TRUE;
   }

   if(fhdr->signature != JAM_SIGNATURE)
   {
      Log("%s: Bad signature in file header" , (char *)fn_jhr);
      error = TRUE;
   }

   fhdr->activeMsgs = 0;

   while(!error)
   {
      JamHeader hdr;
      JamIndex  idx;

      if(fi_jhr.read( &hdr , sizeof(JamHeader) ) != sizeof(JamHeader))
         break;

      if(hdr.signature != JAM_SIGNATURE)
      {
         Log("%s: Bad signature" , (char *)fn_jhr);
         error = TRUE;
         break;
      }

      if(fi_jhr.read(subfields,word(hdr.subFieldLen)) != word(hdr.subFieldLen))
      {
         Log("%s: Premature EOF" , (char *)fn_jhr);
         error = TRUE;
         break;
      }

      String recipient = JamRecipient(subfields,word(hdr.subFieldLen));

      if(hdr.attribute & JAM_MSG_DELETED)
         idx.crc32  = 0xFFFFFFFFL;
      else
      {
         fhdr->activeMsgs++;
         idx.crc32  = JamCrc(recipient);;
      }

      idx.offset = fi_jhr.pos() - sizeof(JamHeader) - hdr.subFieldLen;

      long new_index = hdr.messageNumber - fhdr->baseMsgNum;

      JamIndex dummy;

      dummy.crc32  = 0xFFFFFFFFL;
      dummy.offset = 0xFFFFFFFFL;

      if(new_index > high_index)
         fo_jdx.seek(high_index * sizeof(JamIndex));

      for( ; high_index < new_index ; high_index++)
      {
         fo_jdx.write( &dummy , sizeof(JamIndex) );
      }

      if(new_index >= high_index)
         high_index = new_index+1;

      fo_jdx.write( &idx , sizeof(JamIndex) );
   }

   fi_jhr.rewind();
   fi_jhr.write(fhdr , sizeof(JamFileHeader));

   if(show)
      if(error)
         printf("Error!\n");
      else
         printf("Done.\n");

   delete fhdr;
   delete [] subfields;

   fi_jhr.close();
   fo_jdx.close();

   return !error;
}

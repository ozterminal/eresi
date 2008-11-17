
/*  First try of implementation of a gdb wrapper. 
 *
 *  See gdb documentation, section D for more information on the
 *  remote serial protocol. To make it short, a packet looks like the following:
 *
 *  $packet-data#checksum  or  $sequence-id:packet-data#checksum.
 * 
 *  where the checksum is the sum of all the characters modulo 256.
 */

#include             <netdb.h>
#include             "libaspect.h"
#include             "gdbwrapper-internals.h"
#include             "gdbwrapper.h"


gdbwrapworld_t       gdbwrapworld;


static Bool          gdbwrap_errorhandler(const char *error, gdbwrap_t *desc)
{
  ASSERT(error != NULL);

  if (!strncmp(GDBWRAP_REPLAY_OK, error, strlen(GDBWRAP_REPLAY_OK)))
    return FALSE;

  if (!strncmp(GDBWRAP_NO_SPACE, error, strlen(GDBWRAP_NO_SPACE)))
    fprintf(stderr, "space was not updated.\n");

  if (!strncmp(GDBWRAP_NO_TABLE, error, strlen(GDBWRAP_NO_TABLE)))
    fprintf(stdout, "Not populating the table\n");

  if (!strncmp(GDBWRAP_DEAD, error, strlen(GDBWRAP_DEAD)))
    fprintf(stdout, "The server seems to be dead. Message not sent.\n");

  if (error[0] == GDBWRAP_REPLAY_ERROR)
    fprintf(stdout, "Error: %s\n", error);
  
  if (error[0] == GDBWRAP_EXIT_W_STATUS)
    {
      fprintf(stdout, "Exit with status: %s\n", error);
      desc->is_active = FALSE;
    }
  
  if (error[0] == GDBWRAP_EXIT_W_SIGNAL)
    {
      fprintf(stdout, "Exit with signal: %s\n", error);
      desc->is_active = FALSE;
    }

  if (error[0] == GDBWRAP_NULL_CHAR)
    fprintf(stdout, "Command not supported\n");

  return TRUE;
}


unsigned             gdbwrap_lastsignal(gdbwrap_t *desc)
{
  unsigned           ret = 0;

  /* When we receive a packet starting with GDBWRAP_SIGNAL_RECV, the
     next 2 characters reprensent the signal number. */
  if (desc->packet != NULL && desc->packet[0] == GDBWRAP_SIGNAL_RECV)
    ret = gdbwrap_atoh(desc->packet + 1, 2 * sizeof(char));

  return ret;
}


Bool                 gdbwrap_is_active(gdbwrap_t *desc)
{
  if (desc->is_active)
    return TRUE;
  else
    return FALSE;
}


static char          *gdbwrap_lastmsg(gdbwrap_t *desc)
{
  return desc->packet;
}

/**
 * This function parses a string *strtoparse* starting at character
 * *begin* and ending at character *end*. The new parsed string is
 * saved in *strret*. If *begin* is not found in *strtoparse* then the
 * function returns NULL. If *end* is not found in *strtoparse*, then
 * the function returns NULL..
 *
 * @param strtoparse: String to parse.
 * @param strret    : String to return without *begin* and *end*.
 * @param begin     : String where to start parsing. If NULL,
 *                    we start from the beginning.
 * @param end       : String where to end parsing. If NULL,
 *                    we copy the string from *begin* to then
 *                    end of *strtoparse* (ie a NULL char is found).
 * @param maxsize   : The maximum size to extract.
 * @return          : Returns a pointer on the beginning of the new string.
 */
static char          *gdbwrap_extract_from_packet(const char *strtoparse,
						  char *strret,
						  const char *begin,
						  const char *end,
                                                  int maxsize)
{
  const char         *charbegin;
  const char         *charend;
  unsigned           strtorem;
  ptrdiff_t          strsize;

  ASSERT(strtoparse != NULL);

  if (begin == NULL)
    {
      charbegin = strtoparse;
      strtorem  = 0;
    }
  else
    {
      charbegin = strstr(strtoparse, begin);
      strtorem  = strlen(begin); 
      if (charbegin == NULL)
	return NULL;
    }
  
  if (end == NULL)
    charend = charbegin + strlen(charbegin);
  else
    {
      charend = strstr(charbegin, end);
      if (charend == NULL)
	return NULL;
    }

  strsize = charend - charbegin - strtorem;
  if (strsize > maxsize)
    strsize = maxsize;

  strncpy(strret, charbegin + strtorem, strsize);
  strret[strsize] = GDBWRAP_NULL_CHAR;

  return strret;
}


static la32          gdbwrap_little_endian(la32 addr)
{
  la32               addrlittle = 0;
  unsigned           i;

  for (i = 0; addr > 0; i++)
    {
      addrlittle += (LOBYTE(addr) << (BYTE_IN_BIT * (sizeof(addr) - 1 - i)));
      addr >>= BYTE_IN_BIT;
    }

  return addrlittle;
}


unsigned             gdbwrap_atoh(const char * str, unsigned size)
{
  unsigned           i;
  unsigned           hex;

  for (i = 0, hex = 0; i < size; i++)
    if (str[i] >= 'a' && str[i] <= 'f')
      hex += (str[i] - 0x57) << 4 * (size - i - 1);
    else if (str[i] >= '0' && str[i] <= '9')
      hex += (str[i] - 0x30) << 4 * (size - i - 1);
    else
      {
	fprintf(stderr, "Wrong char: %c - %#x\n", str[i], str[i]);
	ASSERT(FALSE); 
      }
  return hex;
}


static uint8_t       gdbwrap_calc_checksum(const char *str, gdbwrap_t *desc)
{
  unsigned           i;
  uint8_t            sum;
  char               *result;

  
  result = gdbwrap_extract_from_packet(str, desc->packet, GDBWRAP_BEGIN_PACKET,
                                       GDBWRAP_END_PACKET,
				       desc->max_packet_size);
  /* If result == NULL, it's not a packet. */
  if (result == NULL)
    result = gdbwrap_extract_from_packet(str, desc->packet, NULL, NULL,
					 desc->max_packet_size);
    
  for (i = 0, sum = 0; i < strlen(result); i++)
    sum += result[i];

  return  sum;
}


static char          *gdbwrap_make_message(const char *query,
                                           gdbwrap_t *desc)
{
  uint8_t            checksum       = gdbwrap_calc_checksum(query, desc);
  unsigned           max_query_size = (desc->max_packet_size -
				       strlen(GDBWRAP_BEGIN_PACKET)
				       - strlen(GDBWRAP_END_PACKET)
				       - sizeof(checksum));

  /* Sometimes C sucks... Basic source and destination checking. We do
     not check the overlapping tho.*/
  if (strlen(query) < max_query_size && &*query != &*desc->packet)
    ASSERT(snprintf(desc->packet, desc->max_packet_size, "%s%s%s%.2x",
		    GDBWRAP_BEGIN_PACKET, query, GDBWRAP_END_PACKET, checksum) > 0);
  else
    ASSERT(FALSE);

  return desc->packet;
}


/**
 * This function performes a run-length decoding and writes back to
 * *dstpacket*, but no more than *maxsize* bytes. Actually, it blows
 * up if we try to write more packet :).
 *
 * @param srcpacket:  the encoded packet.
 * @param maxsize:  the maximal size of the decoded packet.
 */
static char          *gdbwrap_run_length_decode(char *dstpacket, const char *srcpacket,
						unsigned maxsize)
{
  /* Protocol specifies to take the following value and substract 29
     and repeat by this number the previous character.  Note that the
     compression may be used multiple times in a packet. */
  char               *encodestr;
  char               valuetocopy;
  uint8_t            numberoftimes;
  unsigned           iter;
  unsigned           strlenc;
    
  ASSERT(dstpacket != NULL && srcpacket != NULL &&
	 strncmp(srcpacket, GDBWRAP_START_ENCOD, 1));
  if (&*srcpacket != &*dstpacket)
    strncpy(dstpacket, srcpacket, maxsize);
  encodestr   = strstr(dstpacket, GDBWRAP_START_ENCOD);
  while (encodestr != NULL)
    {
      valuetocopy    = encodestr[-1];
      numberoftimes  = encodestr[1] - 29;
      strlenc        = strlen(encodestr);
/*       locmaxsize    += strlenc + numberoftimes - 2; */
/*       ASSERT(locmaxsize < maxsize); */

/*       printf("The localmax: %#x\n", locmaxsizecd ); */
/*       fflush(stdout); */

      /* We move the string to the right, then set the bytes. We
	 substract 2, because we have <number>*<char> where * and
	 <char> are filled with the value of <number> (ie 2 chars). */
      for (iter = 0; iter < strlenc; iter++)
	encodestr[strlenc + numberoftimes - iter - 2] = encodestr[strlenc - iter];
      memset(encodestr, valuetocopy, numberoftimes);
      encodestr = strstr(NEXT_CHAR(encodestr), GDBWRAP_START_ENCOD);
    }
/*   printf("\nPacket:\n%s\n", dstpacket); */
  return dstpacket;
}


/**
 * Populate the gdb registers with the values received in the
 * packet. A packet has the following form:
 *
 * $n:r;[n:r;]#checksum
 *
 * where n can be a number (the register), or "thread" and r is the
 * value of the thread/register.
 *
 * @param packet: the packet to parse.
 * @param reg   : the structure in which we want to write the registers.
 */
static void         gdbwrap_populate_reg(char *packet,
					 gdbwrap_t *desc)
{
  const char        *nextpacket;
  char              *nextupacket;
  char              packetsemicolon[50];
  char              packetcolon[50];
  unsigned          packetoffset = 0;

  /* If a signal is received, we populate the registers, starting
     after the signal number (ie after Tnn, where nn is the
     number). */
  if (packet[0] == GDBWRAP_SIGNAL_RECV)
    packetoffset = 3;
  
  while ((nextpacket =
	  gdbwrap_extract_from_packet(packet + packetoffset,
				      packetsemicolon,
				      NULL,
				      GDBWRAP_SEP_SEMICOLON,
				      sizeof(packetsemicolon))) != NULL)
    {
      nextupacket = gdbwrap_extract_from_packet(nextpacket, packetcolon, NULL,
						GDBWRAP_SEP_COLON,
						sizeof(packetcolon));
      ASSERT(nextupacket != NULL);
      if (strlen(nextupacket) == 2)
	{
	  uint8_t regnumber = gdbwrap_atoh(nextupacket, strlen(nextupacket));
	  ureg32  regvalue;
	  
	  nextupacket = gdbwrap_extract_from_packet(nextpacket, packetcolon,
						    GDBWRAP_SEP_COLON, NULL,
						    sizeof(packetcolon));
	  ASSERT(nextupacket != NULL);
	  regvalue = gdbwrap_atoh(nextupacket, strlen(nextupacket));
	  regvalue = gdbwrap_little_endian(regvalue);
					   
	  *(&desc->reg32.eax + regnumber) =  regvalue;
	}
      /* We add 1 in order not to take the right limit. In the worst
	 case, we should get the NULL char. */
      packetoffset += strlen(nextpacket) + 1;
    }
}


static void          gdbwrap_check_ack(gdbwrap_t *desc)
{
  int                rval;
  rval = recv(desc->fd, desc->packet, 1, 0);
  /* The result of the previous recv must be a "+". */
  ASSERT(rval != -1  && !strncmp(desc->packet, GDBWRAP_COR_CHECKSUM, 1));
}


static char          *gdbwrap_get_packet(gdbwrap_t *desc)
{
  int                rval;
  char               checksum[3];

  ASSERT(desc != NULL);

  gdbwrap_check_ack(desc);
  rval = recv(desc->fd, desc->packet, desc->max_packet_size, 0);
  /* if rval == 0, it means the host is disconnected/dead. */
  if (rval) {
    desc->packet[rval] = GDBWRAP_NULL_CHAR;
    gdbwrap_extract_from_packet(desc->packet, checksum, GDBWRAP_END_PACKET,
				NULL, sizeof(checksum));
    /* If no error, we ack the packet. */
    if (rval != -1 &&
	gdbwrap_atoh(checksum, strlen(checksum)) ==
	gdbwrap_calc_checksum(desc->packet, desc))
      {
	rval = send(desc->fd, GDBWRAP_COR_CHECKSUM, strlen(GDBWRAP_COR_CHECKSUM),
		    0x0);
	gdbwrap_errorhandler(desc->packet, desc);

	return gdbwrap_run_length_decode(desc->packet, desc->packet,
					 desc->max_packet_size);
      }
    else ASSERT(FALSE);
  }
  else desc->is_active = FALSE;
  
  return NULL;
}


static char          *gdbwrap_send_data(const char *query, gdbwrap_t *desc)
{
  int                rval = 0;
  char               *mes;

  ASSERT(desc != NULL && query != NULL);

  if (gdbwrap_is_active(desc))
    {
      mes  = gdbwrap_make_message(query, desc);
      rval = send(desc->fd, mes, strlen(mes), 0);
      ASSERT(rval != -1);
      mes  = gdbwrap_get_packet(desc);
    }
  else
    {
      gdbwrap_errorhandler(GDBWRAP_DEAD, desc);
      mes = NULL;
    }
  
  return mes;
}


/**
 * Set/Get the gdbwrapworld variable. It's not mandatory to use the
 * other functions, but sometimes a global variable is required.
 */
gdbwrapworld_t       gdbwrap_current_set(gdbwrap_t *world)
{
  gdbwrapworld.gdbwrapptr = world;
  return gdbwrapworld;
}


gdbwrap_t            *gdbwrap_current_get(void)
{
  return gdbwrapworld.gdbwrapptr;
}


/**
 * Initialize the descriptor. We provide a default value of 600B for
 * the string that get the replies from server.
 *
 */
gdbwrap_t            *gdbwrap_init(int fd)
{
  gdbwrap_t          *desc = malloc(sizeof(gdbwrap_t));
      
  ASSERT(fd && desc != NULL);
  desc->max_packet_size   = 600;
  desc->packet            = malloc((desc->max_packet_size + 1) * sizeof(char));
  desc->fd                = fd;
  desc->is_active         = TRUE;
  ASSERT(desc->packet != NULL);

  return desc;
}


void                gdbwrap_close(gdbwrap_t *desc)
{
  ASSERT(desc != NULL && desc->packet != NULL);
  free(desc->packet);
  free(desc);
}


/**
 * Initialize a connection with the gdb server and allocate more
 * memory for packets if necessary.
 *
 */
void                gdbwrap_hello(gdbwrap_t *desc)
{
  char              *received    = NULL;
  char              *result      = NULL;
  unsigned          previousmax  = 0;

  received = gdbwrap_send_data(GDBWRAP_QSUPPORTED, desc);

  if (received != NULL)
    {
      result   = gdbwrap_extract_from_packet(received, desc->packet,
					     GDBWRAP_PACKETSIZE,
					     GDBWRAP_SEP_SEMICOLON,
					     desc->max_packet_size);

      /* If we receive the info, we update gdbwrap_max_packet_size. */
      if (result != NULL)
	{
	  char *reallocptr;
      
	  previousmax = desc->max_packet_size;
	  desc->max_packet_size = gdbwrap_atoh(desc->packet, strlen(desc->packet));
	  reallocptr = realloc(desc->packet, desc->max_packet_size + 1);
	  if (realloc != NULL)
	    desc->packet = reallocptr;
	  else
	    {
	      gdbwrap_errorhandler(GDBWRAP_NO_SPACE, desc);
	      desc->max_packet_size = previousmax;
	    }
	}
      /* We set the last bit to a NULL char to avoid getting out of the
	 weeds with a (unlikely) bad strlen. */
      desc->packet[desc->max_packet_size] = GDBWRAP_NULL_CHAR;
    }
}


/**
 * Send a "disconnect" command to the server and free the packet.
 */
void                gdbwrap_bye(gdbwrap_t *desc)
{
  assert(desc != NULL);
  gdbwrap_send_data(GDBWRAP_DISCONNECT, desc);
  printf("\nThx for using gdbwrap :)\n");
}


void                gdbwrap_reason_halted(gdbwrap_t *desc)
{
  char              *received;

  received = gdbwrap_send_data(GDBWRAP_WHY_HALTED, desc);
  printf("\n\nReceived why halted: %s\n", received);
  fflush(stdout);
  if (gdbwrap_is_active(desc))
    gdbwrap_populate_reg(received, desc);
  else
    gdbwrap_errorhandler(GDBWRAP_NO_TABLE, desc);
}


/**
 * Great, the gdb protocol has absolutely no consistency, thus we
 * cannot reuse the gdbwrap_populate_reg. We receive a poorly
 * documented bulk message when sending the "g" query.
 */
gdbwrap_gdbreg32     *gdbwrap_readgenreg(gdbwrap_t *desc)
{
  char               *rec;
  unsigned           i;
  ureg32             regvalue;
  
  rec = gdbwrap_send_data(GDBWRAP_GENPURPREG, desc);

  for (i = 0; i < sizeof(gdbwrap_gdbreg32) / sizeof(ureg32); i++)
    {
      /* 1B = 2 characters */
       regvalue = gdbwrap_atoh(rec, 2 * DWORD_IN_BYTE);
       regvalue = gdbwrap_little_endian(regvalue);
       *(&desc->reg32.eax + i) = regvalue;
       rec += 2 * DWORD_IN_BYTE;
    }
  
  return &desc->reg32;
}


void                 gdbwrap_continue(gdbwrap_t *desc)
{
  char               *rec;

  if (gdbwrap_is_active(desc))
    {
      rec = gdbwrap_send_data(GDBWRAP_CONTINUE, desc);
      printf("Message received from the cont: %s\n", rec);
      gdbwrap_populate_reg(rec, desc);
    }
}


/**
 * Set a breakpoint. We read the value in memory, save it and write a
 * 0xcc in replacement. The usual command to set a bp is not supported
 * by the gdbserver.
 */
void                 gdbwrap_setbp(la32 linaddr, void *datasaved, gdbwrap_t *desc)
{
  u_char             bp = 0xcc;
  char               *ret;
  unsigned           atohresult;

  ret = gdbwrap_readmemory(linaddr, 1, desc);
   /* Fix: not clean. ATOH is not clean when returning an unsigned. */
  atohresult = gdbwrap_atoh(ret, 2 * 1);
   memcpy(datasaved, &atohresult, 1);
  gdbwrap_writememory(linaddr, &bp, sizeof(u_char), desc);
}


void                 gdbwrap_delbp(la32 linaddr, void *datasaved, gdbwrap_t *desc)
{
   gdbwrap_writememory(linaddr, datasaved, sizeof(u_char), desc);
}


char                 *gdbwrap_readmemory(la32 linaddr, unsigned bytes,
					 gdbwrap_t *desc)
{
  char               *rec;
  char               packet[50];

  snprintf(packet, sizeof(packet), "%s%x%s%x", GDBWRAP_MEMCONTENT,
	   linaddr, GDBWRAP_SEP_COMMA, bytes);
  rec = gdbwrap_send_data(packet, desc);

  return rec;
}


void                 *gdbwrap_writememory(la32 linaddr, void *value,
					  unsigned bytes, gdbwrap_t *desc)
{
  char               packet[50];
  uint8_t            packetsize;
  char               *rec;

  snprintf(packet, sizeof(packet), "%s%x%s%x%s", GDBWRAP_MEMWRITE,
	   linaddr, GDBWRAP_SEP_COMMA, bytes, GDBWRAP_SEP_COLON);
  packetsize = strlen(packet);
  /* GDB protocol expects the value we send to be a "Binary value", ie
     not converted to a char. */
  memcpy(packet + packetsize, value, bytes);
  packet[packetsize + bytes] = GDBWRAP_NULL_CHAR;

  rec = gdbwrap_send_data(packet, desc);
  return rec;
}


char                 *gdbwrap_own_command(char *command,
					  gdbwrap_t *desc)
{
  printf("Received command in %s: %s - size: %d\n", __PRETTY_FUNCTION__,
	 command, strlen(command));
  /* This is hacky. I'll remove this shit. */
  command[strlen(command) - 1] = GDBWRAP_NULL_CHAR;
  return gdbwrap_send_data(command, desc);
}


void                 gdbwrap_writereg(ureg32 regNum, la32 val, gdbwrap_t *desc)
{
  char               regpacket[50];
  
  snprintf(regpacket, sizeof(regpacket), "%s%x=%x",
	   GDBWRAP_WRITEREG, regNum, val);
  gdbwrap_send_data(regpacket, desc);
}


void                 gdbwrap_writereg2(ureg32 regNum, la32 val, gdbwrap_t *desc)
{
  char               *ret;
  gdbwrap_gdbreg32   *reg;
  unsigned           offset;
  char               locreg[700];
  

  reg = gdbwrap_readgenreg(desc);
  ret = gdbwrap_lastmsg(desc);
  offset = 2 * regNum * sizeof(ureg32);
  printf("Value received: %s - %#x - %d\n", ret, val, offset);

  snprintf(locreg, sizeof(locreg), "%x", gdbwrap_little_endian(val));
  memcpy(ret + offset, locreg, 2 * sizeof(ureg32));
  printf("Ret has value: %s\n", ret);
  snprintf(locreg, sizeof(locreg), "%s%s",
	   GDBWRAP_WGENPURPREG, ret);
  printf("locreg has value: %s\n", locreg);
  gdbwrap_send_data(locreg, desc);
  printf("Value received: %s - %#x \n", locreg, gdbwrap_little_endian(val - 1));
  
}


/*
 * Here's the format of a signal:
 *
 * $vCont;C<signum>[:process_pid]#<checksum>
 *
 * Note that que process pid can be retrieved with a "X" command. If
 * process_pid is omited, then we apply to the current process
 * (default behavior).
 */
 
void                 gdbwrap_signal(int signal, gdbwrap_t *desc)
 {
   char              *rec;
   char              signalpacket[50];

   snprintf(signalpacket, sizeof(signalpacket), "%s;C%.2x",
	    GDBWRAP_CONTINUEWITH, signal);
   rec = gdbwrap_send_data(signalpacket, desc);
 }


/* Shall we stepi with a known address ? */
void                 gdbwrap_stepi(gdbwrap_t *desc)
{
  char               *rec;

  rec = gdbwrap_send_data(GDBWRAP_STEPI, desc);

  if (gdbwrap_is_active(desc))
    gdbwrap_populate_reg(rec, desc);
  else
    gdbwrap_errorhandler(GDBWRAP_DEAD, desc);
}


void                 gdbwrap_vmwareinit(gdbwrap_t *desc)
{
  char               *rec;

  gdbwrap_hello(desc);
  gdbwrap_reason_halted(desc);
  rec = gdbwrap_readmemory(desc->reg32.eip, QWORD_IN_BYTE, desc);
}


void                 gdbwrap_test(gdbwrap_t *desc)
{
/*   gdbwrap_memorycontent(0xb7fc99a0 , QWORD_IN_BYTE, desc); */
/*   gdbwrap_writereg(0x183838, 0xff1234, desc); */
  char               *rec;

  unsigned           u = 0xffffffff;
  gdbwrap_writememory(0xb7f959a0, &u, QWORD_IN_BYTE, desc);
  printf("Hiha, Reading the data\n");
  fflush(stdout);
  rec = gdbwrap_readmemory(0xb7f959a0, QWORD_IN_BYTE, desc);
  printf("Hiha, received: %s\n", rec);
    fflush(stdout);
  /* gdbwrap_signal(01, desc); */
}


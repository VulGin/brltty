/*
 * BRLTTY - A background process providing access to the Linux console (when in
 *          text mode) for a blind person using a refreshable braille display.
 *
 * Copyright (C) 1995-2004 by The BRLTTY Team. All rights reserved.
 *
 * BRLTTY comes with ABSOLUTELY NO WARRANTY.
 *
 * This is free software, placed under the terms of the
 * GNU General Public License, as published by the Free Software
 * Foundation.  Please see the file COPYING for details.
 *
 * Web Page: http://mielke.cc/brltty/
 *
 * This software is maintained by Dave Mielke <dave@mielke.cc>.
 */

/* brlapi.c : Main file for BrlApi library */

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif /* HAVE_CONFIG_H */

#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <stddef.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <pthread.h>
#include <signal.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/select.h>
#include <sys/time.h>

#include "api.h"
#include "api_protocol.h"
#include "api_common.h"
#include "rangelist.h"
#include "brl.h"
#include "brltty.h"
#include "misc.h"
#include "scr.h"
#include "tunes.h"

#define UNAUTH_MAX 5
#define UNAUTH_DELAY 30

/* This defines the message that is initially prepared to be printed */
/* on the braille display, when a client takes control of a tty */
#define INITIAL_MESSAGE "Connected to BrlApi"

typedef enum {
  PARM_HOST,
  PARM_KEYFILE
} Parameters;

const char *const api_parameters[] = { "host", "keyfile", NULL };

#define RELEASE "BRLTTY API Library: release " BRLAPI_RELEASE
#define COPYRIGHT "   Copyright Sebastien HINDERER <Sebastien.Hinderer@ens-lyon.org>, \
Samuel THIBAULT <samuel.thibault@ens-lyon.org>"

/* These CHECK* macros check whether a condition is true, and, if not, */
/* send back either a non-fatal error, or an excepti)n */
#define CHECKERR(condition, error) \
if (!( condition )) { \
  writeError(c->fd, error); \
  return 0; \
} else { }
#define CHECKEXC(condition, error) \
if (!( condition )) { \
  writeException(c->fd, error, type, packet, size); \
  return 0; \
} else { }

#define WERR(x, y) writeError(x, y)
#define WEXC(x, y) writeException(x, y, type, packet, size)

#ifdef brlapi_errno
#undef brlapi_errno
#endif

int brlapi_errno;
int *brlapi_errno_location(void) { return &brlapi_errno; }

/****************************************************************************/
/** GLOBAL TYPES AND VARIABLES                                              */
/****************************************************************************/

extern char *opt_brailleParameters;
extern char *cfg_brailleParameters;

typedef enum { FULL, EMPTY} TBrlBufState;

typedef struct Tconnection {
  struct Tconnection *prev, *next;
  int fd;
  int auth;
  int tty;
  int raw;
  uint32_t how; /* how keys must be delivered to clients */
  BrailleDisplay brl;
  unsigned int cursor;
  TBrlBufState brlbufstate;
  pthread_mutex_t brlmutex;
  rangeList *unmaskedKeys;
  pthread_mutex_t maskmutex;
  long upTime;
} Tconnection;

static int connectionsAllowed = 0;

/* Pointer on the connection accepter thread */
static pthread_t serverThread; /* server */

static pthread_mutex_t connections_mutex = PTHREAD_MUTEX_INITIALIZER;
static Tconnection *connections = NULL;

static unsigned int unauthConnections;
static unsigned int unauthConnLog = 0;

/* These variables let access to some information be synchronized */
static pthread_mutex_t packet_mutex = PTHREAD_MUTEX_INITIALIZER;
static Tconnection *rawConnection = NULL;

/* Pointer on subroutines of the real braille driver */
static const BrailleDriver *TrueBraille;
static BrailleDriver ApiBraille;

/* Identication of the REAL braille driver currently used */
static uint32_t DisplaySize[2] = { 0, 0 };

static size_t authKeyLength = 0;
static unsigned char authKey[BRLAPI_MAXPACKETSIZE];

/****************************************************************************/
/** SOME PROTOTYPES                                                        **/
/****************************************************************************/

extern void processParameters(char ***values, const char *const *names, const char *description, char *optionParameters, char *configuredParameters, const char *environmentVariable);
static int initializeunmaskedKeys(Tconnection *c);

/****************************************************************************/
/** SIGNALS HANDLING                                                       **/
/****************************************************************************/

/****************************************************************************/
/** PACKET MANAGING                                                        **/
/****************************************************************************/

/* Function : writeAck */
/* Sends an acknowledgement on the given socket */
static inline void writeAck(int fd)
{
  brlapi_writePacket(fd,BRLPACKET_ACK,NULL,0);
}

/* Function : writeError */
/* Sends the given non-fatal error on the given socket */
static void writeError(int fd, uint32_t err)
{
  uint32_t code = htonl(err);
  brlapi_writePacket(fd,BRLPACKET_ERROR,&code,sizeof(code));
}


/* Function : writeException */
/* Sends the given error code on the given socket */
static void writeException(int fd, uint32_t err, brl_type_t type, const void *packet, size_t size)
{
  int hdrsize, esize;
  unsigned char epacket[BRLAPI_MAXPACKETSIZE];
  errorPacket_t * errorPacket = (errorPacket_t *) epacket;
  hdrsize = sizeof(errorPacket->code)+sizeof(errorPacket->type);
  errorPacket->code = htonl(err);
  errorPacket->type = htonl(type);
  esize = MIN(size, BRLAPI_MAXPACKETSIZE-hdrsize);
  if ((packet!=NULL) && (size!=0)) memcpy(&errorPacket->packet, packet, esize);
  brlapi_writePacket(fd,BRLPACKET_EXCEPTION,epacket, hdrsize+esize);
}

/****************************************************************************/
/** COMMUNICATION PROTOCOLE HANDLING                                       **/
/****************************************************************************/

/* Function : createConnection */
/* Creates a struct of type Tconnection and stores suitable values */
/* x and y correspond to the braille display size */
/* This function also records the connection in an array */
/* If an error occurs, one returns NULL, and an error message is written on */
/* the socket before closing it */
static Tconnection *createConnection(int fd,int x, int y, long currentTime)
{
  Tconnection *c =  (Tconnection *) malloc(sizeof(Tconnection));
  if (c==NULL) {
    writeError(fd,BRLERR_NOMEM);
    close(fd);
    return NULL;
  }
  if (x*y > 0) {
    c->brl.buffer = (unsigned char *) malloc(x*y);
    if (c->brl.buffer==NULL) {
      free(c);
      writeError(fd,BRLERR_NOMEM);
      close(fd);
      return NULL;
    }
  } else c->brl.buffer = NULL;
  c->brl.x = x; c->brl.y = y;
  c->auth = 0;
  c->fd = fd;
  c->tty = -1;
  c->raw = 0;
  c->cursor = 0;
  c->brlbufstate = EMPTY;
  pthread_mutex_init(&c->brlmutex,NULL);
  pthread_mutex_init(&c->maskmutex,NULL);
  c->how = 0;
  c->unmaskedKeys = NULL;
  c->upTime = currentTime;
  unauthConnections++;
  return c;
}

/* Function : freeConnection */
/* Frees all resources associated to a connection */
static void freeConnection(Tconnection *c)
{
  pthread_mutex_destroy(&c->brlmutex);
  pthread_mutex_destroy(&c->maskmutex);
  if (c->brl.buffer!=NULL) free(c->brl.buffer);
  freeRangeList(&c->unmaskedKeys);
  free(c);
}

/* Function : addConnection */
/* Creates a connection and adds it to the connection list */
static Tconnection *addConnection(int fd, int x, int y, long currentTime)
{
  Tconnection *c = createConnection(fd, x, y, currentTime);
  if (c==NULL) return NULL;
  pthread_mutex_lock(&connections_mutex);
  c->next = connections->next;
  c->prev = connections;
  connections->next->prev = c;
  connections->next = c;
  pthread_mutex_unlock(&connections_mutex);
  return c;
}

/* Function : removeConnection */
/* Closes the socket, removesthe connection fromthe list */
/* and frees iss ressources */
static void removeConnection(Tconnection *c)
{
  close(c->fd);
  pthread_mutex_lock(&connections_mutex);
  c->prev->next = c->next;
  c->next->prev = c->prev;
  pthread_mutex_unlock(&connections_mutex);
  freeConnection(c);
}

/* Function LogPrintRequest */
/* Logs the given request */
static inline void LogPrintRequest(int type, int fd)
{
  LogPrint(LOG_DEBUG, "Received %s request on fd %d", brlapi_packetType(type), fd);  
}

/* Function : processRequest */
/* Reads a packet fro c->fd and processes it */
/* Returns 1 if connection has to be removed */
/* If EOF is reached, closes fd and frees all associated ressources */
static int processRequest(Tconnection *c)
{
  int i;
  ssize_t size;
  unsigned char packet[BRLAPI_MAXPACKETSIZE];
  authStruct *auth = (authStruct *) packet;
  uint32_t * ints = (uint32_t *) &packet[0];
  brl_type_t type;
  Tconnection *conn;
  size = brlapi_readPacket(c->fd,&type,packet,BRLAPI_MAXPACKETSIZE);
  if (size<0) {
    if (size==-1) LogPrint(LOG_WARNING,"read : %s (connection on fd %d)",strerror(errno),c->fd);
    else {
      LogPrint(LOG_DEBUG,"Closing connection on fd %d",c->fd);
    }
    if (c->raw) {
      c->raw = 0;
      rawConnection = NULL;
      LogPrint(LOG_WARNING,"Client on fd %d did not give up raw mode properly",c->fd);
      LogPrint(LOG_WARNING,"Trying to reset braille terminal");
      if (!TrueBraille->reset(&c->brl)) {
        LogPrint(LOG_WARNING,"Reset failed. Restarting braille driver");
        restartBrailleDriver();
      }
    }
    if (c->tty!=-1) {
      LogPrint(LOG_WARNING,"Client on fd %d did not give up control of tty %d properly",c->fd,c->tty);
      /* One frees the terminal so as brltty to take back its control as soon as */
      /* possible... */
      pthread_mutex_lock(&connections_mutex);
      c->tty = -1;
      freeRangeList(&c->unmaskedKeys);
      pthread_mutex_unlock(&connections_mutex);
    }
    return 1;
  }
  if (c->auth==0) {
    if ((type==BRLPACKET_AUTHKEY) && (size==sizeof(auth->protocolVersion)+authKeyLength) && (auth->protocolVersion==BRLAPI_PROTOCOL_VERSION) && (!memcmp(&auth->key, authKey, authKeyLength))) {
      writeAck(c->fd);
      c->auth = 1;
      unauthConnections--;
      return 0;
    } else {
      writeError(c->fd, BRLERR_CONNREFUSED);
      return 1;
    }
  }
  switch (type) {
    case BRLPACKET_GETTTY: {
      uint32_t tty, how, n;
      unsigned char *p = INITIAL_MESSAGE;
      LogPrintRequest(type, c->fd);
      CHECKEXC((!c->raw),BRLERR_ILLEGAL_INSTRUCTION);
      CHECKEXC((size==2*sizeof(uint32_t)),BRLERR_INVALID_PACKET);
      how = ntohl(ints[1]);
      CHECKEXC(((how == BRLKEYCODES) || (how == BRLCOMMANDS)),BRLERR_INVALID_PARAMETER);
      tty = ntohl(ints[0]);
      if (c->tty==tty) {
        if (c->how==how) {
          LogPrint(LOG_WARNING,"One already controls tty %d",tty);
          writeAck(c->fd);
        } else {
          /* Here one is in the case where the client tries to change */
          /* from BRLKEYCODES to BRLCOMMANDS, or something like that */
          /* For the moment this operation is not supported */
          /* A client that wants to do that should first LeaveTty() */
          /* and then get it again, risking to lose it */
          LogPrint(LOG_INFO,"Switching from BRLKEYCODES to BRLCOMMANDS not supported yet");
          WERR(c->fd,BRLERR_OPNOTSUPP);
        }
        return 0;
      }
      if (how==BRLKEYCODES) { /* We must check if the braille driver supports that */
        if ((TrueBraille->readKey==NULL) || (TrueBraille->keyToCommand==NULL)) {
          WEXC(c->fd, BRLERR_KEYSNOTSUPP);
          return 0;
        }
      }
      for (conn=connections->next; conn!=connections; conn = conn->next)
        if (conn->tty == tty) break;
      if (conn!=connections) {
        LogPrint(LOG_WARNING,"tty %d is already controled by someone else",tty);
        WERR(c->fd,BRLERR_TTYBUSY);
      } else {
        c->tty = tty;
        c->how = how;
        if (initializeunmaskedKeys(c)==-1) {
          LogPrint(LOG_WARNING,"Failed to initialize unmasked keys");
          freeRangeList(&c->unmaskedKeys);
          WERR(c->fd,BRLERR_NOMEM);
        } else {
          /* The following code fills braille display with blank spaces */
          /* this avoids random display, when client sends no text immediately */
          n = MIN(strlen(p),c->brl.x*c->brl.y);
          for (i=0; i<n; i++) *(c->brl.buffer+i) = textTable[*(p+i)];
          for (i=n; i<c->brl.x*c->brl.y; i++) *(c->brl.buffer+i) = textTable[' '];
          c->brlbufstate = FULL;
          writeAck(c->fd);
          LogPrint(LOG_DEBUG,"Taking control of tty %d (how=%d)",tty,how);
        }
      }
      return 0;
    }
    case BRLPACKET_LEAVETTY: {
      LogPrintRequest(type, c->fd);
      CHECKEXC(!c->raw,BRLERR_ILLEGAL_INSTRUCTION);
      CHECKEXC(c->tty!=-1,BRLERR_ILLEGAL_INSTRUCTION);
      LogPrint(LOG_DEBUG,"Releasing tty %d",c->tty);
      c->tty = -1;
      freeRangeList(&c->unmaskedKeys);
      writeAck(c->fd);
      return 0;
    }
    case BRLPACKET_IGNOREKEYRANGE:
    case BRLPACKET_UNIGNOREKEYRANGE: {
      int res;
      brl_keycode_t x,y;
      LogPrintRequest(type, c->fd);
      CHECKEXC(( (!c->raw) && (c->tty!=-1) ),BRLERR_ILLEGAL_INSTRUCTION);
      CHECKEXC(size==2*sizeof(brl_keycode_t),BRLERR_INVALID_PACKET);
      x = ntohl(ints[0]);
      y = ntohl(ints[1]);
      LogPrint(LOG_DEBUG,"range: [%u..%u]",x,y);
      pthread_mutex_lock(&c->maskmutex);
      if (type==BRLPACKET_IGNOREKEYRANGE) res = removeRange(x,y,&c->unmaskedKeys);
      else res = addRange(x,y,&c->unmaskedKeys);
      pthread_mutex_unlock(&c->maskmutex);
      if (res==-1) WERR(c->fd,BRLERR_NOMEM);
      else writeAck(c->fd);
      return 0;
    }
    case BRLPACKET_IGNOREKEYSET:
    case BRLPACKET_UNIGNOREKEYSET: {
      int i = 0, res = 0;
      uint32_t nbkeys;
      brl_keycode_t *k = (brl_keycode_t *) &packet;
      int (*fptr)(uint32_t, uint32_t, rangeList **);
      LogPrintRequest(type, c->fd);
      if (type==BRLPACKET_IGNOREKEYSET) fptr = removeRange; else fptr = addRange;
      CHECKEXC(( (!c->raw) && (c->tty!=-1) ),BRLERR_ILLEGAL_INSTRUCTION);
      CHECKEXC(size % sizeof(brl_keycode_t)==0,BRLERR_INVALID_PACKET);
      nbkeys = size/sizeof(brl_keycode_t);
      pthread_mutex_lock(&c->maskmutex);
      while ((res!=-1) && (i<nbkeys)) {
        res = fptr(k[i],k[i],&c->unmaskedKeys);
        i++;
      }
      pthread_mutex_unlock(&c->maskmutex);
      if (res==-1) WERR(c->fd,BRLERR_NOMEM);
      else writeAck(c->fd);
      return 0;
    }
    case BRLPACKET_STATWRITE: {
      LogPrintRequest(type, c->fd);
      CHECKEXC(((!c->raw)&&(c->tty!=-1)),BRLERR_ILLEGAL_INSTRUCTION);
      WERR(c->fd,BRLERR_OPNOTSUPP);
      return 0;
    }
    case BRLPACKET_EXTWRITE: {
      extWriteStruct *ews = (extWriteStruct *) packet;
      uint32_t dispSize, cursor, rbeg, rend, strLen;
      unsigned char *p = &ews->data;
      unsigned char buf[200]; /* dirty! */
      LogPrintRequest(type, c->fd);
      CHECKEXC(size>=sizeof(ews->flags), BRLERR_INVALID_PACKET);
      CHECKEXC(((!c->raw)&&(c->tty!=-1)),BRLERR_ILLEGAL_INSTRUCTION);
      pthread_mutex_lock(&c->brlmutex);
      cursor = c->cursor;
      dispSize = c->brl.x*c->brl.y;
      memcpy(buf, c->brl.buffer, dispSize);
      pthread_mutex_unlock(&c->brlmutex);
      size -= sizeof(ews->flags); /* flags */
      CHECKERR((ews->flags & BRLAPI_EWF_DISPLAYNUMBER)==0, BRLERR_OPNOTSUPP);
      if (ews->flags & BRLAPI_EWF_REGION) {
        CHECKEXC(size>2*sizeof(uint32_t), BRLERR_INVALID_PACKET);
        rbeg = ntohl( *((uint32_t *) p) );
        p += sizeof(uint32_t); size -= sizeof(uint32_t); /* region begin */
        rend = ntohl( *((uint32_t *) p) );
        p += sizeof(uint32_t); size -= sizeof(uint32_t); /* region end */
        CHECKEXC(
          (1<=rbeg) && (rbeg<=dispSize) && (1<=rend) && (rend<=dispSize)
          && (rbeg<=rend), BRLERR_INVALID_PARAMETER);
      } else {
        rbeg = 1;
        rend = dispSize;
      }
      strLen = (rend-rbeg) + 1;
      if (ews->flags & BRLAPI_EWF_TEXT) {
        CHECKEXC(size>=strLen, BRLERR_INVALID_PACKET);
        for (i=rbeg-1; i<rend; i++)
          buf[i] = textTable[*(p+i)];
        p += strLen; size -= strLen; /* text */
      }
      if (ews->flags & BRLAPI_EWF_ATTR_AND) {
        CHECKEXC(size>=strLen, BRLERR_INVALID_PACKET);
        for (i=rbeg-1; i<rend; i++)
          buf[i] &= *(p+i);
        p += strLen; size -= strLen; /* and attributes */
      }
      if (ews->flags & BRLAPI_EWF_ATTR_OR) {
        CHECKEXC(size>=strLen, BRLERR_INVALID_PACKET);
        for (i=rbeg-1; i<rend; i++)
          buf[i] |= *(p+i);
        p += strLen; size -= strLen; /* or attributes */
      }
      if (ews->flags & BRLAPI_EWF_CURSOR) {
        CHECKEXC(size>=sizeof(uint32_t), BRLERR_INVALID_PACKET);
        cursor = ntohl( *((uint32_t *) p) );
        p += sizeof(uint32_t); size -= sizeof(uint32_t); /* cursor */
        CHECKEXC(cursor<=dispSize, BRLERR_INVALID_PACKET);
      }
      CHECKEXC(size==0, BRLERR_INVALID_PACKET);
      /* Here all the packet has been processed. */
      /* We can now set the cursor if any, and update the actual buffer */
      /* with the new information to display */
      if (cursor) buf[cursor-1] |= cursorDots();
      pthread_mutex_lock(&c->brlmutex);
      c->cursor = cursor;
      memcpy(c->brl.buffer, buf, dispSize);
      c->brlbufstate = FULL;
      pthread_mutex_unlock(&c->brlmutex);
      return 0;
    }
    case BRLPACKET_GETRAW: {
      uint32_t rawMagic;
      LogPrintRequest(type, c->fd);
      CHECKEXC(((TrueBraille->readPacket!=NULL) && (TrueBraille->writePacket!=NULL)), BRLERR_RAWNOTSUPP);
      if (c->raw) {
        LogPrint(LOG_DEBUG,"satisfied immediately since one already is in raw mode");
        writeAck(c->fd);
        return 0;
      }
      rawMagic = ntohl(ints[0]);
      CHECKEXC(rawMagic==BRLRAW_MAGIC,BRLERR_INVALID_PARAMETER);
      CHECKERR(rawConnection==NULL,BRLERR_RAWMODEBUSY);
      c->raw = 1;
      rawConnection = c;
      writeAck(c->fd);
      return 0;
    }
    case BRLPACKET_LEAVERAW: {
      LogPrintRequest(type, c->fd);
      CHECKEXC(c->raw,BRLERR_ILLEGAL_INSTRUCTION);
      LogPrint(LOG_DEBUG,"Going out of raw mode");
      c->raw = 0;
      rawConnection = NULL;
      writeAck(c->fd);
      return 0;
    }
    case BRLPACKET_PACKET: {
      LogPrintRequest(type, c->fd);
      CHECKEXC(c->raw,BRLERR_ILLEGAL_INSTRUCTION);
      pthread_mutex_lock(&packet_mutex);
      TrueBraille->writePacket(&c->brl,packet,size);
      pthread_mutex_unlock(&packet_mutex);
      return 0;
    }
    case BRLPACKET_GETDRIVERID: {
      LogPrintRequest(type, c->fd);
      CHECKEXC(size==0,BRLERR_INVALID_PACKET);
      CHECKEXC(!c->raw,BRLERR_ILLEGAL_INSTRUCTION);
      brlapi_writePacket(c->fd,BRLPACKET_GETDRIVERID,braille->identifier,strlen(braille->identifier)+1);
      return 0;
    }
    case BRLPACKET_GETDRIVERNAME: {
      LogPrintRequest(type, c->fd);
      CHECKEXC(size==0,BRLERR_INVALID_PACKET);
      CHECKEXC(!c->raw,BRLERR_ILLEGAL_INSTRUCTION);
      brlapi_writePacket(c->fd,BRLPACKET_GETDRIVERNAME,braille->name,strlen(braille->name)+1);
      return 0;
      }
    case BRLPACKET_GETDISPLAYSIZE: {
      LogPrintRequest(type, c->fd);
      CHECKEXC(size==0,BRLERR_INVALID_PACKET);
      CHECKEXC(!c->raw,BRLERR_ILLEGAL_INSTRUCTION);
      brlapi_writePacket(c->fd,BRLPACKET_GETDISPLAYSIZE,&DisplaySize[0],sizeof(DisplaySize));
      return 0;
    }
    default:
      WEXC(c->fd,BRLERR_UNKNOWN_INSTRUCTION);
  }
  return 0;
}

/****************************************************************************/
/** SOCKETS AND CONNECTIONS MANAGING                                       **/
/****************************************************************************/

/* Function : initializeSocket */
/* Creates the listening socket for in-connections */
/* Returns the descriptor, or -1 if an error occurred */
static int initializeSocket(const char *host)
{
  int fd=-1, err, yes=1;
  struct addrinfo *res,*cur;
  struct addrinfo hints;
  char *hostname,*port;

  memset(&hints, 0, sizeof(hints));
  hints.ai_flags = AI_PASSIVE;
  hints.ai_family = PF_UNSPEC;
  hints.ai_socktype = SOCK_STREAM;

  brlapi_splitHost(host,&hostname,&port);

  err = getaddrinfo(hostname, port, &hints, &res);
  if (hostname)
    free(hostname);
  free(port);
  if (err) {
    LogPrint(LOG_WARNING,"getaddrinfo(%s:%s,%s) : %s",host,hostname,port,gai_strerror(err));
    return -1;
  }
  for (cur = res; cur; cur = cur->ai_next) {
    fd = socket(cur->ai_family, cur->ai_socktype, cur->ai_protocol);
    if (fd<0) {
      if (errno != EAFNOSUPPORT)
        LogPrint(LOG_WARNING,"socket : %s",strerror(errno));
      continue;
    }
    /* Specifies that address can be reused */
    if (setsockopt(fd,SOL_SOCKET,SO_REUSEADDR,&yes,sizeof(yes))!=0) {
      LogError("setsockopt");
      close(fd);
       continue;
    }
    if (bind(fd, cur->ai_addr, cur->ai_addrlen)<0) {
      if (errno!=EADDRNOTAVAIL && errno!=EADDRINUSE) {
        LogPrint(LOG_WARNING,"bind : %s",strerror(errno));
        close(fd);
        continue;
      }
      do {
        sleep(1);
        err = bind(fd, cur->ai_addr, cur->ai_addrlen);
      } while ((err == -1) && (errno==EADDRNOTAVAIL));
    }
    if (listen(fd,1)<0) {
      LogPrint(LOG_WARNING,"listen : %s",strerror(errno));
      close(fd);
      continue;
    }
    break;
  }
  freeaddrinfo(res);
  if (cur) return fd;
  LogPrint(LOG_WARNING,"unable to find a local TCP port %s !",host);
  return -1;
}

/* Function : server */
/* The server thread */
/* Returns NULL in any case */
static void *server(void *arg)
{
  int res, entryPoint, fdmax;
  fd_set sockets;
  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);
  char *host = (char *) arg;
  Tconnection *c;
  sigset_t blockedSignals;
  long currentTime;
  struct timeval tv;
  sigemptyset(&blockedSignals);
  sigaddset(&blockedSignals,SIGTERM);
  sigaddset(&blockedSignals,SIGINT);
  sigaddset(&blockedSignals,SIGPIPE);
  sigaddset(&blockedSignals,SIGCHLD);
  sigaddset(&blockedSignals,SIGUSR1);
  if ((res = pthread_sigmask(SIG_BLOCK,&blockedSignals,NULL))!=0) {
    LogPrint(LOG_WARNING,"pthread_sigmask : %s",strerror(res));
    pthread_exit(NULL);
  }
  if ((entryPoint = initializeSocket(host))==-1) {
    LogPrint(LOG_WARNING,"Error while initializing socket : %s",strerror(errno));
    return NULL;
  }
  unauthConnections = 0; unauthConnLog = 0;
  while (1) {
    /* Compute sockets set and fdmax */
    FD_ZERO(&sockets);
    FD_SET(entryPoint, &sockets);
    fdmax = entryPoint;
    for (c = connections->next; c!=connections; c = c->next) {
      if (c->fd>fdmax) fdmax = c->fd;
      FD_SET(c->fd,&sockets);
    }
    tv.tv_sec = 1; tv.tv_usec = 0;
    if (select(fdmax+1, &sockets, NULL, NULL, &tv)<0)
    {
      LogPrint(LOG_WARNING,"select: %s",strerror(errno));
      pthread_exit(NULL);
    }
    gettimeofday(&tv, NULL); currentTime = tv.tv_sec;
    if (FD_ISSET(entryPoint, &sockets)) {
      addrlen=sizeof(addr);
      LogPrint(LOG_DEBUG,"Incoming connection attempt detected");
      res = accept(entryPoint, &addr, &addrlen);
      if (res<0) {
        LogPrint(LOG_WARNING,"accept: %s",strerror(errno));
        close(entryPoint);
        pthread_exit(NULL);
      }
      LogPrint(LOG_DEBUG,"Connection accepted on fd %d",res);
      if (unauthConnections>=UNAUTH_MAX) {
        writeError(res, BRLERR_CONNREFUSED);
        close(res);
        if (unauthConnLog==0) LogPrint(LOG_WARNING, "Too many simultaneous unauthenticated connections");
        unauthConnLog++;
      } else {
        c = addConnection(res,ntohl(DisplaySize[0]),ntohl(DisplaySize[1]),currentTime);
        if (c==NULL) {
          LogPrint(LOG_WARNING,"Failed to create connection structure");
          close(res);
        }
      }
    }
    c = connections->next;
    while (c!=connections) {
      int remove = 0;
      if (FD_ISSET(c->fd, &sockets))    remove = processRequest(c);
      else remove = c->auth==0 && currentTime-(c->upTime) > UNAUTH_DELAY;
      c = c->next;
      if (remove) removeConnection(c->prev);
    }
  }
  return NULL;
}

/****************************************************************************/
/** MISCELLANEOUS FUNCTIONS                                                **/
/****************************************************************************/

/* Function : initializeunmaskedKeys */
/* Specify which keys should be passed to the client by default, as soon */
/* as he controls the tty */
/* If clients asked for commands, one lets him process routing cursor */
/* and screen-related commands */
/* If the client is interested in braille codes, one passes him every key */
static int initializeunmaskedKeys(Tconnection *c)
{
  if (c==NULL) return 0;
  if (addRange(0,BRL_KEYCODE_MAX,&c->unmaskedKeys)==-1) return -1;
  if (c->how==BRLKEYCODES) return 0;
  if (removeRange(CMD_SWITCHVT_PREV,CMD_SWITCHVT_NEXT,&c->unmaskedKeys)==-1) return -1;
  if (removeRange(CMD_RESTARTBRL,CMD_RESTARTSPEECH,&c->unmaskedKeys)==-1) return -1;
  if (removeRange(CR_SWITCHVT,CR_SWITCHVT|0XFF,&c->unmaskedKeys)==-1) return -1;
  return 0;
}

/* Function : terminationHandler */
/* Terminates driver */
static void terminationHandler()
{
  int res;
  if (connectionsAllowed!=0) {
    if ((res = pthread_cancel(serverThread))!=0) {
      LogPrint(LOG_WARNING,"pthread_cancel (1) : %s",strerror(res));
    } else if ((res = pthread_join(serverThread,NULL))!=0) {
      LogPrint(LOG_WARNING,"pthread_join : %s",strerror(res));
    }
    while (connections->next!=connections) removeConnection(connections->next);
  }
  api_unlink();
}

/* Function : api_writeWindow */
static void api_writeWindow(BrailleDisplay *brl)
{
  int tty;
  Tconnection *c;
  if (rawConnection!=NULL) return;
  pthread_mutex_lock(&connections_mutex);
  tty = currentVirtualTerminal();
  for (c=connections->next; c!=connections; c=c->next) if (c->tty==tty) {
    pthread_mutex_unlock(&connections_mutex);
    return;
  }
  pthread_mutex_unlock(&connections_mutex);
  TrueBraille->writeWindow(brl);
}

/* Function : api_readCommand */
static int api_readCommand(BrailleDisplay *disp, DriverCommandContext caller)
{
  int tty,res,refresh = 0, masked;
  ssize_t size;
  static int oldtty = 0;
  Tconnection *c = NULL;
  unsigned char packet[BRLAPI_MAXPACKETSIZE];
  brl_keycode_t keycode = 0;

  if (rawConnection!=NULL) {
    pthread_mutex_lock(&packet_mutex);
    size = TrueBraille->readPacket(&c->brl,packet,BRLAPI_MAXPACKETSIZE);
    pthread_mutex_unlock(&packet_mutex);
    if (size>0) {
      brlapi_writePacket(rawConnection->fd,BRLPACKET_PACKET,packet,size);
    }
    return EOF;
  }
  pthread_mutex_lock(&connections_mutex);
  tty = currentVirtualTerminal();
  if (tty!=oldtty) {
    refresh = 1;
    oldtty = tty;
  }
  for (c=connections->next; c!=connections; c=c->next) if (c->tty==tty) break;
  pthread_mutex_unlock(&connections_mutex);
  if (c!=connections) {
    pthread_mutex_lock(&c->brlmutex);
    if ((c->brlbufstate==FULL) || (refresh)) {
      TrueBraille->writeWindow(&c->brl);
      c->brlbufstate = EMPTY;
    }
    pthread_mutex_unlock(&c->brlmutex);
    if (c->how==BRLKEYCODES) res = TrueBraille->readKey(&c->brl);
    else res = TrueBraille->readCommand(&c->brl,caller);
    if (res==EOF) return EOF;
    keycode = (brl_keycode_t) res;
    pthread_mutex_lock(&c->maskmutex);
    masked = (contains(c->unmaskedKeys,keycode) == NULL);
    pthread_mutex_unlock(&c->maskmutex);
    if (!masked) {
      if (c->how==BRLKEYCODES) {
        LogPrint(LOG_DEBUG,"Transmitting unmasked key %u",keycode);
        keycode = htonl(keycode);
        brlapi_writePacket(c->fd,BRLPACKET_KEY,&keycode,sizeof(keycode));
      } else {
        LogPrint(LOG_DEBUG,"Transmitting unmasked command %u",keycode);
        keycode = htonl(keycode);
        brlapi_writePacket(c->fd,BRLPACKET_KEY,&keycode,sizeof(keycode));
      }
      return EOF;
    }
    if (c->how==BRLKEYCODES) keycode = TrueBraille->keyToCommand(&c->brl,caller,keycode);
  }
  if (keycode==0) {
    res = TrueBraille->readCommand(disp,caller);
    if (res==EOF) return EOF;
    keycode = (brl_keycode_t) res;
  }
  return keycode;
}

/* Function : api_link */
/* Does all the link stuff to let api get events from the driver and */
/* writes from brltty */
void api_link(void)
{
  TrueBraille=braille;
  memcpy(&ApiBraille,braille,sizeof(BrailleDriver));
  ApiBraille.writeWindow=api_writeWindow;
  ApiBraille.readCommand=api_readCommand;
  ApiBraille.readKey = NULL;
  ApiBraille.keyToCommand = NULL;
  ApiBraille.readPacket = NULL;
  ApiBraille.writePacket = NULL;
  braille=&ApiBraille;
}

/* Function : api_unlink */
/* Does all the unlink stuff to remove api from the picture */
void api_unlink(void)
{
  braille=TrueBraille;
}

/* Function : api_identify */
/* Identifies BrlApi */
void api_identify(void)
{
  LogPrint(LOG_NOTICE, RELEASE);
  LogPrint(LOG_INFO,   COPYRIGHT);
}

/* Function : api_open */
/* Initializes BrlApi */
/* One first initialize the driver */
/* Then one creates the communication socket */
void api_open(BrailleDisplay *brl, char **parameters)
{
  static char *host;
  int res;

  DisplaySize[0] = htonl(brl->x);
  DisplaySize[1] = htonl(brl->y);

  api_link();

  res = brlapi_loadAuthKey((*parameters[PARM_KEYFILE]?parameters[PARM_KEYFILE]:BRLAPI_DEFAUTHPATH),
                           &authKeyLength,authKey);
  if (res==-1) {
    LogPrint(LOG_WARNING,"Unable to load API authentication key: no connections will be accepted.");
    connectionsAllowed = 0;
    return;
  }
  LogPrint(LOG_DEBUG, "Authentication key loaded");
  connections = createConnection(-1,0,0,0);
  if (connections==NULL)
  {
    LogPrint(LOG_WARNING, "Unabl tu create connections list");
    pthread_exit(NULL);
  }
  connections->prev = connections;
  connections->next = connections;

  if (*parameters[PARM_HOST]) host = parameters[PARM_HOST];
  else host = "127.0.0.1";

  if ((res = pthread_create(&serverThread,NULL,server,(void *) host)) != 0) {
    LogPrint(LOG_WARNING,"pthread_create : %s",strerror(res));
    return;
  } else connectionsAllowed = 1;
}

/* Function : api_close */
/* End of BrlApi session. Closes the listening socket */
/* destroys opened connections and associated resources */
/* Closes the driver */
void api_close(BrailleDisplay *brl)
{
  terminationHandler();
}

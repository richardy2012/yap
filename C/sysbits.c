/*************************************************************************
*									 *
*	 YAP Prolog 							 *
*									 *
*	Yap Prolog was developed at NCCUP - Universidade do Porto	 *
*									 *
* Copyright L.Damas, V.S.Costa and Universidade do Porto 1985-1997	 *
*									 *
**************************************************************************
*									 *
* File:		sysbits.c						 *
* Last rev:	4/03/88							 *
* mods:									 *
* comments:	very much machine dependent routines			 *
*									 *
*************************************************************************/
#ifdef SCCS
static char SccsId[] = "%W% %G%";
#endif

/*
 * In this routine we shall try to include the inevitably machine dependant
 * routines. These include, for the moment : Time, A rudimentary form of
 * signal handling, OS calls,
 *
 * Vitor Santos Costa, February 1987
 *
 */

/* windows.h does not like absmi.h, this
   should fix it for now */
#include "absmi.h"
#include "yapio.h"
#include "alloc.h"
#include <math.h>
#if STDC_HEADERS
#include <stdlib.h>
#endif
#if HAVE_WINDOWS_H
#include <windows.h>
#endif
#if HAVE_SYS_TIME_H && !defined(__MINGW32__) && !_MSC_VER
#include <sys/time.h>
#endif
#if HAVE_UNISTD_H
#include <unistd.h>
#endif
#if HAVE_SYS_WAIT_H && !defined(__MINGW32__) && !_MSC_VER
#include <sys/wait.h>
#endif
#if HAVE_STRING_H
#include <string.h>
#endif
#if !HAVE_STRNCAT
#define strncat(X,Y,Z) strcat(X,Y)
#endif
#if !HAVE_STRNCPY
#define strncpy(X,Y,Z) strcpy(X,Y)
#endif
#if HAVE_GETPWNAM
#include <pwd.h>
#endif
#if HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#if HAVE_SYS_TYPES_H
#include <sys/types.h>
#endif
#if HAVE_FCNTL_H
#include <fcntl.h>
#endif
#if  _MSC_VER || defined(__MINGW32__)
#include <windows.h>
/* required for DLL compatibility */
#if HAVE_DIRECT_H
#include <direct.h>
#endif
#include <io.h>
#else
#if HAVE_SYS_PARAM_H
#include <sys/param.h>
#endif
#endif
#if HAVE_LIBREADLINE
#include <readline/readline.h>
#endif

STATIC_PROTO (void InitPageSize, (void));
STATIC_PROTO (void InitTime, (void));
STATIC_PROTO (void InitWTime, (void));
STATIC_PROTO (Int p_sh, (void));
STATIC_PROTO (Int p_shell, (void));
STATIC_PROTO (Int p_system, (void));
STATIC_PROTO (Int p_mv, (void));
STATIC_PROTO (Int p_cd, (void));
STATIC_PROTO (Int p_getcwd, (void));
STATIC_PROTO (Int p_dir_sp, (void));
STATIC_PROTO (void InitRandom, (void));
STATIC_PROTO (Int p_srandom, (void));
STATIC_PROTO (Int p_alarm, (void));
STATIC_PROTO (Int p_getenv, (void));
STATIC_PROTO (Int p_putenv, (void));
STATIC_PROTO (void  set_fpu_exceptions, (int));
#ifdef MACYAP
STATIC_PROTO (int chdir, (char *));
/* #define signal	skel_signal */
#endif /* MACYAP */

#if  __simplescalar__
char yap_pwd[YAP_FILENAME_MAX];
#endif

#if HAVE_SIGNAL

static int             snoozing = FALSE;

#endif

STD_PROTO (void exit, (int));

#ifdef _WIN32
static void
Yap_WinError(char *yap_error)
{
  char msg[256];
  /* Error, we could not read time */
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS,
		  NULL, GetLastError(), 
		  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), msg, 256,
		  NULL);
    Yap_Error(SYSTEM_ERROR, TermNil, "%s: %s", yap_error, msg);
}
#endif /* _WIN32 */


#define is_valid_env_char(C) ( ((C) >= 'a' && (C) <= 'z') || ((C) >= 'A' && \
			       (C) <= 'Z') || (C) == '_' )


static int
dir_separator (int ch)
{
#ifdef MAC
  return (ch == ':');
#elif ATARI || _MSC_VER
  return (ch == '\\');
#elif defined(__MINGW32__) || defined(__CYGWIN__)
  return (ch == '\\' || ch == '/');
#else
  return (ch == '/');
#endif
}

int
Yap_dir_separator (int ch)
{
  return dir_separator (ch);
}

void
Yap_InitSysPath(void) {
  strncpy(Yap_FileNameBuf, SHARE_DIR, YAP_FILENAME_MAX);
#ifdef MAC
  strncat(Yap_FileNameBuf,":", YAP_FILENAME_MAX);
#elif ATARI || _MSC_VER || defined(__MINGW32__)
  strncat(Yap_FileNameBuf,"\\", YAP_FILENAME_MAX);
#else
  strncat(Yap_FileNameBuf,"/", YAP_FILENAME_MAX);
#endif
  strncat(Yap_FileNameBuf, "Yap", YAP_FILENAME_MAX);
  Yap_PutValue(Yap_LookupAtom("system_library_directory"),
	   MkAtomTerm(Yap_LookupAtom(Yap_FileNameBuf)));
}

static Int
p_dir_sp (void)
{
#ifdef MAC
  Term t = MkIntTerm(':');
#elif ATARI || _MSC_VER || defined(__MINGW32__)
  Term t = MkIntTerm('\\');
#else
  Term t = MkIntTerm('/');
#endif

  return(Yap_unify_constant(ARG1,t));
}


static void
InitPageSize(void)
{
#ifdef _WIN32
  SYSTEM_INFO si;
  GetSystemInfo(&si);
  Yap_page_size = si.dwPageSize;
#elif HAVE_UNISTD_H
#ifdef __FreeBSD__
  Yap_page_size = getpagesize();
#elif defined(_AIX)
  Yap_page_size = sysconf(_SC_PAGE_SIZE);
#elif !defined(_SC_PAGESIZE)
  Yap_page_size = getpagesize();
#else
  Yap_page_size = sysconf(_SC_PAGESIZE);
#endif
#else
bla bla
#endif
}

#ifdef SIMICS
#ifdef HAVE_GETRUSAGE
#undef HAVE_GETRUSAGE
#endif
#ifdef HAVE_TIMES
#undef HAVE_TIMES
#endif
#endif /* SIMICS */

#ifdef _WIN32
#if HAVE_GETRUSAGE
#undef HAVE_GETRUSAGE
#endif
#endif

#if HAVE_GETRUSAGE

#if HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif
#if HAVE_SYS_RESOURCE_H
#include <sys/resource.h>
#endif

#if THREADS
#define StartOfTimes (*(ThreadHandle[worker_id].start_of_timesp))
#define last_time    (*(ThreadHandle[worker_id].last_timep))
#else
/* since the point YAP was started */
static struct timeval StartOfTimes;

/* since last call to runtime */
static struct timeval last_time;
#endif

/* store user time in this variable */
static void
InitTime (void)
{
  struct rusage   rusage;

#if THREADS
  ThreadHandle[worker_id].start_of_timesp = (struct timeval *)malloc(sizeof(struct timeval));
  ThreadHandle[worker_id].last_timep = (struct timeval *)malloc(sizeof(struct timeval));
#endif
  getrusage(RUSAGE_SELF, &rusage);
  last_time.tv_sec = StartOfTimes.tv_sec = rusage.ru_utime.tv_sec;
  last_time.tv_usec = StartOfTimes.tv_usec = rusage.ru_utime.tv_usec;
}


UInt
Yap_cputime (void)
{
 struct rusage   rusage;

 getrusage(RUSAGE_SELF, &rusage);
 return((rusage.ru_utime.tv_sec - StartOfTimes.tv_sec)) * 1000 +
   ((rusage.ru_utime.tv_usec - StartOfTimes.tv_usec) / 1000);
}

void Yap_cputime_interval(Int *now,Int *interval)
{
  struct rusage   rusage;

  getrusage(RUSAGE_SELF, &rusage);
  *now = (rusage.ru_utime.tv_sec - StartOfTimes.tv_sec) * 1000 +
    (rusage.ru_utime.tv_usec - StartOfTimes.tv_usec) / 1000;
  *interval = (rusage.ru_utime.tv_sec - last_time.tv_sec) * 1000 +
    (rusage.ru_utime.tv_usec - last_time.tv_usec) / 1000;
  last_time.tv_usec = rusage.ru_utime.tv_usec;
  last_time.tv_sec = rusage.ru_utime.tv_sec;
}

#elif defined(_WIN32)

#ifdef __GNUC__

/* This is stolen from the Linux kernel.
   The problem is that mingw32 does not seem to have acces to div */
#ifndef do_div
#define do_div(n,base) ({ \
	unsigned long __upper, __low, __high, __mod; \
	asm("":"=a" (__low), "=d" (__high):"A" (n)); \
	__upper = __high; \
	if (__high) { \
		__upper = __high % (base); \
		__high = __high / (base); \
	} \
	asm("divl %2":"=a" (__low), "=d" (__mod):"rm" (base), "0" (__low), "1" (__upper)); \
	asm("":"=A" (n):"a" (__low),"d" (__high)); \
	__mod; \
})
#endif

#endif



#include <time.h>

static FILETIME StartOfTimes, last_time;

static clock_t TimesStartOfTimes, Times_last_time;

/* store user time in this variable */
static void
InitTime (void)
{
  HANDLE hProcess = GetCurrentProcess();
  FILETIME CreationTime, ExitTime, KernelTime, UserTime;
  if (!GetProcessTimes(hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime)) {
    /* WIN98 */
    clock_t t;
    t = clock ();
    Times_last_time = TimesStartOfTimes = t;
  } else {
    last_time.dwLowDateTime = UserTime.dwLowDateTime;
    last_time.dwHighDateTime = UserTime.dwHighDateTime;
    StartOfTimes.dwLowDateTime = UserTime.dwLowDateTime;
    StartOfTimes.dwHighDateTime = UserTime.dwHighDateTime;
  }
}

UInt
Yap_cputime (void)
{
  HANDLE hProcess = GetCurrentProcess();
  FILETIME CreationTime, ExitTime, KernelTime, UserTime;
  if (!GetProcessTimes(hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime)) {
    clock_t t;
    t = clock ();
    return(((t - TimesStartOfTimes)*1000) / CLOCKS_PER_SEC);
  } else {
#ifdef __GNUC__
    unsigned long long int t =
      *(unsigned long long int *)&UserTime - 
      *(unsigned long long int *)&StartOfTimes;
    do_div(t,10000);
    return((Int)t);
#endif
#ifdef _MSC_VER
    __int64 t = *(__int64 *)&UserTime - *(__int64 *)&StartOfTimes;
    return((Int)(t/10000));
#endif
  }
}

void Yap_cputime_interval(Int *now,Int *interval)
{
  HANDLE hProcess = GetCurrentProcess();
  FILETIME CreationTime, ExitTime, KernelTime, UserTime;
  if (!GetProcessTimes(hProcess, &CreationTime, &ExitTime, &KernelTime, &UserTime)) {
    clock_t t;
    t = clock ();
    *now = ((t - TimesStartOfTimes)*1000) / CLOCKS_PER_SEC;
    *interval = (t - Times_last_time) * 1000 / CLOCKS_PER_SEC;
    Times_last_time = t;
  } else {
#ifdef __GNUC__
    unsigned long long int t1 =
      *(unsigned long long int *)&UserTime -
      *(unsigned long long int *)&StartOfTimes;
    unsigned long long int t2 =
      *(unsigned long long int *)&UserTime -
      *(unsigned long long int *)&last_time;
    do_div(t1,10000);
    *now = (Int)t1;
    do_div(t2,10000);
    *interval = (Int)t2;
#endif
#ifdef _MSC_VER
    __int64 t1 = *(__int64 *)&UserTime - *(__int64 *)&StartOfTimes;
    __int64 t2 = *(__int64 *)&UserTime - *(__int64 *)&last_time;
    *now = (Int)(t1/10000);
    *interval = (Int)(t2/10000);
#endif
    last_time.dwLowDateTime = UserTime.dwLowDateTime;
    last_time.dwHighDateTime = UserTime.dwHighDateTime;
  }
}

#elif HAVE_TIMES

#if defined(_WIN32)

#include <time.h>

#define TicksPerSec     CLOCKS_PER_SEC

#else

#if HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif

#endif

#if defined(__sun__) && (defined(__svr4__) || defined(__SVR4))

#if HAVE_LIMITS_H
#include <limits.h>
#endif

#define TicksPerSec	CLK_TCK
#endif

#if defined(__alpha) || defined(__FreeBSD__) || defined(__linux__)

#if HAVE_TIME_H
#include <time.h>
#endif

#define TicksPerSec	sysconf(_SC_CLK_TCK)

#endif

#if !TMS_IN_SYS_TIME
#if HAVE_SYS_TIMES_H
#include <sys/times.h>
#endif
#endif

static clock_t StartOfTimes, last_time;

/* store user time in this variable */
static void
InitTime (void)
{
  struct tms t;
  times (&t);
  last_time = StartOfTimes = t.tms_utime;
}

UInt
Yap_cputime (void)
{
  struct tms t;
  times(&t);
  return((t.tms_utime - StartOfTimes)*1000 / TicksPerSec);
}

void Yap_cputime_interval(Int *now,Int *interval)
{
  struct tms t;
  times (&t);
  *now = ((t.tms_utime - StartOfTimes)*1000) / TicksPerSec;
  *interval = (t.tms_utime - last_time) * 1000 / TicksPerSec;
  last_time = t.tms_utime;
}

#else /* HAVE_TIMES */

#ifdef SIMICS

#include <sys/time.h>

/* since the point YAP was started */
static struct timeval StartOfTimes;

/* since last call to runtime */
static struct timeval last_time;

/* store user time in this variable */
static void
InitTime (void)
{
  struct timeval   tp;

  gettimeofday(&tp,NULL);
  last_time.tv_sec = StartOfTimes.tv_sec = tp.tv_sec;
  last_time.tv_usec = StartOfTimes.tv_usec = tp.tv_usec;
}


UInt
Yap_cputime (void)
{
  struct timeval   tp;

  gettimeofday(&tp,NULL);
  if (StartOfTimes.tv_usec > tp.tv_usec)
    return((tp.tv_sec - StartOfTimes.tv_sec - 1) * 1000 +
	   (StartOfTimes.tv_usec - tp.tv_usec) /1000);
  else
    return((tp.tv_sec - StartOfTimes.tv_sec)) * 1000 +
      ((tp.tv_usec - StartOfTimes.tv_usec) / 1000);
}

void Yap_cputime_interval(Int *now,Int *interval)
{
  struct timeval   tp;

  gettimeofday(&tp,NULL);
  *now = (tp.tv_sec - StartOfTimes.tv_sec) * 1000 +
    (tp.tv_usec - StartOfTimes.tv_usec) / 1000;
  *interval = (tp.tv_sec - last_time.tv_sec) * 1000 +
    (tp.tv_usec - last_time.tv_usec) / 1000;
  last_time.tv_usec = tp.tv_usec;
  last_time.tv_sec = tp.tv_sec;
}

#endif /* SIMICS */

#ifdef COMMENTED_OUT
/* This code is not working properly. I left it here to help future ports */
#ifdef MPW

#include <files.h>
#include <Events.h>

#define TicksPerSec 60.0

static double
real_cputime ()
{
  return (((double) TickCount ()) / TicksPerSec);
}

#endif /* MPW */

#ifdef LATTICE

#include "osbind.h"

static long *ptime;

gettime ()
{
  *ptime = *(long *) 0x462;
}

static double
real_cputime ()
{
  long thetime;
  ptime = &thetime;
  xbios (38, gettime);
  return (((double) thetime) / (Getrez () == 2 ? 70 : 60));
}

#endif /* LATTICE */

#ifdef M_WILLIAMS

#include <osbind.h>
#include <xbios.h>

static long *ptime;

static long
readtime ()
{
  return (*((long *) 0x4ba));
}

static double
real_cputime ()
{
  long time;

  time = Supexec (readtime);
  return (time / 200.0);
}

#endif /* M_WILLIAMS */

#ifdef LIGHT

#undef FALSE
#undef TRUE

#include <FileMgr.h>

#define TicksPerSec 60.0

static double
real_cputime ()
{
  return (((double) TickCount ()) / TicksPerSec);
}

#endif /* LIGHT */

#endif /* COMMENTED_OUT */

#endif /* HAVE_GETRUSAGE */

#if HAVE_GETHRTIME

#if HAVE_TIME_H
#include <time.h>
#endif

/* since the point YAP was started */
static hrtime_t StartOfWTimes;

/* since last call to walltime */
#define  LastWtime (*(hrtime_t *)ALIGN_YAPTYPE(LastWtimePtr,hrtime_t))

static void
InitWTime (void)
{
  StartOfWTimes = gethrtime();
}

static void
InitLastWtime(void) {
  /* ask for twice the space in order to guarantee alignment */
  LastWtimePtr = (void *)Yap_AllocCodeSpace(2*sizeof(hrtime_t));
  LastWtime = StartOfWTimes;
}

Int
Yap_walltime (void)
{
  hrtime_t tp = gethrtime();
  /* return time in milliseconds */
  return((Int)((tp-StartOfWTimes)/((hrtime_t)1000000)));

}

void Yap_walltime_interval(Int *now,Int *interval)
{
  hrtime_t tp = gethrtime();
  /* return time in milliseconds */
  *now = (Int)((tp-StartOfWTimes)/((hrtime_t)1000000));
  *interval = (Int)((tp-LastWtime)/((hrtime_t)1000000));
  LastWtime = tp;
}


#elif HAVE_GETTIMEOFDAY

/* since the point YAP was started */
static struct timeval StartOfWTimes;

/* since last call to walltime */
#define LastWtime (*(struct timeval *)LastWtimePtr)

/* store user time in this variable */
static void
InitWTime (void)
{
  gettimeofday(&StartOfWTimes,NULL);
}

static void
InitLastWtime(void) {
  LastWtimePtr = (void *)Yap_AllocCodeSpace(sizeof(struct timeval));
  LastWtime.tv_usec = StartOfWTimes.tv_usec;
  LastWtime.tv_sec = StartOfWTimes.tv_sec;
}


Int
Yap_walltime (void)
{
  struct timeval   tp;

  gettimeofday(&tp,NULL);
  if (StartOfWTimes.tv_usec > tp.tv_usec)
    return((tp.tv_sec - StartOfWTimes.tv_sec - 1) * 1000 +
	   (StartOfWTimes.tv_usec - tp.tv_usec) /1000);
  else
    return((tp.tv_sec - StartOfWTimes.tv_sec)) * 1000 +
      ((tp.tv_usec - LastWtime.tv_usec) / 1000);
}

void Yap_walltime_interval(Int *now,Int *interval)
{
  struct timeval   tp;

  gettimeofday(&tp,NULL);
  *now = (tp.tv_sec - StartOfWTimes.tv_sec) * 1000 +
    (tp.tv_usec - StartOfWTimes.tv_usec) / 1000;
  *interval = (tp.tv_sec - LastWtime.tv_sec) * 1000 +
    (tp.tv_usec - LastWtime.tv_usec) / 1000;
  LastWtime.tv_usec = tp.tv_usec;
  LastWtime.tv_sec = tp.tv_sec;
}

#elif defined(_WIN32)

#include <sys/timeb.h>
#include <time.h>

/* since the point YAP was started */
static struct _timeb StartOfWTimes;

/* since last call to walltime */
#define LastWtime (*(struct timeb *)LastWtimePtr)

/* store user time in this variable */
static void
InitWTime (void)
{
  _ftime(&StartOfWTimes);
}

static void
InitLastWtime(void) {
  LastWtimePtr = (void *)Yap_AllocCodeSpace(sizeof(struct timeb));
  LastWtime.time = StartOfWTimes.time;
  LastWtime.millitm = StartOfWTimes.millitm;
}


Int
Yap_walltime (void)
{
  struct _timeb   tp;

  _ftime(&tp);
  if (StartOfWTimes.millitm > tp.millitm)
    return((tp.time - StartOfWTimes.time - 1) * 1000 +
	   (StartOfWTimes.millitm - tp.millitm));
  else
    return((tp.time - StartOfWTimes.time)) * 1000 +
      ((tp.millitm - LastWtime.millitm) / 1000);
}

void Yap_walltime_interval(Int *now,Int *interval)
{
  struct _timeb   tp;

  _ftime(&tp);
  *now = (tp.time - StartOfWTimes.time) * 1000 +
    (tp.millitm - StartOfWTimes.millitm);
  *interval = (tp.time - LastWtime.time) * 1000 +
    (tp.millitm - LastWtime.millitm) ;
  LastWtime.millitm = tp.millitm;
  LastWtime.time = tp.time;
}

#elif HAVE_TIMES

static clock_t StartOfWTimes;

#define LastWtime (*(clock_t *)LastWtimePtr)

/* store user time in this variable */
static void
InitWTime (void)
{
  StartOfWTimes = times(NULL);
}

static void
InitLastWtime(void) {
  LastWtimePtr = (void *)Yap_AllocCodeSpace(sizeof(clock_t));
  LastWtime = StartOfWTimes;
}

Int
Yap_walltime (void)
{
  clock_t t;
  t = times(NULL);
  return ((t - StartOfWTimes)*1000 / TicksPerSec));
}

void Yap_walltime_interval(Int *now,Int *interval)
{
  clock_t t;
  t = times(NULL);
  *now = ((t - StartOfWTimes)*1000) / TicksPerSec;
  *interval = (t - LastWtime) * 1000 / TicksPerSec;
}

#endif /* HAVE_TIMES */

#if HAVE_TIME_H
#include <time.h>
#endif

unsigned int current_seed;

static void
InitRandom (void)
{
  current_seed = (unsigned int) time (NULL);
#if HAVE_RANDOM
  srandom (current_seed);
#elif HAVE_RAND
  srand (current_seed);
#endif
}

STD_PROTO (extern int rand, (void));


double
Yap_random (void)
{
#if HAVE_RANDOM
/*  extern long random (); */
  return (((double) random ()) / 0x7fffffffL /* 2**31-1 */);
#elif HAVE_RAND
  return (((double) (rand ()) / RAND_MAX));
#else
  Yap_Error(SYSTEM_ERROR, TermNil,
	"random not available in this configuration");
  return (0.0);
#endif
}

static Int
p_srandom (void)
{
  register Term t0 = Deref (ARG1);
  if (IsVarTerm (t0)) {
    return(Yap_unify(ARG1,MkIntegerTerm((Int)current_seed)));
  }
  if(!IsNumTerm (t0))
    return (FALSE);
  if (IsIntTerm (t0))
    current_seed = (unsigned int) IntOfTerm (t0);
  else if (IsFloatTerm (t0))
    current_seed  = (unsigned int) FloatOfTerm (t0);
  else
    current_seed  = (unsigned int) LongIntOfTerm (t0);
#if HAVE_RANDOM
  srandom(current_seed);
#elif HAVE_RAND
  srand(current_seed);

#endif
  return (TRUE);
}

#if HAVE_SIGNAL

#include <signal.h>

#ifdef MPW
#define signal	sigset
#endif


#ifdef MSH

#define SIGFPE	SIGDIV

#endif

STATIC_PROTO (void InitSignals, (void));


#if (defined(__svr4__) || defined(__SVR4))

#if HAVE_SIGINFO_H
#include <siginfo.h>
#endif
#if HAVE_SYS_UCONTEXT_H
#include <sys/ucontext.h>
#endif

STATIC_PROTO (void HandleSIGSEGV, (int, siginfo_t   *, ucontext_t *));
STATIC_PROTO (void HandleMatherr,  (int, siginfo_t   *, ucontext_t *));
STATIC_PROTO (void my_signal_info, (int, void (*)(int, siginfo_t  *, ucontext_t *)));
STATIC_PROTO (void my_signal, (int, void (*)(int, siginfo_t  *, ucontext_t *)));

/* This routine believes there is a continuous space starting from the
   HeapBase and ending on TrailTop */
static void
HandleSIGSEGV(int   sig,   siginfo_t   *sip, ucontext_t *uap)
{

#if !USE_SYSTEM_MALLOC
  if (sip->si_code != SI_NOINFO &&
      sip->si_code == SEGV_MAPERR &&
      (void *)(sip->si_addr) > (void *)(Yap_HeapBase) &&
      (void *)(sip->si_addr) < (void *)(Yap_TrailTop+64 * 1024L)) {
    Yap_growtrail(64 * 1024L);
  }  else
#endif
    {
      Yap_Error(FATAL_ERROR, TermNil,
		"likely bug in YAP, segmentation violation at %p", sip->si_addr);
  }
}


static void
HandleMatherr(int  sig, siginfo_t *sip, ucontext_t *uap)
{
  yap_error_number error_no;

  /* reset the registers so that we don't have trash in abstract machine */

  switch(sip->si_code) {
  case FPE_INTDIV:
    error_no = EVALUATION_ERROR_ZERO_DIVISOR;
    break;
  case FPE_INTOVF:
    error_no = EVALUATION_ERROR_INT_OVERFLOW;
    break;
  case FPE_FLTDIV:
    error_no = EVALUATION_ERROR_ZERO_DIVISOR;
    break;
  case FPE_FLTOVF:
    error_no = EVALUATION_ERROR_FLOAT_OVERFLOW;
    break;
  case FPE_FLTUND:
    error_no = EVALUATION_ERROR_FLOAT_UNDERFLOW;
    break;
  case FPE_FLTRES:
  case FPE_FLTINV:
  case FPE_FLTSUB:
  default:
    error_no = EVALUATION_ERROR_UNDEFINED;
  }
  set_fpu_exceptions(0);
  Yap_Error(error_no, TermNil, "");
}


static void
my_signal_info(int sig, void (*handler)(int, siginfo_t  *, ucontext_t *))
{
  struct sigaction sigact;

  sigact.sa_handler = handler;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_flags = SA_SIGINFO;

  sigaction(sig,&sigact,NULL);
}

static void
my_signal(int sig, void (*handler)(int, siginfo_t *, ucontext_t *))
{
  struct sigaction sigact;

  sigact.sa_handler=handler;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_flags = 0;
  sigaction(sig,&sigact,NULL);
}

#else /* if not (defined(__svr4__) || defined(__SVR4)) */

STATIC_PROTO (RETSIGTYPE HandleMatherr, (int));
STATIC_PROTO (RETSIGTYPE HandleSIGSEGV, (int));
STATIC_PROTO (void my_signal_info, (int, void (*)(int)));
STATIC_PROTO (void my_signal, (int, void (*)(int)));

/******** Handling floating point errors *******************/


/* old code, used to work with matherror(), deprecated now:
  char err_msg[256];
  switch (x->type)
    {
    case DOMAIN:
    case SING:
      Yap_Error(EVALUATION_ERROR_UNDEFINED, TermNil, "%s", x->name);
      return(0);
    case OVERFLOW:
      Yap_Error(EVALUATION_ERROR_FLOAT_OVERFLOW, TermNil, "%s", x->name);
      return(0);
    case UNDERFLOW:
      Yap_Error(EVALUATION_ERROR_FLOAT_UNDERFLOW, TermNil, "%s", x->name);
      return(0);
    case PLOSS:
    case TLOSS:
      Yap_Error(EVALUATION_ERROR_UNDEFINED, TermNil, "%s(%g) = %g", x->name,
	       x->arg1, x->retval);
      return(0);
    default:
      Yap_Error(EVALUATION_ERROR_UNDEFINED, TermNil, NULL);
      return(0);
    }
  */


#if HAVE_FENV_H
#include <fenv.h>
#endif

static RETSIGTYPE
HandleMatherr(int sig)
{
#if HAVE_FETESTEXCEPT

  /* This should work in Linux, but it doesn't seem to. */
  
  int raised = fetestexcept(FE_ALL_EXCEPT);

  if (raised & FE_OVERFLOW) {
    Yap_matherror = EVALUATION_ERROR_FLOAT_OVERFLOW;
  } else if (raised & (FE_INVALID|FE_INEXACT)) {
    Yap_matherror = EVALUATION_ERROR_UNDEFINED;
  } else if (raised & FE_DIVBYZERO) {
    Yap_matherror = EVALUATION_ERROR_ZERO_DIVISOR;
  } else if (raised & FE_UNDERFLOW) {
    Yap_matherror = EVALUATION_ERROR_FLOAT_UNDERFLOW;
  } else
#endif
    Yap_matherror = EVALUATION_ERROR_UNDEFINED;
  /* something very bad happened on the way to the forum */
  set_fpu_exceptions(FALSE);
  Yap_Error(Yap_matherror, TermNil, "");
}

static void
SearchForTrailFault(void)
{
  /* If the TRAIL is very close to the top of mmaped allocked space,
     then we can try increasing the TR space and restarting the
     instruction. In the worst case, the system will
     crash again
     */
#ifdef DEBUG
  /*  fprintf(stderr,"Catching a sigsegv at %p with %p\n", TR, TrailTop); */
#endif
#if  OS_HANDLES_TR_OVERFLOW && !USE_SYSTEM_MALLOC
  if ((TR > (tr_fr_ptr)Yap_TrailTop-1024  && 
       TR < (tr_fr_ptr)Yap_TrailTop+(64*1024))|| Yap_DBTrailOverflow()) {
    long trsize = 64*2014L;
    while ((CELL)TR > (CELL)Yap_TrailTop+trsize) {
      trsize += 64*2014L;
    }
    if (!Yap_growtrail(trsize)) {
      Yap_Error(OUT_OF_TRAIL_ERROR, TermNil, "YAP failed to reserve %ld bytes in growtrail", 64*1024L);
    }
    /* just in case, make sure the OS keeps the signal handler. */
    /*    my_signal_info(SIGSEGV, HandleSIGSEGV); */
  } else
#endif /* OS_HANDLES_TR_OVERFLOW */
    Yap_Error(FATAL_ERROR, TermNil,
	  "likely bug in YAP, segmentation violation");
}

static RETSIGTYPE
HandleSIGSEGV(int   sig)
{
  if (Yap_PrologMode & ExtendStackMode) {
    fprintf(stderr,  "%% YAP FATAL ERROR: OS memory allocation crashed: bailing out~n");
    exit(1);
  }
  SearchForTrailFault();
}

#if HAVE_SIGACTION

static void
my_signal_info(int sig, void (*handler)(int))
{
  struct sigaction sigact;

  sigact.sa_handler = handler;
  sigemptyset(&sigact.sa_mask);
#if HAVE_SIGINFO
  sigact.sa_flags = SA_SIGINFO;
#else
  sigact.sa_flags = 0;
#endif

  sigaction(sig,&sigact,NULL);
}

static void
my_signal(int sig, void (*handler)(int))
{
  struct sigaction sigact;

  sigact.sa_handler=handler;
  sigemptyset(&sigact.sa_mask);
  sigact.sa_flags = 0;

  sigaction(sig,&sigact,NULL);
}

#else

static void
my_signal(int sig, void (*handler)(int))
{
  signal(sig, handler);
}

static void
my_signal_info(sig, handler)
int sig;
void (*handler)(int);
{
  if(signal(sig, handler) == SIG_ERR)
    exit(1);
}
#endif /* __linux__ */

#endif /* (defined(__svr4__) || defined(__SVR4)) */


static int
InteractSIGINT(int ch) {
  switch (ch) {
  case 'a':
    /* abort computation */
    if (Yap_PrologMode & ConsoleGetcMode) {
      Yap_PrologMode |= AbortMode;
    } else {
      Yap_Error(PURE_ABORT, TermNil, "");
      /* in case someone mangles the P register */
#if  _MSC_VER || defined(__MINGW32__)
      /* don't even think about trying this */
#else
#if PUSH_REGS
      restore_absmi_regs(&Yap_standard_regs);
#endif
      siglongjmp (Yap_RestartEnv, 1);
#endif
    }
    return(-1);
  case 'c':
    /* continue */
    return(1);
  case 'd':
    /* enter debug mode */
    Yap_PutValue (Yap_LookupAtom ("debug"), MkIntTerm (1));
    return(1);
  case 'e':
    /* exit */
    Yap_exit(0);
    return(-1);
  case 't':
    /* start tracing */
    Yap_PutValue (Yap_LookupAtom ("debug"), MkIntTerm (1));
    Yap_PutValue (Yap_LookupAtom ("spy_sl"), MkIntTerm (0));
    Yap_PutValue (Yap_FullLookupAtom ("$trace"), MkIntTerm (1));
    yap_flags[SPY_CREEP_FLAG] = 1;
    Yap_signal (YAP_CREEP_SIGNAL);
    return(1);
#ifdef LOW_LEVEL_TRACER
  case 'T':
    toggle_low_level_trace();
    return(1);
#endif
  case 's':
    /* show some statistics */
    Yap_show_statistics();
    return(1);
  case EOF:
    return(0);
    break;
  case 'h':
  case '?':
  default:
    /* show an helpful message */
    fprintf(Yap_stderr, "Please press one of:\n");
    fprintf(Yap_stderr, "  a for abort\n  c for continue\n  d for debug\n");
    fprintf(Yap_stderr, "  e for exit\n  s for statistics\n  t for trace\n");
    return(0);
  }
}

/*
  This function talks to the user about a signal. We assume we are in
  the context of the main Prolog thread (trivial in Unix, but hard in WIN32)
*/ 
static int
ProcessSIGINT(void)
{
  int ch, out;

  do {
    ch = Yap_GetCharForSIGINT();
  } while (!(out = InteractSIGINT(ch)));
  return(out);
}

int
Yap_ProcessSIGINT(void)
{
  return ProcessSIGINT();
}

/* This function is called from the signal handler to process signals.
   We assume we are within the context of the signal handler, whatever
   that might be
*/
static RETSIGTYPE
#if (defined(__svr4__) || defined(__SVR4))
HandleSIGINT (int sig, siginfo_t   *x, ucontext_t *y)
#else
HandleSIGINT (int sig)
#endif
{
  my_signal(SIGINT, HandleSIGINT);
  /* do this before we act */
#if HAVE_ISATTY
  if (!isatty(0)) {
    Yap_Error(INTERRUPT_ERROR,MkIntTerm(SIGINT),NULL);
  }
#endif
  if (Yap_PrologMode & (CritMode|ConsoleGetcMode)) {
    Yap_PrologMode |= InterruptMode;
#if HAVE_LIBREADLINE
    if (Yap_PrologMode & ConsoleGetcMode) {
      fprintf(stderr, "Action (h for help): ");
      rl_point = rl_end = 0;
#if HAVE_RL_SET_PROMPT
      rl_set_prompt("Action (h for help): ");
#endif
    }
#endif
    return;
  }
#ifdef HAVE_SETBUF
  /* make sure we are not waiting for the end of line */
  YP_setbuf (stdin, NULL);
#endif
  if (snoozing) {
    snoozing = FALSE;
    return;
  }
  ProcessSIGINT();
}

#if !defined(_WIN32)
/* this routine is called if the system activated the alarm */
static RETSIGTYPE
#if (defined(__svr4__) || defined(__SVR4))
HandleALRM (int s, siginfo_t   *x, ucontext_t *y)
#else
HandleALRM(int s)
#endif
{
  my_signal (SIGALRM, HandleALRM);
  /* force the system to creep */
  Yap_signal (YAP_ALARM_SIGNAL);
  /* now, say what is going on */
  Yap_PutValue(AtomAlarm, MkAtomTerm(AtomTrue));
}
#endif


/*
 * This function is called after a normal interrupt had been caught.
 * It allows 6 possibilities: abort, continue, trace, debug, help, exit.
 */

#if !defined(LIGHT) && !_MSC_VER && !defined(__MINGW32__) && !defined(LIGHT) 
static RETSIGTYPE
#if (defined(__svr4__) || defined(__SVR4))
ReceiveSignal (int s, siginfo_t   *x, ucontext_t *y)
#else
ReceiveSignal (int s)
#endif
{
  switch (s)
    {
#ifndef MPW
    case SIGFPE:
      set_fpu_exceptions(FALSE);
      Yap_Error (SYSTEM_ERROR, TermNil, "floating point exception ]");
      break;
#endif
#if !defined(LIGHT) && !defined(_WIN32)
      /* These signals are not handled by WIN32 and not the Macintosh */
    case SIGQUIT:
    case SIGKILL:
      Yap_Error(INTERRUPT_ERROR,MkIntTerm(s),NULL);
#endif
#if defined(SIGUSR1)
    case SIGUSR1:
      /* force the system to creep */
      Yap_signal (YAP_USR1_SIGNAL);
      break;
#endif /* defined(SIGUSR1) */
#if defined(SIGUSR2)
    case SIGUSR2:
      /* force the system to creep */
      Yap_signal (YAP_USR2_SIGNAL);
      break;
#endif /* defined(SIGUSR2) */
#if defined(SIGHUP)
    case SIGHUP:
      /* force the system to creep */
      Yap_signal (YAP_HUP_SIGNAL);
      break;
#endif /* defined(SIGHUP) */
    default:
      fprintf(Yap_stderr, "\n[ Unexpected signal ]\n");
      exit (EXIT_FAILURE);
    }
}
#endif

#if (_MSC_VER || defined(__MINGW32__))
static BOOL WINAPI
MSCHandleSignal(DWORD dwCtrlType) {
  switch(dwCtrlType) {
  case CTRL_C_EVENT:
  case CTRL_BREAK_EVENT:
    Yap_signal(YAP_ALARM_SIGNAL);
    Yap_PrologMode |= InterruptMode;
    return(TRUE);
  default:
    return(FALSE);
  }
}
#endif

/* SIGINT can cause problems, if caught before full initialization */
static void
InitSignals (void)
{
#if !defined(LIGHT) && !_MSC_VER && !defined(__MINGW32__) && !defined(LIGHT) 
  my_signal (SIGQUIT, ReceiveSignal);
  my_signal (SIGKILL, ReceiveSignal);
  my_signal (SIGUSR1, ReceiveSignal);
  my_signal (SIGUSR2, ReceiveSignal);
  my_signal (SIGHUP,  ReceiveSignal);
  my_signal (SIGALRM, HandleALRM);
#endif
#if _MSC_VER || defined(__MINGW32__)
  signal (SIGINT, SIG_IGN);
  SetConsoleCtrlHandler(MSCHandleSignal,TRUE);
#else
  my_signal (SIGINT, HandleSIGINT);
#endif
#ifndef MPW
  my_signal (SIGFPE, HandleMatherr);
#endif
#if HAVE_SIGSEGV && !defined(THREADS)
  my_signal_info (SIGSEGV, HandleSIGSEGV);
#endif
#ifdef ACOW
  signal(SIGCHLD, SIG_IGN);  /* avoid ghosts */ 
#endif
}

#endif /* HAVE_SIGNAL */


/* TrueFileName -> Finds the true name of a file */

#ifdef __MINGW32__
#include <ctype.h>
#endif

static int
volume_header(char *file)
{
#if _MSC_VER || defined(__MINGW32__)
  char *ch = file;
  int c;

  while ((c = ch[0]) != '\0') {
    if (isalnum(c)) ch++;
    else return(c == ':');
  }
#endif
  return(FALSE);
}

int
Yap_volume_header(char *file)
{
  return volume_header(file);
}

static int
TrueFileName (char *source, char *result, int in_lib)
{
  register int ch;
  register char *res0 = result, *work;
  char ares1[YAP_FILENAME_MAX], *res1 = ares1;
  char *var_name;

  result[0] = '\0';
#if defined(__MINGW32__) || _MSC_VER
  /* step 0: replace / by \ */
  {
    char *p = source, ch = p[0];
    while (ch != '\0') {
      if (ch == '/') p[0] = '\\';
      p++;
      ch = p[0];
    }
  }
#endif
  /* step 1: eating home information */
  if (source[0] == '~')
    {
      if (dir_separator(source[1]) || source[1] == '\0')
	{
	  char *s;
	  source++;
#if defined(_WIN32)
	  s = getenv("HOMEDRIVE");
	  if (s != NULL)
	    strncpy (result, getenv ("HOMEDRIVE"), YAP_FILENAME_MAX);
	  s = getenv("HOMEPATH");
	  if (s != NULL)
	    strncpy (result, s, YAP_FILENAME_MAX);
#else
	  s = getenv ("HOME");
	  if (s != NULL)
	    strncpy (result, s, YAP_FILENAME_MAX);
#endif
	}
#if HAVE_GETPWNAM
      else
	{
	  struct passwd *user_passwd;

	  source++;
	  while (!dir_separator((*res0 = *source)) && *res0 != '\0')
	    res0++, source++;
	  *res0++ = '\0';
	  if ((user_passwd = getpwnam (result)) == NULL)
	    {
	      return(FALSE);
	    }
	  strncpy (result, user_passwd->pw_dir, YAP_FILENAME_MAX);
	}
#endif
      strncat (result, source, YAP_FILENAME_MAX);
    }
  else
    strncpy (result, source, YAP_FILENAME_MAX);
  /* step 2: handling environment variables in file names */
  strncpy (ares1, result, YAP_FILENAME_MAX);
  res0 = result;
  while ((ch = *res1++)!=0)
    {
      if (ch == '\\' && !dir_separator('\\'))
	{
	  ch = *res1++;
	  if (ch == '\0')
	    {
	      *res0 = '\0';
	      break;
	    }
	  else
	    *res0++ = ch;
	}
      if (ch != '$')
	*res0++ = ch;
      else
	{
	  char env_var[256], *sptr = env_var;
	  while (((ch = *res1)!=0) && is_valid_env_char (ch))
	    {
	      res1++;
	      *sptr++ = ch;
	    }
	  *sptr = '\0';
	  if ((var_name = (char *) getenv (env_var)) == NULL)
	    {
	      return(FALSE);
	    }
	  else
	    while ((*res0 = *var_name++)!=0)
	      res0++;
	}
    }
  *res0 = '\0';
  /* step 3: get the full file name */
  if (!dir_separator(result[0]) && !volume_header(result))
    {

#if __simplescalar__
      /* does not implement getcwd */
      strncpy(ares1,yap_pwd,YAP_FILENAME_MAX);
#elif HAVE_GETCWD
      if (getcwd (ares1, YAP_FILENAME_MAX) == NULL)
	return (FALSE);
#else
      if (getwd (ares1) == NULL)
	return (FALSE);
#endif
#if _MSC_VER || defined(__MINGW32__)
      strncat (ares1, "\\", YAP_FILENAME_MAX);
#else
      strncat (ares1, "/", YAP_FILENAME_MAX);
#endif
      strncat (ares1, result, YAP_FILENAME_MAX);
      if (in_lib) {
	int tmpf;
	if ((tmpf = open(ares1, O_RDONLY)) < 0) {
	  /* not in current directory, let us try the library */
	  if  (Yap_LibDir != NULL) {
	    strncpy(Yap_FileNameBuf, Yap_LibDir, YAP_FILENAME_MAX);
#if HAVE_GETENV
	  } else {
	    char *yap_env = getenv("YAPLIBDIR");
	    if (yap_env != NULL) {
	      strncpy(ares1, yap_env, YAP_FILENAME_MAX);
#endif
	    } else {
	      strncpy(ares1, LIB_DIR, YAP_FILENAME_MAX);
	    }
#if HAVE_GETENV
	  }
#endif
#if _MSC_VER || defined(__MINGW32__)
	  strncat(ares1,"\\", YAP_FILENAME_MAX);
#else
	  strncat(ares1,"/", YAP_FILENAME_MAX);
#endif
	  strncat(ares1,result, YAP_FILENAME_MAX);
	  if ((tmpf = open(ares1, O_RDONLY)) >= 0) {
	    close(tmpf);
	    strncpy (result, ares1, YAP_FILENAME_MAX);
	  }
	} else {
	  strncpy (result, ares1, YAP_FILENAME_MAX);
	  close(tmpf);
	}
      } else {
	strncpy (result, ares1, YAP_FILENAME_MAX);
      }
    }
  /* step 4: simplifying the file name */
  work = result;
  while (*work != '\0')
    {
      char *new_work, *next_work;
      if (*work++ != '.')
	continue;
      if (*work != '.')
	{
	  if (!dir_separator(*work) || !dir_separator(work[-2]))
	    continue;
	  next_work = work + 1;
	  new_work = --work;
	}
      else
	{
	  if (!dir_separator(work[1]) || !dir_separator(work[-2]))
	    continue;
	  next_work = work + 2;
	  work -= 2;
	  if (work == result)
	    return (FALSE);
	  while (!dir_separator(*--work) && work != result);
	  if (work == result && !dir_separator(work[0]))
	    return (FALSE);
	  new_work = ++work;
	}
      while ((*new_work++ = *next_work++)!=0);
    }
  return (TRUE);
}

int
Yap_TrueFileName (char *source, char *result, int in_lib)
{
  return TrueFileName (source, result, in_lib);
}

static Int
p_getcwd(void)
{
  Term t;

#if __simplescalar__
  /* does not implement getcwd */
  strncpy(Yap_FileNameBuf,yap_pwd,YAP_FILENAME_MAX);
#elif HAVE_GETCWD
  if (getcwd (Yap_FileNameBuf, YAP_FILENAME_MAX) == NULL)
    return (FALSE);
#else
  if (getwd (Yap_FileNameBuf) == NULL)
    return (FALSE);
#endif

  t = Yap_StringToList(Yap_FileNameBuf);
  return(Yap_unify(ARG1,t));

}

/* Executes $SHELL under Prolog */

static Int
p_sh (void)
{				/* sh				 */
#ifdef HAVE_SYSTEM
  register char *shell;
  shell = (char *) getenv ("SHELL");
  if (shell == NULL)
    shell = "/bin/sh";
  /* Yap_CloseStreams(TRUE); */
  if (system (shell) < 0) {
#if HAVE_STRERROR
    Yap_Error(SYSTEM_ERROR, TermNil, 
	"sh: %s", strerror(errno));
#else
    Yap_Error(SYSTEM_ERROR, TermNil,
	  "sh");
#endif
    return (FALSE);
  }
  return (TRUE);
#else
#ifdef MSH
  register char *shell;
  shell = "msh -i";
  /* Yap_CloseStreams(); */
  system (shell);
  return (TRUE);
#else
  Yap_Error(SYSTEM_ERROR,TermNil,"sh not available in this configuration");
  return(FALSE);
#endif /* MSH */
#endif
}

static Int
p_shell (void)
{				/* '$shell'(+SystCommand)			 */
#if _MSC_VER || defined(__MINGW32__)
   return(0);
#else
#if HAVE_SYSTEM 
  char *shell;
  register int bourne = FALSE;
  Term t1 = Deref (ARG1);

  if (!Yap_GetName (Yap_FileNameBuf, YAP_FILENAME_MAX, t1)) {
    Yap_Error(SYSTEM_ERROR,t1,"invalid argument to shell/1");
    return(FALSE);
  }
  shell = (char *) getenv ("SHELL");
  if (!strcmp (shell, "/bin/sh"))
    bourne = TRUE;
  if (shell == NIL)
    bourne = TRUE;
  /* Yap_CloseStreams(TRUE); */
  if (bourne)
    return (system (Yap_FileNameBuf) == 0);
  else
    {
      int status = -1;
      int child = fork ();
      if (child == 0)
	{			/* let the children go */
	  execl (shell, shell, "-c", Yap_FileNameBuf, NIL);
	  exit (TRUE);
	}
      {				/* put the father on wait */
	int result = child < 0 ||
/* vsc:I am not sure this is used, Stevens say wait returns an integer.
#if NO_UNION_WAIT
*/
	wait ((&status)) != child ||
/*
#else
	wait ((union wait *) (&status)) != child ||
#endif
*/
	status == 0;
	return (result);
      }
    }
#undef command
#else /* HAVE_SYSTEM */
#ifdef MSH
  register char *shell;
  shell = "msh -i";
  /* Yap_CloseStreams(); */
  system (shell);
  return (TRUE);
#else
  Yap_Error (SYSTEM_ERROR,TermNil,"shell not available in this configuration");
  return(FALSE);
#endif
#endif /* HAVE_SYSTEM */
#endif /* _MSC_VER */
}

static Int
p_system (void)
{				/* '$system'(+SystCommand)	       */
#ifdef HAVE_SYSTEM
  Term t1 = Deref (ARG1);
  if (!Yap_GetName (Yap_FileNameBuf, YAP_FILENAME_MAX, t1)) {
    Yap_Error(SYSTEM_ERROR,t1,"argument to system/1 is not valid");
    return(FALSE);
  }
  /* Yap_CloseStreams(TRUE); */
#if _MSC_VER
  _flushall();
#endif
  return (system (Yap_FileNameBuf) == 0);
#else
#ifdef MSH
  register char *shell;
  shell = "msh -i";
  /* Yap_CloseStreams(); */
  system (shell);
  return (TRUE);
#undef command
#else
  Yap_Error(SYSTEM_ERROR,TermNil,"sh not available in this machine");
  return(FALSE);
#endif
#endif /* HAVE_SYSTEM */
}



/* Rename a file */
static Int
p_mv (void)
{				/* rename(+OldName,+NewName)   */
#if HAVE_LINK
  int r;
  char oldname[YAP_FILENAME_MAX], newname[YAP_FILENAME_MAX];
  Term t1 = Deref (ARG1);
  Term t2 = Deref (ARG2);
  if (!Yap_GetName (Yap_FileNameBuf, YAP_FILENAME_MAX, t1)) {
    Yap_Error(SYSTEM_ERROR,t1,"first argument to rename/2 is not valid");
    return(FALSE);
  }
  TrueFileName (Yap_FileNameBuf, oldname, FALSE);
  if (!Yap_GetName (Yap_FileNameBuf, YAP_FILENAME_MAX, t2)) {
    Yap_Error(SYSTEM_ERROR,t2,"second argument to rename/2 is not valid");
    return(FALSE);
  }
  TrueFileName (Yap_FileNameBuf, newname, FALSE);
  if ((r = link (oldname, newname)) == 0 && (r = unlink (oldname)) != 0)
    unlink (newname);
  if (r != 0) {
    Yap_Error(SYSTEM_ERROR,t2,"operating system error in rename/2");
    return(FALSE);
  }
  return (TRUE);
#else
  Yap_Error(SYSTEM_ERROR,TermNil,"rename/2 not available in this machine");
  return (FALSE);
#endif
}


/* Change the working directory */
static Int
p_cd (void)
{				/* cd(+NewD)			 */
#if HAVE_CHDIR
  Term t1 = Deref (ARG1);
  if (t1 == TermNil)
    return(TRUE);
  if (!Yap_GetName (Yap_FileNameBuf, YAP_FILENAME_MAX, t1)) {
    Yap_Error(SYSTEM_ERROR,t1,"argument to cd/1 is not valid");
    return(FALSE);
  }
  TrueFileName (Yap_FileNameBuf, Yap_FileNameBuf2, FALSE);
#if  __simplescalar__
  strncpy(yap_pwd,Yap_FileNameBuf2,YAP_FILENAME_MAX);
#endif
  if (chdir (Yap_FileNameBuf2) < 0) {
#if HAVE_STRERROR
    Yap_Error(SYSTEM_ERROR, t1, 
	"cd(%s): %s", Yap_FileNameBuf2, strerror(errno));
#else
    Yap_Error(SYSTEM_ERROR,t1,"cd(%s)", Yap_FileNameBuf2);
#endif
    return(FALSE);
  }
  return(TRUE);
#else
#ifdef MACYAP
  Term t1 = Deref (ARG1);
  if (!Yap_GetName (Yap_FileNameBuf, YAP_FILENAME_MAX, t1)) {
    Yap_Error(SYSTEM_ERROR,t1,"argument to cd/1 is not valid");
    return(FALSE);
  }
  TrueFileName (Yap_FileNameBuf, Yap_FileNameBuf2, FALSE);
  return (!chdir (Yap_FileNameBuf2));
#else
  Yap_Error(SYSTEM_ERROR,TermNil,"cd/1 not available in this machine");
  return(FALSE);
#endif
#endif
}

#ifdef MAC

void
Yap_SetTextFile (name)
     char *name;
{
#ifdef MACC
  SetFileType (name, 'TEXT');
  SetFileSignature (name, 'EDIT');
#else
  FInfo f;
  FInfo *p = &f;
  GetFInfo (name, 0, p);
  p->fdType = 'TEXT';
#ifdef MPW
  if (mpwshell)
    p->fdCreator = 'MPS\0';
#endif
#ifndef LIGHT
  else
    p->fdCreator = 'EDIT';
#endif
  SetFInfo (name, 0, p);
#endif
}

#endif


/* return YAP's environment */
static Int p_getenv(void)
{
#if HAVE_GETENV
  Term t1 = Deref(ARG1), to;
  char *s, *so;

  if (IsVarTerm(t1)) {
    Yap_Error(INSTANTIATION_ERROR, t1,
	  "first arg of getenv/2");
    return(FALSE);
  } else if (!IsAtomTerm(t1)) {
    Yap_Error(TYPE_ERROR_ATOM, t1,
	  "first arg of getenv/2");
    return(FALSE);
  } else s = RepAtom(AtomOfTerm(t1))->StrOfAE;
  if ((so = getenv(s)) == NULL)
    return(FALSE);
  to = MkAtomTerm(Yap_LookupAtom(so));
  return(Yap_unify_constant(ARG2,to));
#else
    Yap_Error(SYSTEM_ERROR, TermNil,
	  "getenv not available in this configuration");
    return (FALSE);
#endif
}

/* set a variable in YAP's environment */
static Int p_putenv(void)
{
#if HAVE_PUTENV
  Term t1 = Deref(ARG1), t2 = Deref(ARG2);
  char *s, *s2, *p0, *p;

  if (IsVarTerm(t1)) {
    Yap_Error(INSTANTIATION_ERROR, t1,
	  "first arg to putenv/2");
    return(FALSE);
  } else if (!IsAtomTerm(t1)) {
    Yap_Error(TYPE_ERROR_ATOM, t1,
	  "first arg to putenv/2");
    return(FALSE);
  } else s = RepAtom(AtomOfTerm(t1))->StrOfAE;
  if (IsVarTerm(t2)) {
    Yap_Error(INSTANTIATION_ERROR, t1,
	  "second arg to putenv/2");
    return(FALSE);
  } else if (!IsAtomTerm(t2)) {
    Yap_Error(TYPE_ERROR_ATOM, t2,
	  "second arg to putenv/2");
    return(FALSE);
  } else s2 = RepAtom(AtomOfTerm(t2))->StrOfAE;
  p0 = p = Yap_AllocAtomSpace(strlen(s)+strlen(s2)+3);
  while ((*p++ = *s++) != '\0');
  p[-1] = '=';
  while ((*p++ = *s2++) != '\0');
  if (putenv(p0) == 0)
    return(TRUE);
#if HAVE_STRERROR
  Yap_Error(SYSTEM_ERROR, TermNil,
	"putenv: %s", strerror(errno));
#else
  Yap_Error(SYSTEM_ERROR, TermNil,
	"putenv");
#endif
  return (FALSE);
#else
    Yap_Error(SYSTEM_ERROR, TermNil,
	  "putenv not available in this configuration");
    return (FALSE);
#endif
}

/* set a variable in YAP's environment */
static Int p_file_age(void)
{
  char *file_name = RepAtom(AtomOfTerm(Deref(ARG1)))->StrOfAE;
  if (strcmp(file_name,"user_input") == 0) {
    return(Yap_unify(ARG2,MkIntTerm(-1)));
  }
#if HAVE_LSTAT 
 {
   struct stat buf;

   if (lstat(file_name, &buf) == -1) {
     /* file does not exist, but was opened? Return -1 */
     return(Yap_unify(ARG2, MkIntTerm(-1)));
   }
   return(Yap_unify(ARG2, MkIntegerTerm(buf.st_mtime)));
 }
#elif defined(__MINGW32__) || _MSC_VER
  {
    struct _stat buf;

    if (_stat(file_name, &buf) != 0) {
      /* return an error number */
      return(Yap_unify(ARG2, MkIntTerm(-1)));
    }
    return(Yap_unify(ARG2, MkIntegerTerm(buf.st_mtime)));
  }
#else
  return(Yap_unify(ARG2, MkIntTerm(-1)));
#endif
}

/* wrapper for alarm system call */
#if _MSC_VER || defined(__MINGW32__)

static DWORD WINAPI
DoTimerThread(LPVOID targ)
{
  Int time = *(Int *)targ;
  HANDLE htimer;
  LARGE_INTEGER liDueTime;

  htimer = CreateWaitableTimer(NULL,FALSE,NULL);
  liDueTime.QuadPart =  -10000000;
  liDueTime.QuadPart *=  time;
  /* Copy the relative time into a LARGE_INTEGER. */
  if (SetWaitableTimer(htimer, &liDueTime,0,NULL,NULL,0) == 0) {
    return(FALSE);
  }
  if (WaitForSingleObject(htimer, INFINITE) != WAIT_OBJECT_0)
    fprintf(stderr,"WaitForSingleObject failed (%ld)\n", GetLastError());
  Yap_signal (YAP_ALARM_SIGNAL);
  /* now, say what is going on */
  Yap_PutValue(AtomAlarm, MkAtomTerm(AtomTrue));
  ExitThread(1);
#if _MSC_VER
  return(0L);
#endif
}

#endif

static Int
p_alarm(void)
{
  Term t = Deref(ARG1);
  if (IsVarTerm(t)) {
    Yap_Error(INSTANTIATION_ERROR, t, "alarm/2");
    return(FALSE);
  }
  if (!IsIntegerTerm(t)) {
    Yap_Error(TYPE_ERROR_INTEGER, t, "alarm/2");
    return(FALSE);
  }
#if _MSC_VER || defined(__MINGW32__)
  {
    Term tout;
    Int time = IntegerOfTerm(t);
    
    if (time != 0) {
      DWORD dwThreadId; 
      HANDLE hThread; 

      hThread = CreateThread( 
			     NULL,     /* no security attributes */
			     0,        /* use default stack size */ 
			     DoTimerThread, /* thread function */
			     (LPVOID)&time,  /* argument to thread function */
			     0,        /* use default creation flags  */
			     &dwThreadId);  /* returns the thread identifier */
 
      /* Check the return value for success. */
      if (hThread == NULL) {
	Yap_WinError("trying to use alarm");
      }
    }
    tout = MkIntegerTerm(0);
    return(Yap_unify(ARG2,tout));
  }
#elif HAVE_ALARM
  {
    Int left;
    Term tout;

    left = alarm(IntegerOfTerm(t));
    tout = MkIntegerTerm(left);
    return(Yap_unify(ARG2,tout));
  }
#else
  /* not actually trying to set the alarm */
  if (IntegerOfTerm(t) == 0)
    return(TRUE);
  Yap_Error(SYSTEM_ERROR, TermNil,
	"alarm not available in this configuration");
  return(FALSE);
#endif
}

#if HAVE_FPU_CONTROL_H
#include <fpu_control.h>
#endif

/* by default Linux with glibc is IEEE compliant anyway..., but we will pretend it is not. */
static void
set_fpu_exceptions(int flag)
{
  if (flag) {
#if defined(__hpux)
# if HAVE_FESETTRAPENABLE
/* From HP-UX 11.0 onwards: */
    fesettrapenable(FE_INVALID|FE_DIVBYZERO|FE_OVERFLOW|FE_UNDERFLOW);
# else
/*
  Up until HP-UX 10.20:
  FP_X_INV   invalid operation exceptions
  FP_X_DZ    divide-by-zero exception
  FP_X_OFL   overflow exception
  FP_X_UFL   underflow exception
  FP_X_IMP   imprecise (inexact result)
  FP_X_CLEAR simply zero to clear all flags
*/
    fpsetmask(FP_X_INV|FP_X_DZ|FP_X_OFL|FP_X_UFL);
# endif
#endif /* __hpux */
#if HAVE_FPU_CONTROL_H && i386 && defined(__GNUC__)
    /* I shall ignore denormalization and precision errors */
    int v = _FPU_IEEE & ~(_FPU_MASK_IM|_FPU_MASK_ZM|_FPU_MASK_OM|_FPU_MASK_UM);
    _FPU_SETCW(v);
#endif
#if HAVE_FETESTEXCEPT
    feclearexcept(FE_ALL_EXCEPT);
#endif
    my_signal (SIGFPE, HandleMatherr);
  } else {
    /* do IEEE arithmetic in the way the big boys do */
#if defined(__hpux)
# if HAVE_FESETTRAPENABLE
    fesettrapenable(FE_ALL_EXCEPT);
# else
    fpsetmask(FP_X_CLEAR);
# endif
#endif /* __hpux */
#if HAVE_FPU_CONTROL_H && i386 && defined(__GNUC__)
    /* this will probably not work in older releases of Linux */
    int v = _FPU_IEEE;
   _FPU_SETCW(v);
#endif    
    my_signal (SIGFPE, SIG_IGN);
  }
}

void
Yap_set_fpu_exceptions(int flag)
{
  set_fpu_exceptions(flag);
}
static Int
p_set_fpu_exceptions(void) {
  if (yap_flags[LANGUAGE_MODE_FLAG] == 1) {
    set_fpu_exceptions(FALSE); /* can't make it work right */
  } else {
    set_fpu_exceptions(FALSE);
  }
  return(TRUE);
}

static Int
p_host_type(void) {
  Term out = MkAtomTerm(Yap_LookupAtom(HOST_ALIAS));
  return(Yap_unify(out,ARG1));
}

/*
 * This is responsable for the initialization of all machine dependant
 * predicates
 */
void
Yap_InitSysbits (void)
{
#if  __simplescalar__
  {
    char *pwd = getenv("PWD");
    strncpy(yap_pwd,pwd,YAP_FILENAME_MAX);
  }
#endif
  InitPageSize();
  InitWTime ();
  InitRandom ();
  /* let the caller control signals as it sees fit */
  InitSignals ();
}

void
Yap_InitTime(void)
{
  InitTime();
}

void
Yap_ReInitWallTime (void)
{
  InitWTime();
  if (heap_regs->last_wtime != NULL) 
    Yap_FreeCodeSpace(heap_regs->last_wtime);
  InitLastWtime();
}

static Int
p_first_signal(void)
{
  LOCK(SignalLock);
  /* always do wakeups first, because you don't want to keep the
     non-backtrackable variable bad */
  if (ActiveSignals & YAP_WAKEUP_SIGNAL) {
    ActiveSignals &= ~YAP_WAKEUP_SIGNAL;
    UNLOCK(SignalLock);
    return Yap_unify(ARG1, MkAtomTerm(Yap_LookupAtom("sig_wake_up")));
  }
  if (ActiveSignals & YAP_ITI_SIGNAL) {
    ActiveSignals &= ~YAP_ITI_SIGNAL;
    UNLOCK(SignalLock);
    return Yap_unify(ARG1, MkAtomTerm(Yap_LookupAtom("sig_iti")));
  }
  if (ActiveSignals & YAP_INT_SIGNAL) {
    ActiveSignals &= ~YAP_INT_SIGNAL;
    UNLOCK(SignalLock);
    return Yap_unify(ARG1, MkAtomTerm(Yap_LookupAtom("sig_int")));
  }
  if (ActiveSignals & YAP_USR2_SIGNAL) {
    ActiveSignals &= ~YAP_USR2_SIGNAL;
    UNLOCK(SignalLock);
    return Yap_unify(ARG1, MkAtomTerm(Yap_LookupAtom("sig_usr2")));
  }
  if (ActiveSignals & YAP_USR1_SIGNAL) {
    ActiveSignals &= ~YAP_USR1_SIGNAL;
    UNLOCK(SignalLock);
    return Yap_unify(ARG1, MkAtomTerm(Yap_LookupAtom("sig_usr1")));
  }
  if (ActiveSignals & YAP_HUP_SIGNAL) {
    ActiveSignals &= ~YAP_HUP_SIGNAL;
    UNLOCK(SignalLock);
    return Yap_unify(ARG1, MkAtomTerm(Yap_LookupAtom("sig_hup")));
  }
  if (ActiveSignals & YAP_ALARM_SIGNAL) {
    ActiveSignals &= ~YAP_ALARM_SIGNAL;
    UNLOCK(SignalLock);
    return Yap_unify(ARG1, MkAtomTerm(Yap_LookupAtom("sig_alarm")));
  }
  if (ActiveSignals & YAP_CREEP_SIGNAL) {
    ActiveSignals &= ~YAP_CREEP_SIGNAL;
    UNLOCK(SignalLock);
    return Yap_unify(ARG1, MkAtomTerm(Yap_LookupAtom("sig_creep")));
  }
  UNLOCK(SignalLock);
  return FALSE;
}

static Int
p_continue_signals(void)
{
  /* hack to force the signal anew */
  if (ActiveSignals & YAP_ITI_SIGNAL) {
    Yap_signal(YAP_ITI_SIGNAL);
  }
  if (ActiveSignals & YAP_INT_SIGNAL) {
    Yap_signal(YAP_INT_SIGNAL);
  }
  if (ActiveSignals & YAP_USR2_SIGNAL) {
    Yap_signal(YAP_USR2_SIGNAL);
  }
  if (ActiveSignals & YAP_USR1_SIGNAL) {
    Yap_signal(YAP_USR1_SIGNAL);
  }
  if (ActiveSignals & YAP_HUP_SIGNAL) {
    Yap_signal(YAP_HUP_SIGNAL);
  }
  if (ActiveSignals & YAP_ALARM_SIGNAL) {
    Yap_signal(YAP_ALARM_SIGNAL);
  }
  if (ActiveSignals & YAP_CREEP_SIGNAL) {
    Yap_signal(YAP_CREEP_SIGNAL);
  }
  return TRUE;
}

void
Yap_InitSysPreds(void)
{
  /* can only do after heap is initialised */
  InitLastWtime();
  Yap_InitCPred ("srandom", 1, p_srandom, SafePredFlag);
  Yap_InitCPred ("sh", 0, p_sh, SafePredFlag|SyncPredFlag);
  Yap_InitCPred ("$shell", 1, p_shell, SafePredFlag|SyncPredFlag);
  Yap_InitCPred ("$system", 1, p_system, SafePredFlag|SyncPredFlag);
  Yap_InitCPred ("$rename", 2, p_mv, SafePredFlag|SyncPredFlag);
  Yap_InitCPred ("$cd", 1, p_cd, SafePredFlag|SyncPredFlag);
  Yap_InitCPred ("$getcwd", 1, p_getcwd, SafePredFlag|SyncPredFlag);
  Yap_InitCPred ("$dir_separator", 1, p_dir_sp, SafePredFlag);
  Yap_InitCPred ("$alarm", 2, p_alarm, SafePredFlag|SyncPredFlag);
  Yap_InitCPred ("$getenv", 2, p_getenv, SafePredFlag);
  Yap_InitCPred ("$putenv", 2, p_putenv, SafePredFlag|SyncPredFlag);
  Yap_InitCPred ("$file_age", 2, p_file_age, SafePredFlag|SyncPredFlag);
  Yap_InitCPred ("$set_fpu_exceptions", 0, p_set_fpu_exceptions, SafePredFlag|SyncPredFlag);
  Yap_InitCPred ("$host_type", 1, p_host_type, SafePredFlag|SyncPredFlag);
  Yap_InitCPred ("$first_signal", 1, p_first_signal, SafePredFlag|SyncPredFlag);
  Yap_InitCPred ("$continue_signals", 0, p_continue_signals, SafePredFlag|SyncPredFlag);
}


#ifdef VAX

/* avoid longjmp botch */

int vax_absmi_fp;

typedef struct
  {
    int eh;
    int flgs;
    int ap;
    int fp;
    int pc;
    int dummy1;
    int dummy2;
    int dummy3;
    int oldfp;
    int dummy4;
    int dummy5;
    int dummy6;
    int oldpc;
  }

 *VaxFramePtr;


VaxFixFrame (dummy)
{
  int maxframes = 100;
  VaxFramePtr fp = (VaxFramePtr) (((int *) &dummy) - 6);
  while (--maxframes)
    {
      fp = (VaxFramePtr) fp->fp;
      if (fp->flgs == 0)
	{
	  if (fp->oldfp >= &REGS[6] && fp->oldfp < &REGS[REG_SIZE])
	    fp->oldfp = vax_absmi_fp;
	  return;
	}
    }
}

#endif


#if defined(_WIN32)

#include <windows.h>

int WINAPI STD_PROTO(win_yap, (HANDLE, DWORD, LPVOID));

int WINAPI win_yap(HANDLE hinst, DWORD reason, LPVOID reserved)
{
  switch (reason) 
    {
    case DLL_PROCESS_ATTACH:
      break;
    case DLL_PROCESS_DETACH:
      break;
    case DLL_THREAD_ATTACH:
      break;
    case DLL_THREAD_DETACH:
      break;
    }
  return 1;
}
#endif

#if (defined(YAPOR) || defined(THREADS)) && !defined(USE_PTHREAD_LOCKING)
#ifdef sparc
void STD_PROTO(rw_lock_voodoo,(void));

void
rw_lock_voodoo(void) {
  /* code taken from the Linux kernel, it handles shifting between locks */
  /* Read/writer locks, as usual this is overly clever to make it as fast as possible. */
	/* caches... */
	__asm__ __volatile__(
"___rw_read_enter_spin_on_wlock:\n"
"	orcc	%g2, 0x0, %g0\n"
"	be,a	___rw_read_enter\n"
"	 ldstub	[%g1 + 3], %g2\n"
"	b	___rw_read_enter_spin_on_wlock\n"
"	 ldub	[%g1 + 3], %g2\n"
"___rw_read_exit_spin_on_wlock:\n"
"	orcc	%g2, 0x0, %g0\n"
"	be,a	___rw_read_exit\n"
"	 ldstub	[%g1 + 3], %g2\n"
"	b	___rw_read_exit_spin_on_wlock\n"
"	 ldub	[%g1 + 3], %g2\n"
"___rw_write_enter_spin_on_wlock:\n"
"	orcc	%g2, 0x0, %g0\n"
"	be,a	___rw_write_enter\n"
"	 ldstub	[%g1 + 3], %g2\n"
"	b	___rw_write_enter_spin_on_wlock\n"
"	 ld	[%g1], %g2\n"
"\n"
"	.globl	___rw_read_enter\n"
"___rw_read_enter:\n"
"	orcc	%g2, 0x0, %g0\n"
"	bne,a	___rw_read_enter_spin_on_wlock\n"
"	 ldub	[%g1 + 3], %g2\n"
"	ld	[%g1], %g2\n"
"	add	%g2, 1, %g2\n"
"	st	%g2, [%g1]\n"
"	retl\n"
"	 mov	%g4, %o7\n"
"	.globl	___rw_read_exit\n"
"___rw_read_exit:\n"
"	orcc	%g2, 0x0, %g0\n"
"	bne,a	___rw_read_exit_spin_on_wlock\n"
"	 ldub	[%g1 + 3], %g2\n"
"	ld	[%g1], %g2\n"
"	sub	%g2, 0x1ff, %g2\n"
"	st	%g2, [%g1]\n"
"	retl\n"
"	 mov	%g4, %o7\n"
"	.globl	___rw_write_enter\n"
"___rw_write_enter:\n"
"	orcc	%g2, 0x0, %g0\n"
"	bne	___rw_write_enter_spin_on_wlock\n"
"	 ld	[%g1], %g2\n"
"	andncc	%g2, 0xff, %g0\n"
"	bne,a	___rw_write_enter_spin_on_wlock\n"
"	 stb	%g0, [%g1 + 3]\n"
"	retl\n"
"	 mov	%g4, %o7\n"
   );
}
#endif /* sparc */


#ifdef i386
asm(

".align	4\n"
".globl	__write_lock_failed\n"
"__write_lock_failed:\n"
"	lock;   addl	$" RW_LOCK_BIAS_STR ",(%eax)\n"
"1:	cmpl	$" RW_LOCK_BIAS_STR ",(%eax)\n"
"	jne	1b\n"
"	lock;   subl	$" RW_LOCK_BIAS_STR ",(%eax)\n"
"	jnz	__write_lock_failed\n"
"	ret\n"
".align	4\n"
".globl	__read_lock_failed\n"
"__read_lock_failed:\n"
"	lock ; incl	(%eax)\n"
"1:	cmpl	$1,(%eax)\n"
"	js	1b\n"
"	lock ; decl	(%eax)\n"
"	js	__read_lock_failed\n"
"	ret\n"

);
#endif /* i386 */
#endif /* YAPOR || THREADS */

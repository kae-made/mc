/*----------------------------------------------------------------------------
 * File:  sys_user_co.c
 *
 * Description:
 * Interface call-outs allow the user to capture execution control of the
 * generated system running on a target.
 * Especially in the deeply embedded software/hardware development world,
 * it may be necessary to tightly interface the xtUML system to the
 * surrounding/containing system. MC-3020 provides callout routines
 * that enable the user to easily interface code generated by the model
 * compiler with other system code. These callout routines are empty when
 * generated by the model compiler. It is up to the user to define
 * additional functionality (if necessary) to be performed at these
 * callout points.
 *--------------------------------------------------------------------------*/

#include "maslin_sys_types.h"
#include "sys_user_co.h"
#include "sys_xtumlload.h"

#ifdef SYS_USER_CO_PRINTF_ON
#include <stdio.h>
#define SYS_USER_CO_PRINTF( s ) printf( s );
#else
#define SYS_USER_CO_PRINTF( s )
#endif

// we write our own implementation of strsep so it works on windows
#include <string.h>
char *_strsep(char **stringp, const char *delim) {
    if ( !stringp || !*stringp ) return NULL;
    char* start = *stringp;
    char* p = strpbrk( start, delim );
    if ( !p ) {
        *stringp = NULL;
    }
    else {
        *p = '\0';
        *stringp = p+1;
    }
    return start;
}

/*
 * UserInitializationCallout
 *
 * This function is invoked at the immediate beginning of application
 * initialization. It is the very first function to be executed at system
 * startup.
 * User supplied implementation of this function should be restricted to
 * things like memory initialization, early hardware duties, etc.
 */
void
UserInitializationCalloutf( void )
{
/* Activate this invocation to initialize the example simple TIM.  */
  #if ESCHER_SYS_MAX_XTUML_TIMERS > 0
  TIM_init();
  #endif
  /* Insert implementation specific code here.  */
  SYS_USER_CO_PRINTF( "UserInitializationCallout\n" )
}

/*
 * UserPreOoaInitializationCallout
 *
 * This function is invoked immediately prior to executing any xtUML
 * initialization functions.
 */
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include "masl_url.h"
void
UserPreOoaInitializationCalloutf( int argc, char ** argv )
{
  char * globals[3] = { 0, 0, 0 };
  {
    int c;
    opterr = 0;
    while ( ( c = getopt ( argc, argv, "a:i:o:g:" ) ) != -1 ) {
      switch ( c ) {
        case 'a':
        case 'i':
        case 'o':
          break;
        case 'g':
          if ( !optarg ) abort();
          else {
            globals[2] = optarg;
            Escher_xtUML_load( 3, globals );
          }
          break;
        case '?':
          fprintf( stderr, "Unknown option character '%c'.\n", optopt );
          break;
        default:
          abort (); // die ignominiously
      }
    }
  }
}

/*
 * UserPostOoaInitializationCallout
 *
 * This function is invoked immediately after executing any xtUML
 * initialization functions.
 * When this callout function returns, the system dispatcher will allow the
 * xtUML application analysis state models to start consuming events.
 */
void Escher_dump_instances( const Escher_DomainNumber_t, const Escher_ClassNumber_t );
void
UserPostOoaInitializationCalloutf( int argc, char ** argv )
{
  masl2xtuml_model * model = masl2xtuml_model_op_create( "maslin" );
  char s[ ESCHER_SYS_MAX_STRING_LEN ], e[ ESCHER_SYS_MAX_STRING_LEN ], v[ 8 ][ 64000 ], arg[ ESCHER_SYS_MAX_STRING_LEN ];
  char * p, * q, * element = e, * value[ 8 ] = { v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7] };
  int i, j;
  {
    int c;
    opterr = 0;
    optind = 1;
    while ( ( c = getopt ( argc, argv, "a:i:o:g:" ) ) != -1 ) {
      switch ( c ) {
        case 'a':
          if ( !optarg ) abort();
          else masl2xtuml_model_op_setoption( model, "actiondialect", optarg );
          break;
        case 'i':
          if ( !optarg ) abort();
          else masl2xtuml_model_op_setoption( model, "projectroot", optarg );
          break;
        case 'o':
          if ( !optarg ) abort();
          else masl2xtuml_model_op_setoption( model, "projectroot", optarg );
          break;
        case 'g':
          break;
        case '?':
          fprintf( stderr, "Unknown option character '%c'.\n", optopt );
          break;
        default:
          abort (); // die ignominiously
      }
    }
  }
  while ( ( p = fgets( s, ESCHER_SYS_MAX_STRING_LEN, stdin ) ) != NULL ) {
    i = 0;
    p[ strlen(p) - 1 ] = 0;
    if ( ( q = _strsep( &p, "," ) ) != NULL ) { strcpy( element, q ); }

    while ( p != NULL ) {
      q = _strsep(&p, ",");
      masl_url_decode( arg, q );
      strcpy( value[ i++ ], arg );
    }
    masl2xtuml_in_populate( element, value );
  }

  // call the serial interface end signal when we are done reading input from stdin
  masl2xtuml_in_end();

  // For the masl conversion, we have a system, but we don't want that to be the root type during import.
  // Instead, we want the user to create a project, then import this file, so the root types are packages.
  // Output the header info to specify this, then dump the in-memory instances.
  printf("-- root-types-contained: Package_c\n");
  printf("-- BP 7.1 content: StreamData syschar: 3 persistence-version: 7.1.6\n\n");
  for ( i = 0; i < masl2xtuml_MAX_CLASS_NUMBERS; i++ ) {
    // Skip dumping the system instance
    if ( i != masl2xtuml_S_SYS_CLASS_NUMBER ) {
      Escher_dump_instances( 0, i );
    }
  }
}

/*
 * UserBackgroundProcessingCallout
 *
 * This function is invoked once during each loop execution of the system
 * dispather.
 * It is invoked at the 'top' of the system dispatcher loop, immediately
 * prior to dispatching any xtUML application analysis events.
 */
void
UserBackgroundProcessingCalloutf( void )
{
  /* Activate this invocation to periodically tick the example simple TIM.  */
  #if ESCHER_SYS_MAX_XTUML_TIMERS > 0
  TIM_tick();
  #endif
  /* Insert implementation specific code here.  */
}

/*
 * UserPreShutdownCallout
 *
 * This function is invoked at termination of the system dispatcher, but
 * prior to performing any xtUML application analysis shutdown sequencing.
 */
void
UserPreShutdownCalloutf( void )
{
  /* Insert implementation specific code here.  */
  SYS_USER_CO_PRINTF( "UserPreShutdownCallout\n" )
}

/*
 * UserPostShutdownCallout
 *
 * This function is invoked immediately before application exit.
 */
void
UserPostShutdownCalloutf( void )
{
  /* Insert implementation specific code here.  */
  SYS_USER_CO_PRINTF( "UserPostShutdownCallout\n" )
}

/*
 * UserEventCantHappenCallout
 *
 * This function is invoked any time that an event is received that
 * results in a "cant happen" transition.
 */
void
UserEventCantHappenCalloutf(
  const Escher_StateNumber_t current_state,
  const Escher_StateNumber_t next_state,
  const Escher_EventNumber_t event_number )
{
  /* Insert implementation specific code here.  */
  SYS_USER_CO_PRINTF( "UserEventCantHappenCallout\n" )
}

/*
 * UserEventNoInstanceCallout
 *
 * This function is invoked when we failed to validate the instance
 * to which an event was directed.  This can happen in normal operation
 * when the instance was deleted while the event was in flight (analysis
 * error).
 */
void
UserEventNoInstanceCalloutf(
  const Escher_EventNumber_t event_number )
{
  /* Insert implementation specific code here.  */
  SYS_USER_CO_PRINTF( "UserEventNoInstanceCallout\n" )
}

/*
 * UserEventFreeListEmptyCallout
 *
 * This function is invoked when an attempt is made to allocate an
 * event, but there are no more left.
 */
void
UserEventFreeListEmptyCalloutf( void )
{
  /* Insert implementation specific code here.  */
  SYS_USER_CO_PRINTF( "UserEventFreeListEmptyCallout\n" )
}

/*
 * UserEmptyHandleDetectedCallout
 *
 * This function is invoked when an attempt is made to use an instance
 * reference variable (handle) that is null (empty).
 */
void
UserEmptyHandleDetectedCalloutf( c_t * object_keyletters, c_t * s )
{
  fprintf( stderr, "UserEmptyHandleDetectedCallout %s %s.\n", object_keyletters, s );
}

/*
 * UserObjectPoolEmptyCallout
 *
 * This function is invoked when an attempt is made to create an
 * instance of an object, but there are no instances available.
 */
void
UserObjectPoolEmptyCalloutf( const Escher_DomainNumber_t component_number, const Escher_ClassNumber_t class_number )
{
  /* Insert implementation specific code here.  */
  SYS_USER_CO_PRINTF( "UserObjectPoolEmptyCallout\n" )
}

/*
 * UserNodeListEmptyCallout
 *
 * This function is invoked when an attempt is made to allocate a
 * node, but there are no more left.
 */
void
UserNodeListEmptyCalloutf( void )
{
  /* Insert implementation specific code here.  */
  SYS_USER_CO_PRINTF( "UserNodeListEmptyCallout\n" )
}

/*
 * UserInterleavedBridgeOverflowCallout
 *
 * This function is invoked when an attempt is made to post too many
 * interleaved bridges.  The depth of this list is defined by
 * SYS_MAX_INTERLEAVED_BRIDGES (unless changed in the archetype).
 */
void
UserInterleavedBridgeOverflowCalloutf( void )
{
  /* Insert implementation specific code here.  */
  SYS_USER_CO_PRINTF( "UserInterleavedBridgeOverflowCallout\n" )
}

/*
 * UserSelfEventQueueEmptyCallout
 *
 * This function is invoked when the events to self queue is
 * interrogated and found to be empty.
 */
void
UserSelfEventQueueEmptyCalloutf( void )
{
  /* Insert implementation specific code here.  */
}

/*
 * UserNonSelfEventQueueEmptyCallout
 *
 * This function is invoked when the events to instance queue is
 * interrogated and found to be empty.
 */
void
UserNonSelfEventQueueEmptyCalloutf( void )
{
  /* Insert implementation specific code here.  */
}

/*
 * UserPersistenceErrorCallout
 *
 * This function is invoked when the events to instance queue is
 * interrogated and found to be empty.
 */
void
UserPersistenceErrorCalloutf( i_t error_code )
{
  /* Insert implementation specific code here.  */
  SYS_USER_CO_PRINTF( "UserPersistenceErrorCallout\n" )
}

/**
 * FujiNet for #Adam configuration program
 *
 * Type Definitions
 */

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

typedef enum _sf_subState
  {
   SF_SCAN,
   SF_SELECT,
   SF_CUSTOM,
   SF_PASSWORD,
   SF_DONE
  } SFSubState;

typedef enum _state
  {
   CHECK_WIFI,
   CONNECT_WIFI,
   SET_WIFI,
   HOSTS_AND_DEVICES,
   SELECT_FILE,
   SELECT_SLOT,
   PERFORM_COPY,
   SHOW_INFO,
   DONE
  }
State;

#endif /* TYPEDEFS_H */

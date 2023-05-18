/**
 * FujiNet configuration program
 *
 * Type Definitions
 */

#ifndef TYPEDEFS_H
#define TYPEDEFS_H

typedef enum _entry_types
{
  ENTRY_TYPE_TEXT,
  ENTRY_TYPE_FOLDER,
  ENTRY_TYPE_FILE,
  ENTRY_TYPE_LINK,
  ENTRY_TYPE_MENU
} EntryType;

typedef enum _ws_subState
  {
   WS_SCAN,
   WS_SELECT,
   WS_CUSTOM,
   WS_PASSWORD,
   WS_DONE
  } WSSubState;

typedef enum _hd_subState 
{
   HD_HOSTS,
   HD_DEVICES,
   HD_CLEAR_ALL_DEVICES,
   HD_DONE
} HDSubState;

typedef enum _sf_subState
  {
   SF_INIT,
   SF_DISPLAY,
   SF_NEXT_PAGE,
   SF_PREV_PAGE,
   SF_CHOOSE,
   SF_LINK,
   SF_FILTER,
   SF_ADVANCE_FOLDER,
   SF_DEVANCE_FOLDER,
   SF_NEW,
   SF_COPY,
   SF_DONE
  } SFSubState;

typedef enum _ss_subState
  {
   SS_INIT,
   SS_DISPLAY,
   SS_CHOOSE,
   SS_DONE,
   SS_ABORT
  } SSSubState;

typedef enum _dh_subState
  {
   DH_INIT,
   DH_DISPLAY,
   DH_CHOOSE,
   DH_DONE,
   DH_ABORT
  } DHSubState;

typedef enum _si_subState
  {
   SI_SHOWINFO,
   SI_DONE
  } SISubState;

typedef enum _state
  {
   CHECK_WIFI,
   CONNECT_WIFI,
   SET_WIFI,
   HOSTS_AND_DEVICES,
   SELECT_FILE,
   SELECT_SLOT,
   DESTINATION_HOST_SLOT,
   PERFORM_COPY,
   SHOW_INFO,
  #ifdef BUILD_APPLE2
   SHOW_DEVICES,
  #endif
   DONE
  }
State;

#endif /* TYPEDEFS_H */

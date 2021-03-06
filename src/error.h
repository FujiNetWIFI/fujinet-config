/**
 * #FUJINET CONFIG
 *
 * Error Display Routines
 */

#ifndef ERROR_H
#define ERROR_H

/* Note: When adding/removing/reordering things in
   this enum list, be sure to keep `error_texts`
   char * array in "errors.c" synchronized! */
typedef enum _errorMessage
  {
   ERROR_READING_ADAPTER_CONFIG,
   ERROR_SCANNING_NETWORKS,
   ERROR_READING_SCAN_RESULT,
   ERROR_READING_NET_CONFIG,
   ERROR_SETTING_SSID,
   ERROR_READING_WIFI_STATUS,
   ERROR_WIFI_NO_SSID_AVAILABLE,
   ERROR_WIFI_CONNECTION_SUCCESSFUL,
   ERROR_WIFI_CONNECT_FAILED,
   ERROR_WIFI_CONNECTION_LOST,
   ERROR_WIFI_DISCONNECTED,
   ERROR_READING_HOST_SLOTS,
   ERROR_READING_DEVICE_SLOTS,
   ERROR_MOUNTING_HOST_SLOT,
   ERROR_MOUNTING_DEVICE_SLOT,
   ERROR_SETTING_DIRECTORY_POSITION,
   ERROR_OPENING_DIRECTORY,
   ERROR_READING_DIRECTORY,
   ERROR_SETTING_FULL_PATH,
   ERROR_WRITING_DEVICE_SLOTS,
   ERROR_CREATING_NEW_DISK,
   ERROR_INTENTIONAL_STOP,
   ERROR_EMPTY_SD
  } ErrorMessage;

/**
 * Display a fatal error, then die.
 * 
 * msg = An error message from the enum
 */
void error_fatal(ErrorMessage msg);

/**
 * Display error.
 * 
 * msg = An error message from the enum
 */
void error(ErrorMessage msg);


#endif /* ERROR_H */

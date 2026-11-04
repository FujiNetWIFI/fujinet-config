; strings.inc -- every word CONFIG says. Lines in the message window hold
; 30 characters, 26 before an error's hex code.

        INCLUDE "lobby.inc"             ; generated: str_lobby_host

str_fujinet:            DB      "FUJINET",0
str_no_cart:            DB      "NO FUJINET CART",0
str_for_coleco:         DB      "FOR THE COLECOVISION",0
str_press_fire:         DB      "PRESS FIRE",0

; legends: the left button, the right button, * the menu
str_leg_hosts:          DB      "L OPEN  R WIFI  * MENU",0
str_leg_copy:           DB      "L PICK  R CANCEL COPY",0
str_leg_files:          DB      "L OPEN  R UP  <> PAGE  * MENU",0
str_leg_noseek:         DB      "R UP  * MENU",0
str_leg_wifi:           DB      "L PICK  R HOSTS  * RESCAN",0
str_leg_giveup:         DB      "R GIVES UP",0
str_leg_back:           DB      "L OR R GOES BACK",0
str_leg_edit:           DB      "L TYPE  R RUB  * END",0

; titles and menu items
str_hosts:              DB      "HOSTS",0
str_t_copy:             DB      "COPY",0
str_networks:           DB      "NETWORKS",0
str_joining:            DB      "JOINING",0
str_adapter_info:       DB      "ADAPTER INFO",0
str_filter:             DB      "FILTER",0
str_network:            DB      "NETWORK",0
str_password:           DB      "PASSWORD",0
str_host_name:          DB      "HOST NAME",0
str_m_lobby:            DB      "GAME LOBBY",0
str_m_rename:           DB      "RENAME SLOT",0
str_m_wifi:             DB      "WIFI SETUP",0
str_m_copy:             DB      "COPY FILE",0
str_m_copy_here:        DB      "COPY HERE",0
str_m_cancel_copy:      DB      "CANCEL COPY",0
str_m_info              EQU     str_adapter_info
str_m_filter            EQU     str_filter
str_m_hosts             EQU     str_hosts

; prompts and results
str_select_host:        DB      "SELECT A HOST.",0
str_copy_to:            DB      "COPY TO WHICH HOST?",0
str_empty:              DB      "(EMPTY)",0
str_no_matches:         DB      "(NO MATCHES)",0
str_empty_rename:       DB      "EMPTY. * MENU, RENAME SLOT.",0
str_slot_empty:         DB      "THAT SLOT IS EMPTY.",0
str_mounting_host:      DB      "MOUNTING HOST...",0
str_input_host:         DB      "INPUT THE HOST NAME.",0
str_saving:             DB      "SAVING...",0
str_finding_lobby:      DB      "FINDING LOBBY HOST...",0
str_mounting_lobby:     DB      "MOUNTING LOBBY HOST...",0
str_lobby_path:         DB      "/coleco/lobby.rom",0
str_select_file:        DB      "SELECT A FILE.",0
str_select_filt:        DB      "SELECT A FILE. (FILTERED)",0
str_pick_folder:        DB      "COPY: PICK A FOLDER.",0
str_reading:            DB      "READING...",0
str_path_long:          DB      "THE PATH IS TOO LONG.",0
str_menu_copy:          DB      "* MENU, THEN COPY HERE.",0
str_filter_hint:        DB      "EMPTY SHOWS EVERYTHING.",0
str_no_folder:          DB      "A FOLDER CANNOT BE COPIED.",0
str_copying:            DB      "COPYING...",0
str_copied:             DB      "COPIED.",0
str_please_wait:        DB      "PLEASE WAIT...",0
str_please_wait2:       DB      "PLEASE WAIT.",0
str_connected:          DB      "CONNECTED!",0
str_no_ssid:            DB      "NO SSID AVAILABLE.",0
str_conn_failed:        DB      "CONNECT FAILED.",0
str_conn_lost:          DB      "CONNECTION LOST.",0
str_unable:             DB      "UNABLE TO CONNECT.",0
str_other:              DB      "OTHER (TYPE AN SSID)",0
str_select_net:         DB      "SELECT A NETWORK.",0
str_scanning:           DB      "SCANNING...",0
str_input_net:          DB      "INPUT THE NETWORK NAME.",0
str_input_pass:         DB      "INPUT THE PASSWORD.",0
str_setting_ssid:       DB      "SETTING SSID...",0
str_not_rom:            DB      "NOT A CARTRIDGE IMAGE.",0

; the progress window's phase words, padded so each covers the last
str_mounting:           DB      "MOUNTING",0
str_loading:            DB      "LOADING ",0
str_ready:              DB      "READY   ",0

; adapter info
str_i_ssid:             DB      "SSID",0
str_i_host:             DB      "HOST",0
str_i_ip:               DB      "IP",0
str_i_gateway:          DB      "GATEWAY",0
str_i_netmask:          DB      "NETMASK",0
str_i_dns:              DB      "DNS",0
str_i_mac:              DB      "MAC",0
str_i_bssid:            DB      "BSSID",0
str_i_version:          DB      "VERSION",0

; the keyboard's actions
str_a_case:             DB      "CASE",0
str_a_spc:              DB      "SPC",0
str_a_rub:              DB      "RUB",0
str_a_end:              DB      "END",0
str_a_esc:              DB      "ESC",0

; errors, shown with the code beside them
str_ehosts:             DB      "EHOSTS",0
str_ehost:              DB      "EHOST",0
str_eopen:              DB      "EOPEN",0
str_ewrite:             DB      "EWRITE",0
str_epath:              DB      "EPATH",0
str_eseek:              DB      "ESEEK",0
str_eread:              DB      "EREAD",0
str_eempty:             DB      "EEMPTY",0
str_ecopy:              DB      "ECOPY",0
str_escan:              DB      "ESCAN",0
str_essid:              DB      "ESSID",0
str_einfo:              DB      "EINFO",0
str_emount:             DB      "EMOUNT",0
str_etimeout:           DB      "ETIMEOUT",0
str_eload:              DB      "ELOAD",0
str_eload_big:          DB      "ELOAD: TOO BIG",0
str_eload_short:        DB      "ELOAD: TRUNCATED",0
str_eload_map:          DB      "ELOAD: NO MAPPER",0
str_eload_busy:         DB      "ELOAD: CART BUSY",0

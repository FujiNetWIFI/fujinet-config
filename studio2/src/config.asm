; config.asm -- FujiNet CONFIG for the RCA Studio II.
;
; Standalone, like ../channelf/: it mirrors src/'s screens without sharing its
; code. Channel F parity: WiFi status, scan, a network by name and connect
; through an on-screen keyboard; the eight host slots (list, open, rename);
; directory browsing with paging and subdirectories; copying a file to
; another host; adapter info; and boot. Left out, as on every console port:
; the device-slot screen, mount/eject, read/write toggles, new disk, appkeys.
; Device slot 0 is the only one, and CONFIG_BOOT is never sent: CONFIG lives
; in the cart's flash.
;
; The states are routines that never return: each starts with an empty stack
; (STATE) and leaves by FJMP. The runtime (s2call.inc, fujilib.inc,
; fujidisp.inc, input.inc) fills $0400-$07FB; CONFIG starts at $0C00 and
; RFIT hops each region's end to the next $x400.

        CPU     1802
        INCLUDE "fujinet.inc"
        INCLUDE "s2macro.inc"
        INCLUDE "cfgdefs.inc"

        ORG     0400H
        INCLUDE "s2call.inc"
        INCLUDE "fujilib.inc"
        INCLUDE "fujidisp.inc"
        INCLUDE "input.inc"
        IF      $ > FN_CLAIM
        ERROR   "the runtime runs into the claim"
        ENDIF

        ORG     0C00H

        RFIT    59
main:   LDI     HI(V_APP)
        PHI     RB                      ; for good
        SETV    V_KLAST, 0
        SETV    V_HOST, 0
        SETV    V_MODE, 0
        SETV    V_SEEN, 0
        FCALL   def_glyphs
        FCALL   fn_chk
        BDF     mn1
        FCALL   pr_cls
        DB      "NO FUJINET CART", 0
mn0:    BR      mn0
mn1:    FJMP    st_wifi

        INCLUDE "ui.inc"
        INCLUDE "fujicmd.inc"
        INCLUDE "wifi.inc"
        INCLUDE "edit.inc"
        INCLUDE "hosts.inc"
        INCLUDE "browse.inc"
        INCLUDE "copy.inc"
        INCLUDE "info.inc"
        INCLUDE "boot.inc"
CFG_END:

        S2CLAIM

        END

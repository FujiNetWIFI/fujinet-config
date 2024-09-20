; This code is taken (and adapted) from Apple IIgs Technical Notes #71: DA Tips and Techniques

                case    on              required for C compatibility
		        mcopy   13/orcainclude/m16.orca assembler macros (short, long etc.)
                mcopy   13:ainclude:M16.Memory

CDAentry        start
HowMuchStack    gequ    $0800           try for 2K of stack space

start           phd
                phb
                phk
                plb
                pha                     Space for result
                pha
                ph4     #HowMuchStack
                pha
                _MMStartup
                pla
; we chose $e aux ID here because $f is already used inside FujiNet lib
                ora     #$0e00          OR in an arbitrary auxiliary ID
                pha
                ph2     #$C001          fixed, locked, use specified bank
                ph4     #0              (specify bank 0)
                _NewHandle
                tsc
                sta     theOldStack
                bcs     NoStackSpace    still set from _NewHandle
                tcd
                lda     [1]
                tcd
;                clc                     carry is already clear
                adc     #HowMuchStack-1
NoStackSpace    pha
                ldx     #$fe
keepStack       lda     >$000100,x
                sta     stackImage,x
                dex
                dex
                bpl     keepStack
                pla
                tcs
; sp_is_init and sp_fuji_id are initialized at CDA loading.
; When run again, these 2 static variables keep their previous
; value and so MUST be reset to 0                
                short   m
                stz     sp_is_init      
                stz     sp_fuji_id
                long    m
                jsl     main            carry is clear if large stack available
                php
                php
                pla
                sta     pRegister
                sei
                ldx     #$fe
restoreStack    lda     stackImage,x
                sta     >$000100,x
                dex
                dex
                bpl     restoreStack
                lda     theOldStack
                tcs
                lda     pRegister
                pha
                plp
                plp
                lda     1,s
                ora     3,s
                beq     noDispose
                _DisposeHandle
                bra     Exit
noDispose       pla
                pla
Exit            plb
                pld
                rtl
pRegister       ds 2
theOldStack     ds 2
stackImage      ds 256

                end
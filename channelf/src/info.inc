; ---------------------------------------------------------------------------
; info.inc -- ST_INFO: what the adapter says about itself.
;
; One transaction. GET_ADAPTERCONFIG_EXTENDED puts everything shown here in a
; single reply: ssid at +0, fn_version at +125, sLocalIP at +140, all as text,
; so the rows are drawn straight out of the reply window with no copying.
; ---------------------------------------------------------------------------

SINFO:	ENTER
	LI CVAL3
	LR 3,A
	DCI SITITLE
	XDC
	PI UITITLE
	PI UICLRL
	LI CVAL1
	LR 3,A
	DCI SIFOOT
	XDC
	PI UIFOOT

	PI CADPTX
	CI 0
	BZ SIOK
	LR 1,A
	LIS 8
	LR 0,A
	PI SETERR
	JMP FAIL

SIOK:	LI CVAL1
	LR 3,A
	CLR
	LR 0,A
	DCI SILSSID
	XDC
	PI UIROW
	LI CVAL3
	LR 3,A
	LIS 1
	LR 0,A
	DCI FNREPLY
	XDC
	PI UIROW

	LI CVAL1
	LR 3,A
	LIS 2
	LR 0,A
	DCI SILIP
	XDC
	PI UIROW
	LI CVAL3
	LR 3,A
	LIS 3
	LR 0,A
	DCI FNREPLY+140
	XDC
	PI UIROW

	LI CVAL1
	LR 3,A
	LIS 4
	LR 0,A
	DCI SILVER
	XDC
	PI UIROW
	LI CVAL3
	LR 3,A
	LIS 5
	LR 0,A
	DCI FNREPLY+125
	XDC
	PI UIROW

SIIN:	PI INSCAN
	LR A,0
	CI EV_NONE
	BZ SIIN
	DCI VPEND
	LM
	LR 0,A
	JMP GOTO

SITITLE: DB "FUJINET INFO",0
SIFOOT:	DB "ANY KEY: BACK",0
SILSSID: DB "NETWORK",0
SILIP:	DB "ADDRESS",0
SILVER:	DB "FIRMWARE",0

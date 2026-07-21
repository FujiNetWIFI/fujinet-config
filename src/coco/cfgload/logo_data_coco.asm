; Standalone machine-language binary containing just the ZX0-compressed
; splash logo data (516 bytes from 4048 bytes raw), assembled directly to
; a loadable LOGO.BIN file -- kept separate from cfgload.bin so the logo
; doesn't count against that program's size budget. Generated with:
; salvador -classic bitmap.bin bitmap.zx0
;
; CoCo loads this via LOADM"LOGO" in autoexec.bas, *before*
; LOADM"CFGLOAD":EXEC, rather than fetching it over DriveWire like
; Dragon's LOGO.DWL does: on real CoCo 2 hardware, the DriveWire mount
; request for LOGO.DWL never reached the FujiNet server at all (confirmed
; via the server's own debug log), even though the DW ROM vectors
; ($D941/$D93F, see dwload_cmoc.c) were independently verified correct --
; something about that path doesn't work on real hardware that isn't yet
; understood. A prior attempt read this from within cfgload.bin itself
; via disk.c's DSKCON-based file access, which worked but only after a
; long chain of fixes for disk.c's own call chain nesting deep enough on
; real hardware to land return addresses on top of the still-visible
; splash screen buffer. Loading it via BASIC's own already-proven LOADM
; (the same mechanism CFGLOAD.BIN/CONFIG.BIN use) sidesteps all of that.
;
; org matters here (unlike the old raw-format build): LOADM reads the
; load address from this file's own DECB-format header (produced by
; `lwasm --dragon`), and must match LOGO_SCRATCH in cfgload.c.

	org $4400

logo_data
	fcb $25,$7f,$ff,$4f,$c0,$15,$48,$95,$f8,$07,$22,$e0,$01,$54,$88,$c0
	fcb $00,$00,$a4,$7f,$5a,$fc,$a2,$80,$94,$ef,$2a,$00,$95,$3f,$0a,$fe
	fcb $01,$29,$1f,$13,$ff,$85,$fe,$3d,$d1,$40,$c9,$80,$44,$dd,$c0,$b8
	fcb $7f,$c0,$06,$ca,$c1,$a6,$be,$1c,$fe,$3b,$0f,$c1,$e4,$8f,$0b,$1f
	fcb $e0,$c0,$a1,$c0,$a4,$3f,$0c,$3e,$40,$c0,$a4,$1f,$18,$82,$f0,$03
	fcb $90,$0f,$62,$fc,$0f,$0a,$07,$13,$b8,$ea,$80,$62,$c0,$1f,$22,$fc
	fcb $00,$28,$03,$3e,$77,$ef,$bc,$83,$c0,$21,$aa,$00,$0f,$c7,$e1,$f8
	fcb $7f,$85,$01,$36,$01,$08,$07,$cf,$f8,$f0,$e5,$c0,$6a,$fc,$0b,$01
	fcb $bf,$fc,$60,$f9,$4f,$7b,$c0,$58,$86,$3f,$fe,$84,$3f,$22,$e7,$f8
	fcb $62,$ff,$40,$50,$fe,$4f,$49,$c0,$89,$08,$0f,$12,$aa,$80,$07,$84
	fcb $0c,$62,$c0,$00,$4d,$8f,$ff,$0e,$07,$80,$46,$20,$0f,$03,$3f,$22
	fcb $84,$c0,$a3,$81,$4f,$aa,$84,$c0,$fe,$9e,$c1,$85,$c0,$73,$40,$f8
	fcb $c0,$84,$e0,$7f,$37,$c0,$78,$c1,$84,$f0,$3f,$2a,$fe,$62,$03,$9f
	fcb $fc,$88,$1f,$97,$f8,$1f,$7e,$80,$c0,$5a,$cf,$a4,$f0,$c2,$40,$52
	fcb $20,$e3,$e3,$98,$0f,$fc,$07,$43,$1e,$c0,$23,$07,$fe,$a1,$c1,$fe
	fcb $4b,$f0,$1f,$fb,$18,$e2,$41,$ff,$03,$11,$b4,$fc,$af,$0a,$fc,$41
	fcb $a9,$c0,$81,$06,$a9,$f8,$3f,$a4,$c0,$1c,$f8,$80,$c1,$91,$e0,$41
	fcb $a1,$c3,$2a,$e0,$98,$0f,$91,$f0,$01,$a0,$83,$0a,$f8,$41,$a1,$c0
	fcb $a1,$fc,$68,$e0,$a4,$03,$29,$07,$a4,$fe,$2a,$7f,$80,$07,$28,$ff
	fcb $59,$a2,$f0,$3f,$fe,$fb,$f8,$0b,$80,$13,$be,$3d,$c0,$2e,$0f,$fc
	fcb $bd,$f8,$80,$30,$93,$80,$75,$dd,$a3,$db,$e0,$c0,$c0,$a2,$fc,$90
	fcb $1f,$a0,$0f,$a1,$e1,$3b,$ec,$7c,$95,$fb,$97,$00,$a1,$c1,$f9,$32
	fcb $d3,$eb,$01,$78,$ff,$91,$c0,$fc,$fe,$3c,$17,$79,$1b,$da,$60,$00
	fcb $c5,$e8,$40,$fd,$17,$0d,$80,$b4,$f9,$65,$80,$30,$df,$00,$c0,$17
	fcb $07,$46,$f0,$c0,$28,$ff,$29,$ef,$14,$5c,$3d,$28,$0f,$d6,$c0,$c0
	fcb $a4,$f0,$dd,$c0,$41,$b9,$ff,$c0,$01,$38,$fe,$04,$45,$b5,$e7,$26
	fcb $45,$3f,$e0,$7f,$20,$d5,$81,$fc,$fc,$fe,$ee,$28,$1f,$e3,$0b,$9f
	fcb $9f,$00,$3f,$15,$e1,$f9,$c0,$58,$ee,$cf,$cf,$9f,$3c,$71,$0e,$fc
	fcb $3c,$f9,$f9,$e2,$a2,$e7,$a9,$87,$3f,$b9,$fc,$c0,$16,$22,$3f,$3f
	fcb $bc,$cc,$62,$be,$99,$01,$c0,$a1,$ff,$12,$20,$80,$1f,$aa,$f0,$81
	fcb $49,$9a,$9e,$1f,$3f,$c0,$10,$33,$80,$68,$fc,$0a,$9f,$5b,$87,$10
	fcb $af,$30,$1f,$c0,$56,$37,$e0,$07,$9e,$dd,$c0,$22,$3f,$ff,$78,$c0
	fcb $47,$00,$00,$80

	end logo_data

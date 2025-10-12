****************************************************
* CONFIG LOADER
* BASED ON CODE FROM BY SIMON JONASSEN (AKA INVISIBLE MAN)
*
* PREREQUISITES:
*
* DECB FAT ETC INTACT AT $600 > $DFF
* CB/ECB/DECB NOT SWITCHED OUT
* DP=0
****************************************************
	org	$7c00
lhook	equ	$cfc5
fname	equ	$094c		(8.3 notation) PADDED without "." ($20 pad)
ftype	equ	$0957		$0200 for .bin file
execsd  equ	$009D

start
	ldx	#myfile
	ldu	#fname
dofname	ldd	,x++
	std	,u++
	cmpx	#endmy
	blo	dofname
	ldd	#$200
	std	ftype		;bin file type right after name
	jsr	lhook
	ldx	execsd
	jmp	,x
	rts

myfile	fcc	'CONFIG  BIN'
endmy	equ	*
	end	start

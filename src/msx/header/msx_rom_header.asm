	EXTERN	start
	EXTERN  callstmt

; ROM header
	defm	"AB"
	defw	start
	defw	callstmt                ;CallSTMT handler
	defw	0                       ;Device handler
	defw	0                       ;basic
	defs	6

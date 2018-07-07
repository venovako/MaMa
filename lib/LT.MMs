# MaMa BSO LT
# %0:= %1 < %2

	SYSTEM  3

	SET %3 %1
	SET %4 %2

:loop	DEC %3 :maybe
	DEC %4 :false
	GOTO   :loop

:maybe	DEC %4 :false
	SET %0 1
	GOTO ::

:false	SET %0 0

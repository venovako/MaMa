# MaMa BSO LE
# %0:= %1 <= %2

	SYSTEM  3

	SET %3 %1
	SET %4 %2

:loop	DEC %3 :true
	DEC %4 :false
	GOTO :loop

:true	SET %0 1
	GOTO ::

:false	SET %0 0

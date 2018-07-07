# MaMa BSO EQ
# %0:= %1 = %2

	SYSTEM  3

	SET %3 %1
	SET %4 %2

:loop	DEC %3 :maybe
	DEC %4 :false
	GOTO   :loop

:maybe	DEC %4 :true

:false	SET %0 0
	GOTO ::

:true	SET %0 1

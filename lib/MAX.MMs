# MaMa BSO MAX
# %0:= max{%1,%2}

	SYSTEM  3

	SET %3 %1
	SET %4 %2

:loop	DEC %3 :is2
	DEC %4 :is1
	GOTO   :loop

:is1	SET %0 %1
	GOTO ::

:is2	SET %0 %2

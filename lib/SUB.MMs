# MaMa BSO SUB
# %O:= max{ %1 - %2, 0 }

	SYSTEM  3

	SET %3 %1
	SET %4 %2

:loop	DEC %4 :end
	DEC %3 :end
	GOTO   :loop

:end	SET %0 %3

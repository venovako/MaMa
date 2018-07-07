# MaMa primjer POW2
# %0:= 2 ^ %1

	SYSTEM  3

	SET %2  1
	SET %3 %1

:loop	DEC %3 :end
	CALL MUL %2 %2 2
	GOTO   :loop

:end	SET %0 %2

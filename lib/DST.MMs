# MaMa BSO DST
# %0:= | %1 - %2 |

	SYSTEM  3

	SET %3 %1
	SET %4 %2

:loop	DEC %3 :is4
	DEC %4 :is3
	GOTO   :loop

:is3	INC %3
	SET %0 %3
	GOTO ::

:is4	SET %0 %4

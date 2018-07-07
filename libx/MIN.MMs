# MaMa BSO MIN
# %0:= min{%1,%2}

	SYSTEM  3

	SET %3 %1
	SET %4 %2

:loop	DEC %3 :end
	DEC %4 :end
	INC %%
	GOTO   :loop

:end	SET %0 %%

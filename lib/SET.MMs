# MaMa primjer SET
# %0:= %1

	SYSTEM 3

:loop1	DEC %1 :loop2
	INC %2
	INC %%
	GOTO   :loop1

:loop2	DEC %2 :loop3
	INC %1
	GOTO   :loop2

	DEK %0 :!

:loop3	DEC %% ::
	INC %0
	GOTO   :loop3

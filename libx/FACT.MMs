# MaMa primjer FACT
# %0:= %1 !

	SYSTEM  3

	SET %2 %1
	DEC %2 :stop

# rekurzivan poziv ovdje je za ilustraciju
# kao i citav ovaj primjer -- faktorijele
# doista *necete* racunati ovako

	CALL %% %2

	CALL MUL %0 %% %1
	GOTO ::

:stop	SET %0 1

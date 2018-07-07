# MaMa BSO POW
# %0:= %1 ^ %2

	SYSTEM  3

	SET %3 %1
	SET %4 %2

# VAZNE NAPOMENE O SLOZENOSTI:
#
# Nastojte uvijek kao argument %1 navesti veci,
# a kao %2 manji od dvaju faktora, tako da je
# efekt gornjih SETova identican donjem kodu:
#
#	CALL MAX %3 %1 %2
#	CALL MIN %4 %1 %2

	SET %% 1

:loop	DEC %4 :end
	CALL MUL %% %% %3
	GOTO   :loop

:end	SET %0 %%

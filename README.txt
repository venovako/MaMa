                            MaMa :: MAcro MAchine


MaMa je programski paket namijenjen razvoju makro programa za makro stroj,
što je detaljnije opisano u MaMa specifikaciji [ doc\MaMaSpec.(dvi|ps|pdf) ].

MaMa i svi njeni dijelovi objavljeni su pod licencom [ LICENSE.txt ].

Svaki poddirektorij distribucije sadrži dokument [ descript.ion ], koji
ukratko kazuje što se u njemu nalazi (kao i specifikacija).  U trenutnu su
binarnu distribuciju uključeni sljedeći programi, biblioteke i dokumenti:

==> [ bin\MaMa*.exe ]   MaMa interpreter, verzije 000 i 001
==> [ bin\MaMaC*.exe ]  MaMa bytecode kompajler, verzije 000 i 001
==> [ doc\MaMaSpec.* ]  MaMa specifikacija (razni formati)
==> [ lib\*.MM?* ]      Biblioteka Standardnih makrOa (izvorni i binarni kôd)

U bin\ poddirektoriju naći ćete mnogo izvršnih fajlova, imenovanih tako
da odaju platformu (OS + procesor + kompajler) za koju su kompajlirani.

WIN32 i x86 označavaju IA32 arhitekturu, tj. klasične Pentiume, Athlone
i druge rođake; WIN64 i x64 stoje za AMD64 i Intel EM64T procesore.

MSVC sufiks kazuje da su programi kompajlirani MS Visual C++ kompajlerom.

Pregledajte [ MaMa.bat ] i [ MaMaC.bat ] datoteke; ako vam ne odgovara
odabir izvršnih verzija programa one su pravo mjesto da ga promijenite.


*** KAKO POČETI ***

0. Pročitajte MaMa specifikaciju, barem dijelove koji se tiču MaMa jezika,
   kompajlera i interpretera!  MaMa je VRLO SPECIFIČAN softver za koristiti!

1. Vratite se na korak 0. ako ste od onih koji ne čitaju manuale ili mislite
   da to ovdje nije potrebno - inače ćete spoznati što je Teži Način učenja.

2. Unixaši su silom prilika postali dovoljno vješti korisnici da će si bez
   problema prevesti niže napisane upute za korisnike Windows paketa...

3. Pokrenite MaMaCmd.bat (ili adekvatno podesite environment varijable).

4. Pokrenite trivijalan program...

   MaMa -v MIN 5 3

5. ...pogledajte njegov izvorni kôd...

   cd lib
   more MIN.MMs

6. ...i rekompajlirajte ga.

   MaMaC -v -g MIN.MMs

7. Čitajte, eksperimentirajte, pišite programe...


Za kraj:  Budite milostivi!  Uzmite u obzir da je ovo prvi release i da
su bugovi nužni, a performanse (naročito interpretera) lošije od onog što
bi se maksimalno moglo izvući.  Ovo je ipak free studentski projekt :-)


Zagreb, 2007-11-10

Vedran Novaković <venovako@gmail.com>

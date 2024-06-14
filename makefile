COPTS=-O3 -fomit-frame-pointer -std=gnu9x

centipede:	libsprite.a centipede.o
	qgcc qdos-gcc -o centipede centipede.o -lsprite

centipede.o:	libsprite.a centipede.c makefile
		qgcc qdos-gcc $(COPTS) -o centipede.o -c centipede.c

libsprite.a:
	make -C QL-sprites
	cp QL-sprites/libsprite.a .
	cp QL-sprites/*.h .

clean:
	rm -f centipede.o libsprite.a
	make -C QL-sprites clean

git:	clean
	git add .
	git commit
	git push

deploy:  centipede
	cp centipede /home/simon/emulators/ql/emulators/sQLux/flp1/
	cp centipede.lib /home/simon/emulators/ql/emulators/sQLux/flp1/
	cp centipede.scr /home/simon/emulators/ql/emulators/sQLux/flp1/centipede_scr
	cp QL-sprites/env.bin /home/simon/emulators/ql/emulators/sQLux/flp1/
	cp BOOT_flp1 /home/simon/emulators/ql/emulators/sQLux/flp1/BOOT

run:    deploy
	cd /home/simon/emulators/ql/emulators/sQLux && ./sqlux --SPEED=0.75 --RAMSIZE=896 --SOUND 8 -b "LRUN flp1_BOOT"

runfast:    deploy
	cd /home/simon/emulators/ql/emulators/sQLux && ./sqlux --SPEED=0 --RAMSIZE=896 --SOUND 8 -b "LRUN flp1_BOOT"

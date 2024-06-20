COPTS=-O3 -fomit-frame-pointer -std=gnu9x -Wall

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

cleaner:	clean
	rm -f centipede

git:	cleaner
	git add .
	git commit
	git push

deploy:  centipede
	cp centipede /home/simon/emulators/ql/emulators/sQLux/flp1/
	cp centipede.lib /home/simon/emulators/ql/emulators/sQLux/flp1/
	cp atari.lib /home/simon/emulators/ql/emulators/sQLux/flp1/atari_lib
	cp centipede.scr /home/simon/emulators/ql/emulators/sQLux/flp1/centipede_scr
	cp QL-sprites/env.bin /home/simon/emulators/ql/emulators/sQLux/flp1/
	cp BOOT_flp1 /home/simon/emulators/ql/emulators/sQLux/flp1/BOOT

run:    deploy
	cd /home/simon/emulators/ql/emulators/sQLux && ./sqlux --SPEED=0.75 --RAMSIZE=896 --SOUND 8 -b "LRUN flp1_BOOT"

runfast:    deploy
	cd /home/simon/emulators/ql/emulators/sQLux && ./sqlux --SPEED=0 --RAMSIZE=896 --SOUND 8 -b "LRUN flp1_BOOT"

dist:	deploy
	cp centipede /home/simon/emulators/ql/emulators/sQLux/flp1
	cp centipede.lib /home/simon/emulators/ql/emulators/sQLux/flp1/centipede_lib
	cp atari.lib /home/simon/emulators/ql/emulators/sQLux/flp1/atari_lib
	cp centipede.scr /home/simon/emulators/ql/emulators/sQLux/flp1/centipede_scr
	cp LICENSE /home/simon/emulators/ql/emulators/sQLux/flp1/LICENSE
	cp BOOT_zip /home/simon/emulators/ql/emulators/sQLux/flp1/BOOT
	cp QL-sprites/env.bin /home/simon/emulators/ql/emulators/sQLux/flp1/env_bin
	cd /home/simon/emulators/ql/emulators/sQLux && ./sqlux --SPEED=0 -b "LRESPR flp1_sigext_rext:EW flp1_zip,#1;'flp1_centipede_zip BOOT env_bin centipede centipede_lib atari_lib centipede_scr LICENSE'"
	mv /home/simon/emulators/ql/emulators/sQLux/flp1/centipede_zip centipede.zip
	qltools centipede.img -fdd centipede -W
	cp BOOT_flp1 BOOT
	cp centipede.scr centipede_scr
	cp centipede.lib centipede_lib
	cp atari.lib atari_lib
	cp env.bin env_bin
	qltools centipede.img -W BOOT env_bin centipede_lib atari_lib centipede_scr LICENSE
	rm -f env_bin atari_lib centipede_lib centipede_scr BOOT
	#cp BOOT_mdv1 BOOT
	#cp centipede.zip c.zip
	#mdvtool create name centipede zip_import c.zip import BOOT write centipede.mdv 
	#rm -f BOOT c.zip centipede_scr env_bin



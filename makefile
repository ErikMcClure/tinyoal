.PHONY: all clean distclean

all:
	make -f tinyoal.mk
	make -f 00testbed.mk
	make -f 01wavfile.mk
	make -f 02oggfile.mk
	make -f 03properties.mk
	make -f 04mp3file.mk
	make -f 05flacfile.mk
	
clean:
	make clean -f tinyoal.mk
	make clean -f 00testbed.mk
	make clean -f 01wavfile.mk
	make clean -f 02oggfile.mk
	make clean -f 03properties.mk
	make clean -f 04mp3file.mk
	make clean -f 05flacfile.mk
	
dist: all distclean
	-@ rm ../tinyoal-posix.tar.gz
	tar -czf ../tinyoal-posix.tar.gz "../$$(basename "$$(pwd)")"

distclean:
	make distclean -f tinyoal.mk
	make distclean -f 00testbed.mk
	make distclean -f 01wavfile.mk
	make distclean -f 02oggfile.mk
	make distclean -f 03properties.mk
	make distclean -f 04mp3file.mk
	make distclean -f 05flacfile.mk

debug:
	make debug -f tinyoal.mk
	make debug -f 00testbed.mk
	make debug -f 01wavfile.mk
	make debug -f 02oggfile.mk
	make debug -f 03properties.mk
	make debug -f 04mp3file.mk
	make debug -f 05flacfile.mk

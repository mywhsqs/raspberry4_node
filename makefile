all: main spectrum0 spectrum1 spectrum2 spectrum3 localsocket occulocalsocket
.PHONY: all
main: main.o database.o hackrf_setting.o cJSON.o
	gcc -o main main.o database.o hackrf_setting.o cJSON.o -lpthread -lmysqlclient -lm -l wiringPi
	main.o: main.c api.h
	gcc -c main.c main.o
	cJSON.o: cJSON.c cJSON.h
	gcc -c cJSON.c cJSON.o
	database.o: database.c database.h api.h
	gcc -c database.c database.o 
	hackrf_setting.o: hackrf_setting.c hackrf_setting.h api.h
	gcc -c hackrf_setting.c hackrf_setting.o 
spectrum0: spectrum.o database.o hackrf_setting.o cJSON.o
	gcc -o spectrum0 spectrum.o database.o hackrf_setting.o -lpthread -lmysqlclient -lm
	spectrum.o: spectrum.c api.h
	gcc -c spectrum.c spectrum.o
	database.o: database.c database.h api.h
	gcc -c database.c database.o 
	hackrf_setting.o: hackrf_setting.c hackrf_setting.h api.h
	gcc -c hackrf_setting.c hackrf_setting.o
	cJSON.o: cJSON.c cJSON.h
	gcc -c cJSON.c cJSON.o
spectrum1: spectrum.o database.o hackrf_setting.o
	gcc -o spectrum1 spectrum.o database.o hackrf_setting.o -lpthread -lmysqlclient -lm
	spectrum.o: spectrum.c api.h
	gcc -c spectrum.c spectrum.o
	database.o: database.c database.h api.h
	gcc -c database.c database.o 
	hackrf_setting.o: hackrf_setting.c hackrf_setting.h api.h
	gcc -c hackrf_setting.c hackrf_setting.o
spectrum2: spectrum.o database.o hackrf_setting.o
	gcc -o spectrum2 spectrum.o database.o hackrf_setting.o -lpthread -lmysqlclient -lm
	spectrum.o: spectrum.c api.h
	gcc -c spectrum.c spectrum.o
	database.o: database.c database.h api.h
	gcc -c database.c database.o 
	hackrf_setting.o: hackrf_setting.c hackrf_setting.h api.h
	gcc -c hackrf_setting.c hackrf_setting.o
spectrum3: spectrum.o database.o hackrf_setting.o
	gcc -o spectrum3 spectrum.o database.o hackrf_setting.o -lpthread -lmysqlclient -lm
	spectrum.o: spectrum.c api.h
	gcc -c spectrum.c spectrum.o
	database.o: database.c database.h api.h
	gcc -c database.c database.o 
	hackrf_setting.o: hackrf_setting.c hackrf_setting.h api.h
	gcc -c hackrf_setting.c hackrf_setting.o
localsocket: localsocket.o
	gcc -o localsocket localsocket.o
	localsocket.o: localsocket.c
	gcc -c localsocket.c localsocket.o
occulocalsocket: occulocalsocket.o
	gcc -o occulocalsocket occulocalsocket.o
	occulocalsocket.o: occulocalsocket.c
	gcc -c occulocalsocket.c occulocalsocket.o
.PHONY: cleanall cleanobj 
cleanall: cleanobj
	#rm -rf *.o main
	#rm main.o database.o hackrf_setting.o
cleanobj: 
	rm *.o	

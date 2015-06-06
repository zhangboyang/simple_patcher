gcc -O2 -g -Wall -o make_script.exe make_script.c md5.c
gcc -O2 -g -Wall -o run_script.exe run_script.c md5.c
pause
strip make_script.exe
strip run_script.exe
pause
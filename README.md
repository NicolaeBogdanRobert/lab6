# Lab 6 - Memorie partajata si semafoare

Doua procese numara de la 1 la 1000 intr-o memorie partajata (shm_open + mmap),
sincronizate printr-un semafor POSIX cu nume (sem_open).

Compilare: gcc numarare.c -o numarare -pthread -lrt
Rulare:    ./numarare & ./numarare; wait

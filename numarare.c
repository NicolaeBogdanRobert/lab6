#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>       /* O_CREAT, O_RDWR */
#include <sys/mman.h>    /* shm_open, mmap */
#include <sys/stat.h>
#include <semaphore.h>   /* sem_open, sem_wait, sem_post */
#include <unistd.h>      /* ftruncate, getpid, usleep */
#include <time.h>

#define SHM_NAME "/lab6_shm"   /* numele memoriei partajate */
#define SEM_NAME "/lab6_sem"   /* numele semaforului */
#define MAX 1000

int main(void)
{
    /* seed diferit pentru fiecare proces */
    srand(time(NULL) ^ getpid());

    /* 1. Creez (sau deschid, daca exista deja) obiectul de memorie partajata */
    int fd = shm_open(SHM_NAME, O_CREAT | O_RDWR, 0666);
    if (fd == -1) { perror("shm_open"); return 1; }

    /* ii dau dimensiunea unui int (o memorie noua e initializata cu 0) */
    if (ftruncate(fd, sizeof(int)) == -1) { perror("ftruncate"); return 1; }

    /* 2. Mapez memoria in spatiul procesului */
    int *numar = mmap(NULL, sizeof(int), PROT_READ | PROT_WRITE,
                      MAP_SHARED, fd, 0);
    if (numar == MAP_FAILED) { perror("mmap"); return 1; }
    close(fd);

    /* 3. Creez/deschid semaforul, cu valoarea initiala 1 (liber) */
    sem_t *sem = sem_open(SEM_NAME, O_CREAT, 0666, 1);
    if (sem == SEM_FAILED) { perror("sem_open"); return 1; }

    int gata = 0;
    while (!gata) {
        /* rezerv semaforul - astept daca il are celalalt proces */
        sem_wait(sem);

        /* citesc memoria */
        int n = *numar;

        /* dau cu banul: cat timp cade 1, scriu urmatorul numar */
        while (n < MAX && rand() % 2 == 1) {
            n++;
            *numar = n;
            printf("[PID %d] am scris %d\n", getpid(), n);
            fflush(stdout);
        }

        if (n >= MAX)
            gata = 1;

        /* eliberez semaforul */
        sem_post(sem);

        /* pauza mica, ca sa aiba si celalalt proces sansa sa intre */
        usleep(1000);
    }

    printf("[PID %d] am terminat.\n", getpid());

    /* curatenie */
    munmap(numar, sizeof(int));
    sem_close(sem);
    shm_unlink(SHM_NAME);   /* daca a sters-o deja celalalt, nu e problema */
    sem_unlink(SEM_NAME);

    return 0;
}

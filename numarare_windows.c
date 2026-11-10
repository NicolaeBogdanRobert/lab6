#include <windows.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define SHM_NAME "Local\\lab6_shm"   /* numele memoriei partajate */
#define SEM_NAME "Local\\lab6_sem"   /* numele semaforului */
#define MAX 1000

int main(void)
{
    DWORD pid = GetCurrentProcessId();

    /* seed diferit pentru fiecare proces */
    srand((unsigned)time(NULL) ^ pid);

    /* 1. Creez (sau deschid, daca exista deja) obiectul mapat in memorie.
          INVALID_HANDLE_VALUE = memoria nu e legata de un fisier de pe disc.
          O memorie noua este initializata cu 0. */
    HANDLE hMap = CreateFileMappingA(INVALID_HANDLE_VALUE, NULL, PAGE_READWRITE,
                                     0, sizeof(int), SHM_NAME);
    if (hMap == NULL) {
        printf("CreateFileMapping a esuat: %lu\n", GetLastError());
        return 1;
    }

    /* 2. Mapez memoria in spatiul procesului */
    int *numar = (int *)MapViewOfFile(hMap, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(int));
    if (numar == NULL) {
        printf("MapViewOfFile a esuat: %lu\n", GetLastError());
        CloseHandle(hMap);
        return 1;
    }

    /* 3. Creez/deschid semaforul: valoare initiala 1 (liber), maxim 1 */
    HANDLE hSem = CreateSemaphoreA(NULL, 1, 1, SEM_NAME);
    if (hSem == NULL) {
        printf("CreateSemaphore a esuat: %lu\n", GetLastError());
        UnmapViewOfFile(numar);
        CloseHandle(hMap);
        return 1;
    }

    int gata = 0;
    while (!gata) {
        /* rezerv semaforul - astept daca il are celalalt proces */
        WaitForSingleObject(hSem, INFINITE);

        /* citesc memoria */
        int n = *numar;

        /* dau cu banul: cat timp cade 1, scriu urmatorul numar */
        while (n < MAX && rand() % 2 == 1) {
            n++;
            *numar = n;
            printf("[PID %lu] am scris %d\n", pid, n);
            fflush(stdout);
        }

        if (n >= MAX)
            gata = 1;

        /* eliberez semaforul */
        ReleaseSemaphore(hSem, 1, NULL);

        /* pauza mica, ca sa aiba si celalalt proces sansa sa intre */
        Sleep(1);
    }

    printf("[PID %lu] am terminat.\n", pid);

    /* curatenie - Windows sterge obiectele cand le inchide ultimul proces */
    UnmapViewOfFile(numar);
    CloseHandle(hMap);
    CloseHandle(hSem);

    return 0;
}
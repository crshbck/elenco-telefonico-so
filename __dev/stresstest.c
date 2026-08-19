#define _GNU_SOURCE
#include <assert.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/types.h>
#include <unistd.h>

#include "sync.h"

#define NUM_READERS 8
#define NUM_WRITERS 3
#define READ_OPERATIONS 50
#define WRITE_OPERATIONS 15

int rwlock_sem;

// Risorsa protetta dal RWLock
int shared_data = 0;

// Variabili ausiliarie di controllo e statistiche
int active_readers = 0;
int active_writers = 0;
int max_concurrent_readers = 0;

// Mutex standard usato SOLO per aggiornare le statistiche di test (non protegge shared_data)
pthread_mutex_t stats_mutex = PTHREAD_MUTEX_INITIALIZER;

// ==========================================
// FUNZIONI DEI THREAD
// ==========================================

void *reader_thread(void *arg)
{
	int id = (intptr_t) arg;

	for (int i = 0; i < READ_OPERATIONS; i++)
	{
		// 1. Acquisizione lock in lettura
		assert(rwlock_get_reader(&rwlock_sem));

		// --- INIZIO SEZIONE CRITICA ---
		pthread_mutex_lock(&stats_mutex);
		active_readers++;
		if (active_readers > max_concurrent_readers)
		{
			max_concurrent_readers = active_readers;
		}
		// Invariante: mentre legge, non ci devono essere scrittori attivi
		assert(active_writers == 0);
		pthread_mutex_unlock(&stats_mutex);

		// Simulazione lettura
		int val = shared_data;
		(void) val;
		usleep(500); // 0.5 ms per aumentare la sovrapposizione tra lettori

		pthread_mutex_lock(&stats_mutex);
		active_readers--;
		pthread_mutex_unlock(&stats_mutex);
		// --- FINE SEZIONE CRITICA ---

		// 2. Rilascio lock in lettura
		assert(rwlock_release_reader(&rwlock_sem));

		// Breve pausa fuori dalla sezione critica
		usleep(200);
	}

	printf("[Reader #%d] Completate %d letture.\n", id, READ_OPERATIONS);
	return NULL;
}

void *writer_thread(void *arg)
{
	int id = (intptr_t) arg;

	for (int i = 0; i < WRITE_OPERATIONS; i++)
	{
		// 1. Acquisizione lock in scrittura
		assert(rwlock_get_writer(&rwlock_sem));

		// --- INIZIO SEZIONE CRITICA ---
		pthread_mutex_lock(&stats_mutex);
		active_writers++;
		// Invariante: solo 1 scrittore alla volta e 0 lettori
		assert(active_writers == 1);
		assert(active_readers == 0);
		pthread_mutex_unlock(&stats_mutex);

		// Simulazione scrittura: incremento del dato condiviso
		int temp = shared_data;
		usleep(2000); // 2 ms per stressare la mutua esclusione
		shared_data = temp + 1;

		pthread_mutex_lock(&stats_mutex);
		active_writers--;
		pthread_mutex_unlock(&stats_mutex);
		// --- FINE SEZIONE CRITICA ---

		// 2. Rilascio lock in scrittura
		assert(rwlock_release_writer(&rwlock_sem));

		// Pausa fuori dalla sezione critica
		usleep(1000);
	}

	printf("[Writer #%d] Completate %d scritture.\n", id, WRITE_OPERATIONS);
	return NULL;
}

// ==========================================
// MAIN TEST SUITE
// ==========================================

int _main(void)
{
	printf("=== AVVIO TEST CONCORRENZA RWLOCK ===\n");
	printf("Configurazione: %d Lettori (%d op/cad), %d Scrittori (%d op/cad)\n\n", NUM_READERS,
		   READ_OPERATIONS, NUM_WRITERS, WRITE_OPERATIONS);

	// 1. Inizializzazione RWLock
	if (!rwlock_init(&rwlock_sem))
	{
		fprintf(stderr, "Errore inizializzazione semafori.\n");
		return EXIT_FAILURE;
	}

	pthread_t readers[NUM_READERS];
	pthread_t writers[NUM_WRITERS];

	// 2. Creazione thread scrittori e lettori
	for (intptr_t i = 0; i < NUM_READERS; i++)
	{
		if (pthread_create(&readers[i], NULL, reader_thread, (void *) i) != 0)
		{
			perror("pthread_create reader");
			return EXIT_FAILURE;
		}
	}

	for (intptr_t i = 0; i < NUM_WRITERS; i++)
	{
		if (pthread_create(&writers[i], NULL, writer_thread, (void *) i) != 0)
		{
			perror("pthread_create writer");
			return EXIT_FAILURE;
		}
	}

	// 3. Attesa terminazione di tutti i thread
	for (int i = 0; i < NUM_READERS; i++)
	{
		pthread_join(readers[i], NULL);
	}
	for (int i = 0; i < NUM_WRITERS; i++)
	{
		pthread_join(writers[i], NULL);
	}

	// 4. Verifica finale dei risultati
	int expected_writes = NUM_WRITERS * WRITE_OPERATIONS;
	printf("\n=== RISULTATI TEST ===\n");
	printf("Scritture attese: %d | Valore finale shared_data: %d\n", expected_writes, shared_data);
	printf("Massimo numero di lettori simultanei registrato: %d\n", max_concurrent_readers);

	assert(shared_data == expected_writes);
	assert(max_concurrent_readers > 1);

	printf("\nTEST COMPLETATO CON SUCCESSO: Tutte le asserzioni sono verificate.\n");

	// 5. Cleanup delle risorse IPC
	rwlock_destroy(&rwlock_sem);

	return EXIT_SUCCESS;
}

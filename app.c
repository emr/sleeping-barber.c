#include <stdio.h>
#include <signal.h>
#include <unistd.h>
#include <semaphore.h>
#include <pthread.h>
#include <stdlib.h>

struct Config {
	unsigned int seats_count;
	unsigned int barbers_count;
	unsigned int customers_count;
};

struct State {
	unsigned int free_seats_count;
	unsigned int total_happy_customers;
	unsigned int total_unhappy_customers;
};

struct Barber {
	unsigned int id;
	pthread_t worker;
};

struct Customer {
	unsigned int id;
	pthread_t worker;
};

sem_t access_seats, barber_ready, customer_ready;
struct State state;

struct Config configure();
void initialize(struct Config *config);
void progress(struct Config *config);
void terminate(int sig_num);
void * barber(void *arg);
void * customer(void *arg);
void operate();
void complete();

int main()
{
	signal(SIGINT, terminate);
	struct Config config = configure();
	initialize(&config);
	progress(&config);

	return 0;
}

struct Config configure()
{
	struct Config config;

	printf("Berber sayisi: ");
	scanf("%d", &config.barbers_count);

	printf("Koltuk sayisi: ");
	scanf("%d", &config.seats_count);

	printf("Musteri sayisi: ");
	scanf("%d", &config.customers_count);

	return config;
}

void initialize(struct Config *config)
{
	state.free_seats_count = config->seats_count;

	sem_init(&access_seats, 0, 1);
	sem_init(&barber_ready, 0, 0);
	sem_init(&customer_ready, 0, 0);

	printf("Dukkan acildi.\n");
}

void progress(struct Config *config)
{
	struct Barber barbers[config->barbers_count];
	struct Customer customers[config->customers_count];
	int i;
	for (i = 0; i < config->barbers_count; i++) {
		barbers[i].id = i+1;
		pthread_create(&barbers[i].worker, NULL, barber, (void *)&barbers[i].id);
	}
	for (i = 0; i < config->customers_count; i++) {
		customers[i].id = i+1;
		pthread_create(&customers[i].worker, NULL, customer, (void *)&customers[i].id);
	}
	for (i = 0; i < config->barbers_count; i++) {
		pthread_join(barbers[i].worker, NULL);
	}
	for (i = 0; i < config->customers_count; i++) {
		pthread_join(customers[i].worker, NULL);
	}
}

void terminate(int sig_num)
{
	sem_destroy(&access_seats);
	sem_destroy(&customer_ready);
	sem_destroy(&barber_ready);
	printf(
		"\nSimulasyon tamamlandi. Toplamda %d musteri kabul edildi. %d musteri geri cevirildi.\n",
		state.total_happy_customers,
		state.total_unhappy_customers
	);
	exit(0);
}

void * barber(void *arg)
{
	int id = *(int *) arg;

	printf("Berber %d ise geldi.\n", id);

	while (1) {
		printf("Berber %d uyuyor.\n", id);
		sem_wait(&customer_ready);
		sem_wait(&access_seats);
		printf("Berber %d uyandi.\n", id);
		++state.free_seats_count;
		sem_post(&access_seats);
		sem_post(&barber_ready);
		printf("Berber %d calisiyor.\n", id);
		operate(id);
		printf("Berber %d isini bitirdi.\n", id);
	}
}

void * customer(void *arg)
{
	int id = *(int *) arg;

	printf("Musteri %d geldi.\n", id);

	sem_wait(&access_seats);
	if (state.free_seats_count == 0) {
		sem_post(&access_seats);
		++state.total_unhappy_customers;
		printf("Musteri %d mutsuz ayrildi.\n", id);
		return 0;
	}
	state.free_seats_count -= 1;
	sem_post(&access_seats);
	sem_post(&customer_ready);
	sem_wait(&barber_ready);
	complete(id);
}

void operate(int id)
{
	// Customer: Yanlari al ustler kalsin abi.
	sleep(1);
	state.total_happy_customers += 1;
}

void complete(int id)
{
	// Barber: Sihhatler olsun.
	// Customer: Tesekkurler, ellerinize saglik.
	printf("Musteri %d mutlu ayrildi.\n", id);
}

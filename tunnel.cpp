#include <pthread.h>
#include <signal.h>
#include <stdio.h>
#include <string.h>
#include <cstring>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include <stdlib.h>
#include <string>

using namespace std;

static string traffic;
static int done = 0;
static int maxNCars;
static int totalNCars;
static int nCarsInTunnel;
static int wCars;
static int bCars;
static int dCars;
static pthread_mutex_t traffic_lock = PTHREAD_MUTEX_INITIALIZER;
static pthread_cond_t wb_can = PTHREAD_COND_INITIALIZER;
static pthread_cond_t bb_can = PTHREAD_COND_INITIALIZER;

struct Car{
	int arrive;
	int cross;
	string bound;
	int num;
};
struct Car cars[128];

void *carW(void *arg);
void *carB(void *arg);
void *tunnelp(void *arg);

void readCars(std::string filename) {
    std::ifstream ifs(filename.c_str());
    if (ifs){
        ifs >> maxNCars;
        int i = 0;
        while(ifs >> cars[i].arrive >> cars[i].bound >> cars[i].cross) {
        	cars[i].num = i+1;
        	i++;
        }
        totalNCars = i;
    }
    else {
        cout << "Cannot open file." <<endl;
    }
    ifs.close();
}

int main (int argc, char* argv[])
{
    if (argc < 2)
    {
        cerr << "Usage : ./assignment3 <inputFile>" << endl;
        return 0;
    }
	char *fileName;
    fileName = argv[1];
    readCars(fileName);

	pthread_t tunnel;
	pthread_create(&tunnel, NULL, tunnelp, NULL);

	pthread_t cartid[totalNCars];
	for (int j=0; j<totalNCars; j++) {
		sleep(cars[j].arrive);
		if(cars[j].bound == "WB"){
			pthread_create(&cartid[j], NULL, carW, (void*) &cars[j] );
		}
		if(cars[j].bound == "BB"){
			pthread_create(&cartid[j], NULL, carB, (void*) &cars[j] );
		}
	}
	for (int i=0; i<totalNCars; i++){
		pthread_join(cartid[i], NULL);
	}
	done = 1;
	cout << wCars << " car(s) going to Whittier arrived at the tunnel." << endl;
	cout << bCars << " car(s) going to Bear Valley arrived at the tunnel." << endl;
	cout << dCars << " car(s) were delayed." << endl;
}

void *carW(void *arg) {
	pthread_mutex_lock(&traffic_lock);
	struct Car *wCar;
	wCar = (struct Car *)arg;
	string myDirection = "W";
	cout << "Car #" << wCar->num << " going to Whittier arrives at the tunnel." << endl;
	while (traffic != myDirection || nCarsInTunnel == maxNCars){
		if (traffic == myDirection && nCarsInTunnel >= maxNCars) {
			dCars++;
			cout << "car #" << wCar->num << " delayed." << endl;
		}
		pthread_cond_wait(&wb_can, &traffic_lock);	
	}
	nCarsInTunnel++;
	cout << "Car #" << wCar->num << " going to Whittier enters the tunnel." << endl;
	pthread_mutex_unlock(&traffic_lock);
	sleep(wCar->cross);

	pthread_mutex_lock(&traffic_lock);
	nCarsInTunnel--;
	pthread_cond_broadcast(&wb_can);
	cout << "Car #" << wCar->num << " going to Whittier exits the tunnel." << endl;
	wCars++;
	pthread_mutex_unlock(&traffic_lock);
	pthread_exit((void*) 0);
}

void *carB(void *arg) {
	pthread_mutex_lock(&traffic_lock);
	struct Car *bCar;
	bCar = (struct Car *)arg;
	string myDirection = "B";
	cout << "Car #" << bCar->num << " going to Bear Valley arrives at the tunnel." << endl;
	while (traffic != myDirection || nCarsInTunnel == maxNCars){
		if (traffic == myDirection && nCarsInTunnel >= maxNCars) {
			dCars++;
			cout << "car #" << bCar->num << " delayed." << endl;
		}
		pthread_cond_wait(&bb_can, &traffic_lock);
	}
	nCarsInTunnel++;
	cout << "Car #" << bCar->num << " going to Bear Valley enters the tunnel." << endl;
	pthread_mutex_unlock(&traffic_lock);
	sleep(bCar->cross);

	pthread_mutex_lock(&traffic_lock);
	nCarsInTunnel--;
	pthread_cond_broadcast(&bb_can);
	cout << "Car #" << bCar->num << " going to Bear Valley exits the tunnel." << endl;
	bCars++;
	pthread_mutex_unlock(&traffic_lock);
	pthread_exit((void*) 0);
}

void *tunnelp(void *arg){
	while(done == 0) {
		pthread_mutex_lock(&traffic_lock);
		traffic = 'W';
		cout << "The tunnel is now open to Whittier-bound traffic." << endl;
		pthread_cond_broadcast(&wb_can);
		pthread_mutex_unlock(&traffic_lock);
		sleep(5);

		pthread_mutex_lock(&traffic_lock);
		traffic = 'N';
		cout << "The tunnel is now closed to ALL traffic." << endl;
		pthread_mutex_unlock(&traffic_lock);
		sleep(5);

		pthread_mutex_lock(&traffic_lock);
		traffic = 'B';
		cout << "The tunnel is now open to Bear-Valley-bound traffic." << endl;
		pthread_cond_broadcast(&bb_can);
		pthread_mutex_unlock(&traffic_lock);
		sleep(5);

		pthread_mutex_lock(&traffic_lock);
		traffic = 'N';
		cout << "The tunnel is now closed to ALL traffic." << endl;
		pthread_mutex_unlock(&traffic_lock);
		sleep(5);
	}
	pthread_exit((void*) 0);
}
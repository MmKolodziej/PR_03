// authors: Ernest Przestrzelski, Kamil Starczak, Piotr Szyperski, Rafał Szczekutek

#include <mpi.h>
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <unistd.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <time.h>
#include <vector>

using namespace MPI;
using namespace std;

const int waiting_time = 250; //msec
MPI_Datatype mapPoint;

struct coords {
    int X;
    int Y;
};

bool found();
vector<coords> readTreasureMap(char *);
void readMap(char * input, coords* tab);

int main (int argc, char **argv)
{
    int rank, size;
    int n, k, i, local_sum = 0, global_sum = -1;
    char mapa_def[] = "./mapa.txt";
    char* mapa = mapa_def;

    n = 4;
    k = 3;

    if(argc >= 4)
        mapa = (char *)argv[3];
    if (argc >= 3)
        k = atoi(argv[2]);
    if (argc >= 2)
        n = atoi(argv[1]);

    // Inicjalizacja środowiska
    MPI_Init(&argc, &argv);
    size = COMM_WORLD.Get_size();
    rank = COMM_WORLD.Get_rank();

    srand(time(NULL) * rank); // Generator liczb pseudolosowych inicjalizowany w zależności od czasu i numeru węzła

    // Liczba poszukiwaczy musi być równa liczbie dostępnych slotów z pliku hosts
    if(size != n)
    {
        cout << "Wrong number of processes, should be: " << n << " and actually is: " << size << endl;
        Finalize();
        return 0;
    }

    // Tworzenie typu użytkownika używanego do przesyłania w funkcjach Send, Recv, itp. (struktura o dwóch parametrach typu INT)
    MPI_Datatype type[2] = { MPI_INT, MPI_INT }; // Typy w strukturze
    int blocklen[2] = { 1, 1 }; // Ilość każdego ww typu w strukturze

    Aint disp[2] = { // Przesunięcie w pamięci dla struktury
       offsetof(coords, X),
       offsetof(coords, Y)
    };

    MPI_Type_struct(2, blocklen, disp, type, &mapPoint);
    MPI_Type_commit(&mapPoint); // Od tej pory w programie dostępny jest typ 'mapPoint'

    coords points[k]; // Tablica struktur, w której zostaną umieszczone lokacje używane przez Poszukiwaczy
    coords allPoints[n*k]; // Tablica struktur, w której zostaną umieszczone wszyskie lokacje z pliku wejściowego 'mapa' - tam gdzie nie ma wraku jest (0,0)
    vector<coords> v;

    if(rank == 0) // Dowódca
    {
        cout << "I am Commander and I'm starting to read the wreck's map." << endl;
        //v = readTreasureMap(mapa); // Odczytanie mapy i zapisanie lokacji wektora v
        //copy(v.begin(), v.end(), allPoints); // kopiowanie zawartości wektora v do tablicy wszystkich lokacji
		readMap(mapa, allPoints);
    }

    cout << "I'm Finder number " << rank << ". Now I'm waiting for instructions from Commander." << endl;
    COMM_WORLD.Scatter(&allPoints, k, mapPoint, &points, k, mapPoint, 0);

    for(i = 0; i < k; i++)
    {
        cout << "Finder " << rank << " checks a place " << i << ": (" << points[i].X << "," << points[i].Y << ")" << endl;
        if(found()) // Jeśli wrak jest, zwiększam wartość znalezionych wraków
        {
            local_sum++;
            cout << "Finder " << rank << " reports: wreck at (" << points[i].X << "," << points[i].Y << ")" << endl;
        }
        else // Jeśli nie ma, ustawiam daną lokację na null - (0,0)
        {
            points[i].X = 0;
            points[i].Y = 0;
        }
    }

    // Dowódca zbiera (sumuje) od Poszukiwaczy informacje o liczbie znalezionych wraków
    COMM_WORLD.Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0);

    // Dowódca zbiera od każdego Poszukiwacza k lokacji (wysłanych wcześniej), które zawierają informacje o istnieniu wraku
    COMM_WORLD.Gather(&points, k, mapPoint, &allPoints, k, mapPoint, 0);

	if(rank == 0) // Dowódca
    {
        cout << "Wrecks found: " << global_sum << endl;
        for(i = 0; i < n*k; i++)
            if(allPoints[i].X != 0 || allPoints[i].Y != 0)
                cout << "Wreck "<< i << ": (" << allPoints[i].X << "," << allPoints[i].Y << ")" <<endl;
    }

    MPI_Type_free(&mapPoint);
    Finalize();
    return 0;
}

// 20% szansy
bool found()
{
    usleep(waiting_time * 1000);
    return rand() % 5 == 0 ? true : false;
}

// parsowanie
void readMap(char * input, coords* tab)
{
	FILE* f = fopen(input, "rt");
	//char* line;
	int i = 0;
	int a,b;

	while(!feof(f)) {
		fscanf(f, "%d, %d\n", &a, &b);
		tab[i].X = a;
		tab[i].Y = b;
		i++;
	}

	fclose(f);
}

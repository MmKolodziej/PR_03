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

MPI_Datatype mapPoint;

struct point{
    int x,y;
};

char* getParamValue(char *s);
bool found();
vector<point> readTreasureMap(char *);
void readMap(char * input, point* tab);
void AddMPIPointType();
void FreeMPIPointType();
string pointToString(point);

int main (int argc, char **argv)
{
  int rank, size;
  int i, local_sum = 0, global_sum = -1;

  if (argc!=4){
  	cout<<"Nieprawidlowa liczba parametrow. Poprawne uzycie ./main n=[n] k=[k] mapa=[mapa]"<<endl;
    MPI_Finalize();
  	return 0;
  }

  int n = atoi(getParamValue(argv[1]));
  int k = atoi(getParamValue(argv[2]));
  char *mapa = getParamValue(argv[3]);
  cout<<"n:"<<n<<" , k:"<<k<<" , mapa:"<<mapa<<endl;

  // Inicjalizacja MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank( MPI_COMM_WORLD, &rank ); //numer procesu
  MPI_Comm_size( MPI_COMM_WORLD, &size ); //Rozmiar grupy

  //Losowanie w zaleznosci od czasu i numeru procesu
  srand(time(NULL) + rank);

  // Liczba poszukiwaczy musi być równa liczbie dostępnych slotów z pliku hosts
  if(size != n){
    cout << "Nieprawidlowa liczba procesow. Docelowa: " << n << " , Aktualna: " << size << endl;
    MPI_Finalize();
    return 0;
  }

  AddMPIPointType();
  // Tworzenie typu użytkownika używanego do przesyłania w funkcjach Send, Recv, itp. (struktura o dwóch parametrach typu INT)
  point points[k]; // Tablica struktur, w której zostaną umieszczone lokacje używane przez Poszukiwaczy
  point allPoints[n*k]; // Tablica struktur, w której zostaną umieszczone wszyskie lokacje z pliku wejściowego 'mapa' - tam gdzie nie ma wraku jest (0,0)
  vector<point> v;

  if(rank == 0){
    cout << "I'm the Commander and I'm starting to read the wreck's map." << endl;
    readMap(mapa, allPoints);
  }

  //Rozeslij po k lokalizacji do kazdego z poszukiwaczy
  MPI_Scatter(&allPoints, k, mapPoint, &points, k, mapPoint, 0, COMM_WORLD);

  cout << "I'm Finder number " << rank << ". Now I'm waiting for instructions from Commander." << endl;
   for(i = 0; i < k; i++)
  {
    cout << "Finder " << rank << " checks a place " << i << ": " << pointToString(points[i]) << endl;
    if(found()) // Jeśli wrak jest, zwiększam wartość znalezionych wraków
    {
      local_sum++;
      cout << "Finder " << rank << " reports: wreck at " << pointToString(points[i]) << endl;
    }
    else // Jeśli nie ma, ustawiam daną lokację na null - (0,0)
    {
      points[i].x = 0;
      points[i].y = 0;
    }
  }
  
  
  MPI_Barrier(COMM_WORLD);
  if (rank==0) {
    cout<<"Wszyscy poszukiwacze wrocili. Podsumowuje poszukiwania..."<<endl;
  }  

// Dowódca zbiera (sumuje) od Poszukiwaczy informacje o liczbie znalezionych wraków
  MPI_Reduce(&local_sum, &global_sum, 1, MPI_INT, MPI_SUM, 0, COMM_WORLD);

  // Dowódca zbiera od każdego Poszukiwacza k lokacji (wysłanych wcześniej), które zawierają informacje o istnieniu wraku
  MPI_Gather(&points, k, mapPoint, &allPoints, k, mapPoint, 0, COMM_WORLD);

  if(rank == 0) // Dowódca
  {
    cout << "Wrecks found: " << global_sum << endl;
    for(i = 0; i < n*k; i++)
      if(allPoints[i].x != 0 || allPoints[i].y != 0)
        cout << "Wreck "<< i << ": " << pointToString(allPoints[i]) <<endl;
  }
  FreeMPIPointType();
  MPI_Finalize();
  return 0;
}

char* getParamValue(char *s) {
  vector<char*> v;
  char* chars_array = strtok(s, "=");
  while(chars_array){
    v.push_back(chars_array);
    chars_array = strtok(NULL, "=");
  }
  return v[1];
}

string pointToString(point loc){
  stringstream ss;
  ss << "(" << loc.x << "," << loc.y << ")";
  return ss.str();
}

void AddMPIPointType()
{
  MPI_Datatype type[2] = { MPI_INT, MPI_INT }; // Typy w strukturze
  int blocklen[2] = { 1, 1 }; // Ilość każdego ww typu w strukturze

  Aint disp[2] = { // Przesunięcie w pamięci dla struktury
    offsetof(point, x),
    offsetof(point, y)
  };

  MPI_Type_struct(2, blocklen, disp, type, &mapPoint);
  MPI_Type_commit(&mapPoint); // Od tej pory w programie dostępny jest typ 'mapPoint'
}

void FreeMPIPointType()
{
	MPI_Type_free(&mapPoint);
}

// 20% szansy
bool found()
{
  usleep(rand()%400000+100000);
  return (rand() % 5 == 0);
}

// parsowanie
void readMap(char * input, point* tab)
{
	FILE* f = fopen(input, "rt");
	int i = 0;
	int a,b;

	while(!feof(f)) {
		fscanf(f, "%d, %d\n", &a, &b);
		tab[i].x = a;
		tab[i].y = b;
		i++;
	}

	fclose(f);
}

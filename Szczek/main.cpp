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

using namespace std;

MPI_Datatype MPIPoint;

struct point{
    int x,y;
};

char* getParamValue(char *s);
bool searchForWreckage();
void readMap(char * input, point* tab);
void AddMPIPointType();
void FreeMPIPointType();
string pointToString(point);

int main (int argc, char **argv)
{
  int rank, size;
  int i;
  int currentWreckagesCount, allWreckagesCount;

  if (argc!=4){
  	cout<<"Nieprawidlowa liczba parametrow. Poprawne uzycie ./main n=[n] k=[k] mapa=[mapa]"<<endl;
    MPI_Finalize();
  	return 0;
  }

  int n = atoi(getParamValue(argv[1]));
  int k = atoi(getParamValue(argv[2]));
  char *mapa = getParamValue(argv[3]);
  point currentPoints[k]; // Tablica struktur, w której zostaną umieszczone lokacje używane przez Poszukiwaczy
  point allPoints[n*k]; // Tablica struktur, w której zostaną umieszczone wszyskie lokacje z pliku wejściowego 'mapa' - tam gdzie nie ma wraku jest (0,0)
  
  // Inicjalizacja MPI
  MPI_Init(&argc, &argv);
  MPI_Comm_rank( MPI_COMM_WORLD, &rank ); //numer procesu
  MPI_Comm_size( MPI_COMM_WORLD, &size ); //Rozmiar grupy

 if (rank==0) cout<<"n:"<<n<<" , k:"<<k<<" , mapa:"<<mapa<<endl;

  //Losowanie w zaleznosci od czasu i numeru procesu
  srand(time(NULL) + rank);

  // Czy n zgadza sie z liczba wymaganych procesow?
  if(size != n){
    cout << "Nieprawidlowa liczba procesow. Docelowa: " << n << " , Aktualna: " << size << endl;
    MPI_Finalize();
    return 0;
  }

  //Deklaruje specjalny typ potrzebny do wymiany informacji pomiedzy procesami
  AddMPIPointType();
  
  if(rank == 0){
    cout << "Dowodca czyta mape..." << endl;
    readMap(mapa, allPoints);
  }

  //Rozeslij po k lokalizacji do kazdego z poszukiwaczy
  MPI_Scatter(&allPoints, k, MPIPoint, &currentPoints, k, MPIPoint, 0, MPI_COMM_WORLD);

  for(i = 0; i < k; i++)
  {
    cout << "Poszukiwacz " << rank << " sprawdza miejsce " << pointToString(currentPoints[i]) << endl;
    if(searchForWreckage()) // Zwraca true, jezeli znaleziono wrak
    {
      currentWreckagesCount++;
      cout << "\033[1;32mPoszukiwacz " << rank << " znalazl wrak w " << pointToString(currentPoints[i]) << "\033[0m\n";
    }
    else 
    {
     //jesl9i nie ma wraku, ustaw NULL
      currentPoints[i].x = 0;
      currentPoints[i].y = 0;
    }
  }

  //Poczekaj na wszystkich poszukiwaczy
  MPI_Barrier(MPI_COMM_WORLD);
  if (rank==0)
    cout<<"\033[1;34mWszyscy poszukiwacze wrocili. Podsumowuje poszukiwania:\033[0m\n";
  
 // Proces 0 (dowodca) sumuje zlicza liczbe wszystkich znalezionych wrakow
  MPI_Reduce(&currentWreckagesCount, &allWreckagesCount, 1, MPI_INT, MPI_SUM, 0, MPI_COMM_WORLD);

  // Proces 0 (dowodca) gromadzi informacje o odwiedzonych miejscach od poszczegolnych poszukiwaczy i zapisuje je w allPoints
  MPI_Gather(&currentPoints, k, MPIPoint, &allPoints, k, MPIPoint, 0, MPI_COMM_WORLD);

  if(rank == 0)
  {
    int wreckageNumber = 1;
    cout << "Znaleziono wrakow: " << allWreckagesCount << endl;
    for(i = 0; i < n*k; i++)
    {
      //Jezeli dany punkt nie jest = NULL
      if(!(allPoints[i].x == 0 && allPoints[i].y == 0))
        cout << "\033[1;33mWrak "<< wreckageNumber++ << ": " << pointToString(allPoints[i]) << "\033[0m\n";
    }
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

  MPI_Aint disp[2] = { // Przesunięcie w pamięci dla struktury
    offsetof(point, x),
    offsetof(point, y)
  };

  MPI_Type_struct(2, blocklen, disp, type, &MPIPoint);
  MPI_Type_commit(&MPIPoint); // Od tej pory w programie dostępny jest typ 'MPIPoint'
}

void FreeMPIPointType()
{
  MPI_Type_free(&MPIPoint);
}

bool searchForWreckage()
{
  usleep(rand()%400000+100000);
  return (rand() % 5 == 0);
}

void readMap(char * mapFile, point* points)
{
  FILE* f = fopen(mapFile, "rt");
  int i = 0;
  int x,y;

  while(!feof(f)) {
    fscanf(f, "%d, %d\n", &x, &y);
    points[i].x = x;
    points[i].y = y;
    i++;
  }

  fclose(f);
}

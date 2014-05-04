#include <unistd.h>
#include <mpi.h>
#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <time.h>
#include <vector>

using namespace MPI;
using namespace std;

struct location {
    int x;
    int y;
};

bool found();
location* parseMap(char * input, int n, int k);

int main (int argc, char **argv)
{
  char* mapa_file = "./mapa";
  int n = 4;
  int k = 3;
  if (argc==4){
    n = atoi(argv[1]);
    k = atoi(argv[2]);
    mapa_file = argv[3];
  }
  int size, rank;
  location* points;
  MPI_Init(&argc, &argv);
  size = COMM_WORLD.Get_size();
  rank = COMM_WORLD.Get_rank();
  if (rank==0){
    points = parseMap(mapa_file,n,k);
    int i = 0;
    for (i=0; i<n*k; i++)
      cout << points[i].x << " " << points[i].y << endl;
  }
  Finalize();
  return 0;
}

// 20% szansy
bool found()
{
  usleep(1* 1000);
  return rand() % 5 == 0;
}

// parsowanie
location* parseMap(char * input, int n, int k)
{
  FILE* f = fopen(input, "rt");
  location points[n*k];
  int i = 0;
  int a,b;

  while(!feof(f)) {
    fscanf(f, "%d, %d\n", &a, &b);
    points[i].x = a;
    cout << "a: " << endl;
    points[i].y = b;
    cout << "b: " << endl;
    i++;
  }
  fclose(f);
  return points;
}

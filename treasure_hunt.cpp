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

//using namespace MPI;
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
  location* points;
  points = parseMap(mapa_file,n,k);

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
    points[i].y = b;
    cout << a << " " << b << endl;
    i++;
  }
  fclose(f);
  return points;
}

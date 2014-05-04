#include <cstddef>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <iostream>
#include <mpi.h>
#include <sstream>
#include <string>
#include <time.h>
#include <unistd.h>
#include <vector>

using namespace MPI;
using namespace std;

MPI_Datatype mapPoint;

struct location {
    int x;
    int y;
};

bool found();
void readMap(char * input, coords* tab);

int main (int argc, char **argv)
{

  return 0;
}

// 20% szansy
bool found()
{
  usleep(waiting_time * 1000);
  return rand() % 5 == 0;
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

#include <stdio.h>
#include <unistd.h>
#include <sys/types.h>
#include <stdlib.h>
#include <pthread.h>
#include "prototype.h"


int main (int argc, char *argv[]) {
  args *arg;
  double *a, *x;
  int n, p, res, i;
  pthread_t *tids;
  double time;
  int err = 0;
  if (argc < 3 || argc > 4 || (n = atoi (argv[1])) <= 0 || (p = atoi (argv[2])) <= 0) {
    printf (" Usage: %s <n> <p> [<name>] \n", argv[0]);
    return 1;
  }
  a = new double[n * n];
  x = new double[n * n];
  tids = new pthread_t[p];
  arg = new args[p];
  if (!(a && x && tids && arg)) {
    printf ("Cannot allocate memory!\n");
    if (!a)
      delete []a;
    if (!x)
      delete []x;
    if (!tids)
      delete []tids;
    if (!arg)
      delete []arg;
    return 1;
  }

  if (argc == 4) {
    res = read_matrix (argv[3], a, n);
    if (res < 0) {
      delete []a;
      delete []x;
      delete []arg;
      delete []tids;
      switch (res) {
        case ERR_CANNOT_OPEN:
            printf ("Can not open\n");
          return ERR_CANNOT_OPEN;
        case ERR_CANNOT_READ:
            printf ("Can not read\n");
          return ERR_CANNOT_READ;
        case ERR_NOT_ENOUGH_ELEMENTS:
            printf ("Not enough elements\n");
          return ERR_NOT_ENOUGH_ELEMENTS;
        default:
            printf ("Error\n");
          return ERR_UNKNOWN;
      }
    }
  }
  else
    init_matrix (a, n);
  printf ("Matrix A:\n");
  print_matrix (a, n);
  for (i = 0; i < p; i++) {
      arg[i].n = n;
      arg[i].p = p;
      arg[i].id = i;
      arg[i].a = a;
      arg[i].x = x;
      arg[i].err = &err;
  }
  for (int i = 0; i < n; i++)
    x[i*n+i] = 1.0;
  for (i = 1; i < p; i++) {
    if (pthread_create (tids + i, 0, &process, arg + i)) {
      printf ("Cannot create thread %d\n", i);
      delete []a;
      delete []x;
      delete []arg;
      delete []tids;
      abort();
    }
  }
  time = get_full_time ();
  process (arg + 0);
  for (i = 1; i < p; i++)
      pthread_join (tids[i], 0);
  if (err) {
    printf ("Wrong matrix %d\n", err);
    delete []arg;
    delete []tids;
    delete []x;
    delete []a;
    return -1;
  }
  transpose(x, n);
  time = get_full_time () - time;
  printf ("Inverse A:\n");
  print_matrix (x, n);
  printf ("\n");
  if (argc == 4)
    res = read_matrix (argv[3], a, n);
  else
    init_matrix (a, n);
  printf ("Full Time: %f\n", time);
  printf ("Time in every thread:\n");
  for(i = 0; i < p; i++)
    printf ("Thread %d: %f\n",i , arg[i].time_thr);
  printf ("\n");
  printf("Discrepancy = %g\n", Discrepancy (a, x, n));
  delete []a;
  delete []x;
  delete []arg;
  delete []tids;
  return 0;
}

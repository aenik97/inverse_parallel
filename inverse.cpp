#include "prototype.h"


double f(int i, int j, int k) {
  //return 1/(i+j+1.0);
  //return fabs(i - j) + 1 + k - k;
  if (i > j)
    return k - i;
  else
    return k - j;
}

void init_matrix (double *a, int n) {
    for (int i = 0; i < n; i++)
        for (int j = 0; j < n; j++)
            a[i * n + j] = f (i, j, n);
}

int read_matrix (const char *name, double *a, int n) {
    FILE *fp;
    fp = fopen (name , "r");
    if (fp == 0)
        return ERR_CANNOT_OPEN;
    for (int i = 0; i < n * n; i++)
        if (fscanf (fp, "%lf", a + i) != 1) {
            if (feof (fp) == 1) {
                fclose (fp);
                return ERR_NOT_ENOUGH_ELEMENTS;
            }
            fclose (fp);
            return ERR_CANNOT_READ;
        }
    fclose (fp);
    return 0;
}

void print_matrix (double *a, int n) {
    printf("\n");
    for (int i = 0; (i < n) && (i < MAX_PRINT); i++) {
        for (int j = 0; (j < n) && (j < MAX_PRINT); j++)
            printf("%10.3f   ", a[i * n + j]);
        printf("\n");
    }
    printf("\n");
}

void transpose (double *b, int n) {
    double res = 0.0;
    for (int i = 0; i < n - 1; i++)
        for (int j = i + 1; j < n; j++) {
            res = b[j * n + i];
            b[j * n + i] = b[i * n + j];
            b[i * n + j] = res;
        }
    return;
}

/*double get_time() {
    struct rusage buf;
    getrusage (RUSAGE_THREAD, &buf);
    return (double)buf.ru_utime.tv_sec + (double)buf.ru_utime.tv_usec / 1000000.;
}*/

double get_full_time() {
    struct timeval buf;
    gettimeofday (&buf, 0);
    return (double)buf.tv_sec + (double)buf.tv_usec / 1000000.;
}

double Discrepancy (double *a, double *b, int n) {
    double *d, *c, s = 0, S = 0, ans = 0;
    for (int i = 0; i < n; i++) {
        S = 0;
        for (int j = 0; j < n; j++) {
            d = a + i * n;
            c = b + j;
            s = 0;
            for (int t = 0; t < n; t++) {
                s += d[t] * c[t * n];
            }
            if (i == j)
                s--;
            S += fabs(s);
        }
        if (S > ans)
            ans = S;
    }
    return ans;
}

void synchronize (int total_threads)
{
  static pthread_mutex_t mutex      = PTHREAD_MUTEX_INITIALIZER;
  static pthread_cond_t condvar_in  = PTHREAD_COND_INITIALIZER;
  static pthread_cond_t condvar_out = PTHREAD_COND_INITIALIZER;
  static int threads_in  = 0;
  static int threads_out = 0;

  pthread_mutex_lock (&mutex);
  
  threads_in++;
  
  if (threads_in >= total_threads) {
    threads_out = 0;
    pthread_cond_broadcast (&condvar_in);
  } else 
    while (threads_in < total_threads) 
      pthread_cond_wait (&condvar_in, &mutex);
      
  threads_out++;

  if (threads_out >= total_threads) {
    threads_in = 0;
    pthread_cond_broadcast (&condvar_out);
  } else 
    while (threads_out < total_threads)
      pthread_cond_wait (&condvar_out, &mutex);
      
  pthread_mutex_unlock (&mutex);
}

int norm(double *a, double *b,int n, int i, int k, int j) {
  double l = sqrt(a[k*n + i]*a[k*n+i] + a[j*n+i]*a[j*n+i]);
  /*if (l < EPS) {
    printf("%d--%d--%d\n", i,k,j);
    print_matrix(a,n);
    return -1;
  }*/
  if (fabs(l) < EPS)
    return 0;
  double cos = a[k*n + i]*(1.0 / l);
  double sin = -a[j*n + i]*(1.0 / l);
  a[k*n + i] = l;
  a[j*n + i] = 0.0;
  for (int u = i+1; u < n; u++) {
    double tmp1 = a[k*n + u];
    double tmp2 = a[j*n + u];
    a[k*n + u] = tmp1 * cos - tmp2 * sin;
    a[j*n + u] = tmp1 * sin + tmp2 * cos;
  }
  for (int u = 0; u < n; u++) {
    double tmp1 = b[u*n + k];
    double tmp2 = b[u*n + j];
    b[u*n+k] = tmp1 * cos - tmp2 * sin;
    b[u*n+j] = tmp1 * sin + tmp2 * cos;
  }
    return 0;
}
void *process (void *c) {
  args *A =(args*)c;
  int n = A -> n;
  int p = A -> p;
  int id = A -> id;
  double *a = A -> a;
  double *b = A -> x;
  double t1;
  t1 = get_full_time ();
  double tmp1;
  int ret;
  for (int i = 0; i < n - 1; i++) {
      int d_par = 1;
      int d_blo = 2 * p;
      while (i + d_par < n) {
          for (int k = 0; i + 2 * id * d_par + d_par + d_blo * k < n; k++) {
            int first_row = i + 2 * id * d_par + d_blo * k;
            int second_row = i + 2 * id * d_par + d_par + d_blo * k;
            norm (a, b, n, i, first_row, second_row);
          }
        d_par *= 2;
        d_blo *= 2;
        synchronize(p);
      }
      if (fabs(a[i * n + i]) < EPS) {
        *(A->err) = 1;
        return 0;
      }
  }
  synchronize(p);
  if (fabs(a[n*n-1]) < EPS) {
    *(A->err) = 2;//выход bad Matrix;
    return 0;
  }
  synchronize(p);
  for (int k = id; k < n; k+=p)
    for (int i = n - 1; i >= 0; i--) {
      tmp1 = b[i + k*n];
      for (int j = i + 1; j < n; j++)
        tmp1 -= a[i*n+j] * b[j+k*n];
      b[i+k*n] = tmp1 * (1.0 / a[i*n+i]);
  }
  A -> time_thr = get_full_time () - t1;
  return 0;
}

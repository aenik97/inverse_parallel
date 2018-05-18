#ifndef STRUCT_QUEUE_H
#define STRUCT_QUEUE_H

#include <cstdio>
#include <cstdlib>
#include <cfloat>
#include <string.h>
#include <ctime>
#include <sys/time.h>
#include <sys/resource.h>
#include <cmath>
#include <pthread.h>
#include <sched.h>

#define ERR_CANNOT_OPEN -1
#define ERR_CANNOT_READ -2
#define ERR_NOT_ENOUGH_ELEMENTS -3
#define ERR_UNKNOWN -4
#define MAX_PRINT 8
#define EPS 1e-14


void synchronize (int total_threads);
double f(int i, int j, int k);
void init_matrix (double *a, int n);
int read_matrix (const char *name, double *a, int n);
void print_matrix (double *a, int n);
void transpose (double *a, int n);
double get_time();
double get_full_time();
double Discrepancy (double *a, double *b, int n);
void turn(double * a, double * b, int k, double cos, double sin);
//void synch (int p, double *a = 0, int n = 0, int i = 0, bool *err = 0, double *help = 0);
//void synch1 (int p, double *a, int n, int i, double *help);
void *process (void *a);

struct queue_node{
private:
    queue_node * next;
    int key;
    friend struct queue;
public:
    queue_node() {next = 0; key = 0;}
    ~queue_node() {next = 0; key = 0;}
};


struct queue {
private:
    int currsize;
    queue_node * head;
    queue_node * top;
public:
    queue() {
        head = 0;
        top = 0;
    }
    ~queue() {
        while (head) {
            queue_node * tmp = head;
            head = head -> next;
            delete tmp;
        }
        top = 0;
    }
    int get_currsize() {return currsize;}
    queue_node * get_head() {return head;}
    bool emty() {return head == 0;}
    queue_node* get() {return top;}
    void add_element(queue_node* add) {
        if (head) {
                top -> next = add;
                top = add;
        } else {
        	head = add;
        	top = add;
        }
        currsize++;
    }
	int pop() {
    	if (head == 0)
    		return -1;
		queue_node *tmp = head;
    	head = head -> next;
    	if (tmp == top)
    		top = 0;
    	delete tmp;
    	return 0;
	}
};

struct args {
  int n, p;
  int id;
  double time_thr;
  double *a;
  double *x;
  int *err;
  args () {
      id = 0;
      n = 0;
      p = 0;
      time_thr = 0;
      a = 0;
      x = 0;
      err = 0;
    }
};
#endif

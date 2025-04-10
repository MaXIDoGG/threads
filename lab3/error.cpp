#include <pthread.h>
#include <stdio.h>
int a = 0;
pthread_mutex_t m;
void* thread_func(void*) {
 pthread_mutex_lock(&m);
 a++;
 return 0;
}
int main() {
 pthread_t t[2];
 pthread_mutex_init(&m, NULL);
 pthread_create(&t[0], NULL, thread_func, NULL);
 pthread_create(&t[1], NULL, thread_func, NULL);
 pthread_join(t[0], NULL);
 pthread_join(t[1], NULL);
 pthread_mutex_destroy(&m);
 return 0;
}

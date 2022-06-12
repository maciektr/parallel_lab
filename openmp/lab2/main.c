# include <omp.h>
# include <stdio.h>
#include <stdlib.h>

# define PROG 10000000

int main ( int argc , char *argv[] ) {
    int N ;
    int i ;
    int *t;

    omp_set_num_threads(atoi(argv[1]));
    omp_set_nested(atoi(argv[2])) ;
    omp_set_dynamic(atoi(argv[3])) ;

    printf ( "Podaj ilosc losowan: \n" ) ;
    scanf ( "%d" ,&N ) ;
    printf("Ilosc losowan: %d\n", N);

    t = (int *)calloc(N, sizeof(int));


    for(int nt = 1; nt<=4; nt++) {
        double time = omp_get_wtime();
        #pragma omp parallel num_threads(nt) private(i)
        {
            int ct = omp_get_thread_num(); 
            if ( ct == 0 ) 
                printf("Liczba wątków=%d\n" ,omp_get_num_threads() ) ;

            unsigned int myseed = omp_get_thread_num();
            for(i=omp_get_thread_num();i<N;i+=4)
                t[i] = rand_r(&myseed);
        }
        printf("Time: %f\n", omp_get_wtime() - time);
    }
    
    free(t);
}
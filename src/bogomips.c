/*
 * Based on code Linux kernel code in init/main.c and include/linux/delay.h
 * and the example code by Jeff Tranter (Jeff_Tranter@Mitel.COM)
 */

#include <stdio.h>
#include <mpi.h>
#include <unistd.h>

// #define PORTABLE_BOGOMIPS
#define CLASSIC_BOGOMIPS

#ifdef CLASSIC_BOGOMIPS
    /* the original code from the Linux kernel */
    int HZ = 100;

    #define rdtscl(low) \
         __asm__ __volatile__ ("rdtsc" : "=a" (low) : : "edx")

    //This delay() is the one used on x86's with TSC after 2.2.14.
    //It won't work on a non TSC x86, period.
    void __inline__ delay(unsigned long loops)
    {
        unsigned long bclock, now;
        rdtscl(bclock);
        do {
    	rdtscl(now);
        }
        while ((now - bclock) < loops);
    }
#endif

#ifdef PORTABLE_BOGOMIPS
    /* portable version */
    static void delay(int loops)
    {
        long i;
        for (i = loops; i >= 0; i--);
    }
#endif

/* this should be approx 2 Bo*oMips to start (note initial shift), and will
 *    still work even if initially too large, it will just take slightly longer */
unsigned long loops_per_jiffy = (1 << 12);

/* This is the number of bits of precision for the loops_per_jiffy.  Each
 *    bit takes on average 1.5/HZ seconds.  This (like the original) is a little
 *       better than 1% */
#define LPS_PREC 8

int numprocs, rank, namelen;
char processor_name[MPI_MAX_PROCESSOR_NAME];

//plagiarized straight from the 2.4 sources.
float calibrate_delay(void)
{
    unsigned long ticks, loopbit;
    int lps_precision = LPS_PREC;
    loops_per_jiffy = (1 << 12);
    while (loops_per_jiffy <<= 1) {
	ticks = clock();
	while (ticks == clock())
	    /* nothing */ ;
	ticks = clock();
	delay(loops_per_jiffy);
	ticks = clock() - ticks;
	if (ticks)
	    break;
    }
    loops_per_jiffy >>= 1;
    loopbit = loops_per_jiffy;
    while (lps_precision-- && (loopbit >>= 1)) {
	loops_per_jiffy |= loopbit;
	ticks = clock();
	while (ticks == clock());
	ticks = clock();
	delay(loops_per_jiffy);
	if (clock() != ticks)
	    loops_per_jiffy &= ~loopbit;
    }
    return (loops_per_jiffy / (500000/HZ)) + (float)((loops_per_jiffy/(5000/HZ))%100) / (float)100;
}

int main(int argc, char *argv[])
{
    unsigned long loops_per_sec = 1;
    unsigned long ticks;
    int i;

    MPI_Status stat;
    MPI_Init(&argc, &argv);
    MPI_Comm_size(MPI_COMM_WORLD, &numprocs);
    MPI_Comm_rank(MPI_COMM_WORLD, &rank);
    MPI_Get_processor_name(processor_name, &namelen);

    float bogomips = 0;
    for ( i = 0; i < 9; i++ ) {
        bogomips += calibrate_delay();
    }
    bogomips = bogomips / (float) 10;

    printf( "[%02d/%02d %s] returned = %f BogoMips\n", rank, numprocs, processor_name, bogomips );

    if ( rank == 0 ) {
        float totalBogomips = bogomips;
        for ( i = 1; i < numprocs; i++ ) {
            float f = 0;
            MPI_Recv(&f, 1, MPI_FLOAT, i, 0, MPI_COMM_WORLD, &stat);
            totalBogomips += f;
        }
        printf( "Total = %f BogoMips\n", totalBogomips );
    } else {
        MPI_Send(&bogomips, 1, MPI_FLOAT, 0, 0, MPI_COMM_WORLD);
    }

    MPI_Finalize();
    return 0;
}

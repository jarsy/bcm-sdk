/* $Id: stone.c,v 1.3 2011/07/21 16:14:43 yshtil Exp $
 * Stone - a short tunable optimize networker experince 
 * performance test.
 */
/* standard C library headers required */
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

/* the following is optional depending on the timing function used */
#include <time.h>

#include <tickLib.h>

int array1[200] = {0};
int array2[200] = {0};

int memory_1 = 0;

void copy1();
void copy2();
void copy3();
void copy4();
void copy5();
void copy6();
void copy7();
void copy8();
void copy9();
void copy10();
void copy11();
void copy12();
void copy13();
void copy14();
void copy15();
void copy16();
void copy17();
void copy18();
void copy19();

void copy20();
void copy21();
void copy22();
void copy23();
void copy24();
void copy25();
void copy26();
void copy27();
void copy28();
void copy29();

void copy30();
void copy31();
void copy32();
void copy33();
void copy34();
void copy35();
void copy36();
void copy37();
void copy38();
void copy39();

void copy40();
void copy41();
void copy42();
void copy43();
void copy44();
void copy45();
void copy46();
void copy47();
void copy48();
void copy49();

void copy50();
void copy51();
void copy52();
void copy53();
void copy54();
void copy55();
void copy56();
void copy57();
void copy58();
void copy59();

void copy60();
void copy61();
void copy62();
void copy63();
void copy64();
void copy65();
void copy66();
void copy67();
void copy68();
void copy69();

void copy70();
void copy71();
void copy72();
void copy73();
void copy74();
void copy75();
void copy76();
void copy77();
void copy78();
void copy79();

void copy80();
void copy81();
void copy82();
void copy83();
void copy84();
void copy85();
void copy86();
void copy87();
void copy88();
void copy89();

void copy90();
void copy91();
void copy92();
void copy93();
void copy94();
void copy95();
void copy96();
void copy97();
void copy98();
void copy99();

void copy100();


void jump1();
void jump2();
void jump3();
void jump4();
void jump5();
void jump6();
void jump7();
void jump8();
void jump9();
void jump10();
void jump11();
void jump12();
void jump13();
void jump14();
void jump15();
void jump16();
void jump17();
void jump18();
void jump19();

void jump20();
void jump21();
void jump22();
void jump23();
void jump24();
void jump25();
void jump26();
void jump27();
void jump28();
void jump29();

void jump30();
void jump31();
void jump32();
void jump33();
void jump34();
void jump35();
void jump36();
void jump37();
void jump38();
void jump39();

void jump40();
void jump41();
void jump42();
void jump43();
void jump44();
void jump45();
void jump46();
void jump47();
void jump48();
void jump49();

void jump50();
void jump51();
void jump52();
void jump53();
void jump54();
void jump55();
void jump56();
void jump57();
void jump58();
void jump59();

void jump60();
void jump61();
void jump62();
void jump63();
void jump64();
void jump65();
void jump66();
void jump67();
void jump68();
void jump69();

void jump70();
void jump71();
void jump72();
void jump73();
void jump74();
void jump75();
void jump76();
void jump77();
void jump78();
void jump79();

void jump80();
void jump81();
void jump82();
void jump83();
void jump84();
void jump85();
void jump86();
void jump87();
void jump88();
void jump89();

void jump90();
void jump91();
void jump92();
void jump93();
void jump94();
void jump95();
void jump96();
void jump97();
void jump98();
void jump99();

void jump100();




void multi1();

void init_array(voi)
{
    int i;

    for (i = 0; i < 200; i++)
    {
        array1[i] = array2[i] = 0;
    }
    memory_1 = 0;    
}

int
stone_main(int loops)
{
    int iterations;
   long startsec, finisec;

    init_array();
    startsec = tickGet();
    /*startsec = time(0);*/

    for (iterations = 1; iterations < loops; ++iterations)
    {
        
        copy1();
        multi1();
        jump1();

    }


    finisec = tickGet();
    /*finisec = time(0);*/

    printf("\n");


    if (finisec-startsec <= 0)
    {
        printf("Insufficient duration- Increase the LOOP count\n");
        return(1);
    }

    printf("iiterations: %d, Duration: %ld ticks (%ld sec.)\n",
           iterations, (finisec-startsec), (finisec-startsec)/sysClkRateGet());

    return(0);
}

int
stone_main_long(int loops)
{
    int iterations;
   long startsec, finisec;

    init_array();

    startsec = tickGet();
    /*startsec = time(0);*/

    for (iterations = 1; iterations < loops; ++iterations)
    {
        
        copy1();
        copy2();
        copy3();
        copy4();
        copy5();
        copy6();
        copy7();
        copy8();
        copy9();
        copy10();
        copy11();
        copy12();
        copy13();
        copy14();
        copy15();
        copy16();
        copy17();
        copy18();
        copy19();
        copy20();
        copy21();
        copy22();
        copy23();
        copy24();
        copy25();
        copy26();
        copy27();
        copy28();
        copy29();
        copy30();
        copy31();
        copy32();
        copy33();
        copy34();
        copy35();
        copy36();
        copy37();
        copy38();
        copy39();
        copy40();
        copy41();
        copy42();
        copy43();
        copy44();
        copy45();
        copy46();
        copy47();
        copy48();
        copy49();
        copy50();
        copy51();
        copy52();
        copy53();
        copy54();
        copy55();
        copy56();
        copy57();
        copy58();
        copy59();
        copy60();
        copy61();
        copy62();
        copy63();
        copy64();
        copy65();
        copy66();
        copy67();
        copy68();
        copy69();
        copy70();
        copy71();
        copy72();
        copy73();
        copy74();
        copy75();
        copy76();
        copy77();
        copy78();
        copy79();
        copy80();
        copy81();
        copy82();
        copy83();
        copy84();
        copy85();
        copy86();
        copy87();
        copy88();
        copy89();
        copy90();
        copy91();
        copy92();
        copy93();
        copy94();
        copy95();
        copy96();
        copy97();
        copy98();
        copy99();
        copy100();


        jump1();
        jump2();
        jump3();
        jump4();
        jump5();
        jump6();
        jump7();
        jump8();
        jump9();
        jump10();
        jump11();
        jump12();
        jump13();
        jump14();
        jump15();
        jump16();
        jump17();
        jump18();
        jump19();
        jump20();
        jump21();
        jump22();
        jump23();
        jump24();
        jump25();
        jump26();
        jump27();
        jump28();
        jump29();
        jump30();
        jump31();
        jump32();
        jump33();
        jump34();
        jump35();
        jump36();
        jump37();
        jump38();
        jump39();
        jump40();
        jump41();
        jump42();
        jump43();
        jump44();
        jump45();
        jump46();
        jump47();
        jump48();
        jump49();
        jump50();
        jump51();
        jump52();
        jump53();
        jump54();
        jump55();
        jump56();
        jump57();
        jump58();
        jump59();
        jump60();
        jump61();
        jump62();
        jump63();
        jump64();
        jump65();
        jump66();
        jump67();
        jump68();
        jump69();
        jump70();
        jump71();
        jump72();
        jump73();
        jump74();
        jump75();
        jump76();
        jump77();
        jump78();
        jump79();
        jump80();
        jump81();
        jump82();
        jump83();
        jump84();
        jump85();
        jump86();
        jump87();
        jump88();
        jump89();
        jump90();
        jump91();
        jump92();
        jump93();
        jump94();
        jump95();
        jump96();
        jump97();
        jump98();
        jump99();
        jump100();

    }


    finisec = tickGet();
    /*finisec = time(0);*/

    printf("\n");


    if (finisec-startsec <= 0)
    {
        printf("Insufficient duration- Increase the LOOP count\n");
        return(1);
    }

    /*printf("Iterations: %d, Duration: %ld sec.\n",
           iterations, finisec-startsec);*/
    printf("Iterations: %d, Duration: %ld ticks (%ld sec.)\n",
           iterations, (finisec-startsec), (finisec-startsec)/sysClkRateGet());

    return(0);
}






/* just a test function, which copies a chunk of data between two arrays 100 times */
void copy1()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

 

void copy2()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy3()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy4()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy5()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy6()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy7()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy8()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy9()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy10()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy11()
{        
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy12()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy13()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy14()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy15()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy16()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy17()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy18()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy19()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy20()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy21()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy22()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy23()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy24()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy25()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy26()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy27()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy28()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy29()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}



void copy30()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy31()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy32()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy33()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy34()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy35()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy36()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy37()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy38()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy39()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}
void copy40()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy41()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy42()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy43()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy44()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy45()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy46()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy47()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy48()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy49()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}
void copy50()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy51()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy52()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy53()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy54()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy55()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy56()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy57()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy58()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy59()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}
void copy60()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy61()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy62()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy63()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy64()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy65()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy66()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy67()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy68()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy69()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}
void copy70()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy71()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy72()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy73()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy74()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy75()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy76()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy77()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy78()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy79()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}
void copy80()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy81()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy82()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy83()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy84()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy85()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy86()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy87()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy88()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy89()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}
void copy90()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy91()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy92()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy93()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy94()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy95()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy96()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy97()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy98()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}

void copy99()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}
void copy100()
{
    int i;
    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]+1+i+memory_1;
    }
}
 






void multi1()
{
    int i;

    for (i=1; i<100; ++i)
    {
        memory_1 = array1[i] = array2[i+2]-1+i*(i+memory_1);
    }
}


void jump1()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump2()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump3()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump4()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump5()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump6()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump7()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump8()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump9()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump10()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump11()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump12()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump13()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump14()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump15()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump16()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump17()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump18()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump19()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}






void jump20()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump21()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump22()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump23()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump24()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump25()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump26()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump27()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump28()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump29()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}






void jump30()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump31()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump32()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump33()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump34()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump35()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump36()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump37()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump38()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump39()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}






void jump40()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump41()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump42()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump43()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump44()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump45()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump46()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump47()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump48()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump49()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}






void jump50()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump51()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump52()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump53()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump54()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump55()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump56()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump57()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump58()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump59()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}






void jump60()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump61()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump62()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump63()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump64()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump65()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump66()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump67()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump68()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump69()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}






void jump70()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump71()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump72()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump73()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump74()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump75()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump76()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump77()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump78()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump79()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}






void jump80()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump81()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump82()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump83()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump84()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump85()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump86()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump87()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump88()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump89()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}






void jump90()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump91()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump92()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump93()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump94()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump95()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump96()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump97()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump98()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}



void jump99()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}






void jump100()
{

    int i;

    for (i=1; i<100; ++i)
    {


        switch (memory_1%100)
        {
        

        case 2:
            memory_1 = memory_1 +5243 +i; break;
        case 3:
            memory_1 = memory_1 +5253 +i; break; 
        case 4:
            memory_1 = memory_1 +5263 +i; break; 
        case 5:
            memory_1 = memory_1 +523 +i; break; 
        case 6:
            memory_1 = memory_1 +5623 +i; break; 

        case 0:

            memory_1 = memory_1 +523 +i; 
            break; 
        case 1:
            memory_1 = memory_1 +5223 +i; break;



        case 7:
            memory_1 = memory_1 +5273 +i; break; 
        case 8:
            memory_1 = memory_1 +5823 +i; break; 
        case 9:
            memory_1 = memory_1 +5233 ; break; 

        case 27:
            memory_1 = memory_1 +5263 +i; break; 
        case 28:
            memory_1 = memory_1 +5263 +i; break; 
        case 29:
            memory_1 = memory_1 +5263 +i; break; 
        case 30:
            memory_1 = memory_1 +5263 +i; break; 
        case 31:
            memory_1 = memory_1 +5263 +i; break; 
        case 32:
            memory_1 = memory_1 +523 +i; break; 
        case 33:
            memory_1 = memory_1 +5423 +i; break; 



        case 10:               
            memory_1 = memory_1 +5323 +i; break; 
        case 11:
            memory_1 = memory_1 +5233 +i; break; 
        case 12:
            memory_1 = memory_1 +523 +i; break; 
        case 13:
            memory_1 = memory_1 +55523 +i; break; 
        case 14:
            memory_1 = memory_1 +5623 +i; break; 
        case 15:
            memory_1 = memory_1 +5273 +i; break; 
        case 16:
            memory_1 = memory_1 +5273 +i; break; 
        case 17:
            memory_1 = memory_1 +5283 +i; break; 
        case 18:              
            memory_1 = memory_1 +5023 +i; break; 
        case 19:
            memory_1 = memory_1 +5253 +i; break; 
        case 20:
            memory_1 = memory_1 +5423 +i; break; 
        case 21:
            memory_1 = memory_1 +52223 +i; break; 
        case 22:
            memory_1 = memory_1 +5223 +i; break; 
        case 23:
            memory_1 = memory_1 +5243; break; 
        case 24:
            memory_1 = memory_1 +5423 +i; break; 
        case 25:
            memory_1 = memory_1 +5253 +i; break; 
        case 26:
            memory_1 = memory_1 +5263 +i; break; 
        case 34:
            memory_1 = memory_1 +5243 +i; break; 
        case 35:
            memory_1 = memory_1 +5323 +i; break; 
        case 36:
            memory_1 = memory_1 +5233 +i; break; 
        case 37:
            memory_1 = memory_1 +5233 +i; break; 
        case 38:
            memory_1 = memory_1 +5523 +i; break; 
        case 39:
            memory_1 = memory_1 +523 +i; break; 
        case 48:
            memory_1 = memory_1 +52377 +i; break; 
        case 49:
            memory_1 = memory_1 +5223 +i; break; 
        case 50:
            memory_1 = memory_1 +5239 +i; break; 

        case 40:
            memory_1 = memory_1 +52223 +i; break; 
        case 41:
            memory_1 = memory_1 +523 +i; break; 
        case 42:
            memory_1 = memory_1 +523 +i; break; 
        case 43:
            memory_1 = memory_1 +23 +i; break; 
        case 44:
            memory_1 = memory_1 +7523 +i; break; 
        case 45:
            memory_1 = memory_1 +5723 +i; break; 
        case 46:
            memory_1 = memory_1 +5273 +i; break; 
        case 47:
            memory_1 = memory_1 +5237 +i; break; 

        case 51:
            memory_1 = memory_1 +5293 +i; break; 
        case 52:
            memory_1 = memory_1 +5923 +i; break; 
        case 53:
            memory_1 = memory_1 +5923 +i; break; 
        case 54:
            memory_1 = memory_1 +9523 +i/2; break; 
        case 55:
            memory_1 = memory_1 +5923 +i; break; 

        case 72:
            memory_1 = memory_1 +5823 +i; break; 
        case 73:
            memory_1 = memory_1 +5283 +i; break; 
        case 74:
            memory_1 = memory_1 +5283 +i; break; 
        case 75:
            memory_1 = memory_1 +5283 +i; break; 
        case 76:
            memory_1 = memory_1 +5823 +i; break; 
        case 77:
            memory_1 = memory_1 +5283 +i; break; 
        case 78:
            memory_1 = memory_1 +5283 +i; break; 
        case 79:
            memory_1 = memory_1 +5723 +i; break; 
        case 80:
            memory_1 = memory_1 +5273 +i; break; 
        case 81:
            memory_1 = memory_1 +7523 +i; break; 
        case 56:
            memory_1 = memory_1 +5293 +i/2; break; 
        case 57:
            memory_1 = memory_1 +5293 +i; break; 
        case 58:
            memory_1 = memory_1 +5293 +i; break; 
        case 59:
            memory_1 = memory_1 +5293 +i; break; 
        case 60:
            memory_1 = memory_1 +5923 +i; break; 
        case 61:
            memory_1 = memory_1 +5293 +i; break; 
        case 62:
            memory_1 = memory_1 +88523 +i; break; 
        case 63:
            memory_1 = memory_1 +5283 +i; break; 
        case 64:
            memory_1 = memory_1 +5823 +i; break; 
        case 65:
            memory_1 = memory_1 +5283 +i; break; 
        case 66:
            memory_1 = memory_1 +88523 +i; break; 
        case 67:
            memory_1 = memory_1 +5283 +i; break; 
        case 68:
            memory_1 = memory_1 +5823 +i; break; 
        case 69:
            memory_1 = memory_1 +5283 +i; break; 
        case 70:
            memory_1 = memory_1 +5283 +i; break; 
        case 71:
            memory_1 = memory_1 +5283 +i; break; 
        case 82:
            memory_1 = memory_1 +5723 +i; break; 
        case 83:
            memory_1 = memory_1 +5273 +i; break; 
        case 93:
            memory_1 = memory_1 +5213 +i; break; 
        case 94:
            memory_1 = memory_1 +5231 +i; break; 
        case 95:
            memory_1 = memory_1 +52311 +i; break; 
        case 96:
            memory_1 = memory_1 +1523 +i; break; 
        case 97:
            memory_1 = memory_1 +5123 +i; break; 
        case 98:
            memory_1 = memory_1 +5213 +i; break; 
        case 99:
            memory_1 = memory_1 +5231 +i; break; 
        case 100:
            memory_1 = memory_1 +5231 +i; break; 

        case 84:
            memory_1 = memory_1 +5237 +i; break; 
        case 85:
            memory_1 = memory_1 +52377 +i; break; 
        case 86:
            memory_1 = memory_1 +523 +i; break; 
        case 87:
            memory_1 = memory_1 +5623 +i; break; 
        case 88:
            memory_1 = memory_1 +5263 +i; break; 
        case 89:
            memory_1 = memory_1 +5236 +i; break; 
        case 90:
            memory_1 = memory_1 +52366; break; 
        case 91:
            memory_1 = memory_1 +56236 +i; break; 
        case 92:
            memory_1 = memory_1 +66523 +i; break; 

        }
    }

}







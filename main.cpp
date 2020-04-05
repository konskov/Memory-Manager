#include <cstddef>
#include <iostream>
#include <array>
#include <cstdint>
#include <unistd.h>
#include <assert.h>
#include <time.h>
#include "mem_manager.h"
using namespace std;


int main(int argc, char **argv)
{
    int* array[1000];
    clock_t start, end; 
    start = clock();
         
    for (int i = 0; i < 5000; i++)
    {
        //cout << "iteration " << i << endl;
        for (int j = 0; j < 1000; j++)
        {
            //array[j] = new int(j);
            array[j] = new int(j);
            // cout << (array[j])->getRe() << ' ' << array[j]->getIm() << endl;
            
        }
        for (int j = 0; j < 1000; j++)
        {
            //delete array[j];
            delete array[999-j];
        }

    }
    
    end = clock();
    cout << "time taken " << double (end - start)/double(CLOCKS_PER_SEC) << " sec" << endl; 
    return 0;
}
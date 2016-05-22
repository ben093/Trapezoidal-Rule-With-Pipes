#include "math.h"
#include "childData.h"
#include "unistd.h"
#include "sys/wait.h"
#include <iostream>

double callFunction(double x)
{
 double value = pow(x,2) + (2*x) + 4;

 return value;
}

int main(int argc, char* argv[])
{ 
    childData temp; 
    int len; 
      
    while(true)
	{ 
        len = read(*argv[1], &temp, sizeof(temp)); 
        if(len != sizeof(temp))
		{ 
            cout << "Error reading child from pipe." << endl; 
            close(*argv[1]); 
            return 0; 
        } 
        if(temp.value == -1)
		{  // Close the open pipes.
            close(*argv[1]); 
            close(*argv[2]); 
            break; 
        } 
		
		double a = callFunction(temp.left);
		double b = callFunction(temp.right);
		
		temp.result = ((a+b)/2) * temp.delta;
        
        len = write(*argv[2], &temp, sizeof(temp)); 
		
        if(len != sizeof(temp))
		{ 
            cout << "Error writing child back to master." << endl; 
            close(*argv[2]); 
            return 0; 
        } 
    } 
    return 0; 
} 

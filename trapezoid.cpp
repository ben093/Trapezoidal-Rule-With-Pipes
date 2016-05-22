/*=============================================================
	COURSE:	CSC460, assignment3
	PROGRAMMERS: 	Ben (bb), Anthony (av)
	MODIFIED BY: Ben, Anthony
	LAST MODIFIED DATE:	4/1/2016
	DESCRIPTION:	This program is an implementation of the trapezoidal rule
					using pipes and non-duplicated child processes
	NOTE: 50% Ben - Step 4, 5, 6. callFunction.cpp, childData.h, 
							  trapezoid.cpp
		  50% Anthony - Step 1, 2, 3. makefile, trapezoid.cpp
	FILES:	makefile, trapezoid.cpp, callFunction.cpp, childData.h, 
			trapezoid, child
	IDE/COMPILER/INTERPRETER:	GNU g++
	INSTRUCTION FOR COMPILATION AND EXECUTION:
	1. make	to COMPILE
	2. type:	./trapezoid left right n m to EXECUTE
=============================================================*/

#include "iostream"
#include "stdio.h"
#include "stdlib.h"
#include "time.h"
#include <iomanip>
#include <ctype.h>
#include "vector"
#include "math.h"
#include "unistd.h"
#include "sys/wait.h"
#include "childData.h"

using namespace std;

#define READ 0
#define WRITE 1

int pipes[8][2];
int masterPipe[2];
double leftEnd, rightEnd;

void writePipe(childData temp)
{
    int len = write(pipes[temp.pipeIndex][WRITE], &temp, sizeof(temp));
    if(len != sizeof(temp))
	{
        cout << "Error in write to child pipe." << endl;
        exit(0);
    }
}

childData readPipe()
{
    childData temp;
    int len = read(masterPipe[READ], &temp, sizeof(temp));
    if(len != sizeof(temp))
	{
        cout << "Error reading from master pipe." << endl;
        exit(0);
    }
    return temp;
}

int main(int argCount, char* argList[])
{
  double n = 0, m = 0;
  
  // ******************************* 
  // Step 1. Sanitize input arguments.
  // *******************************
  if(argCount != 5)
  { // Validate number of arguments
    cout << "Program takes 4 input parameters. " << endl <<
			"Program should be ran like ./trapezoid <left> <right> <n> <m>" << endl;
	cout << "\t<left>: The left end of the integral." << endl;
	cout << "\t<right>: The right end of the integral." << endl;
	cout << "\t<m>: The number of processes. (0 < m <= 8)" << endl;
	cout << "\t<n>: The number of sub-intervals." << endl;
    return 0;
  }

  for (int i=1; i < argCount; i++) 
  { // Validate entries to only allow double inputs.
    char* end;
    double val = strtod(argList[i], &end);
    if (end[0])
    { // end[0] is the first non-numeric argument.
      cout << "Error: Invalid input. Input parameter " << i << " must be numeric." << endl;
      return 0;
    }
    else
    {  // Input appears valid so far.
       if(i == 1)
       { // First iteration is leftEnd
         leftEnd = val;
       }
       else if(i == 2)
       { // Second iteration is rightend
         rightEnd = val;
       }
	   else if(i == 3)
	   { // Third iteration is number of sub-intervals
		 n = val;  
		 if(n <= 0)
		 {
			 cout << "Error: n must be greater than 0." << endl;
			 return 0;
		 }
	   }
	   else if(i == 4)
	   { // Fourth iteration is number of processes to spawn.
		 m = val;
		 if(m < 1 || m > 8)
		 {
			 cout << "Error: m must be > 0 and <= 8." << endl;
		 }
	   }
    }
  }
    
    if(leftEnd >= rightEnd)
    { // Left end cannot be greater than or equal to the right end.
      cout <<  "Left end must be less than the right end." << endl;
      return 0;
    }
  
    // ******************************* 
    // Step 2. START THE MAIN PART OF THE PROGRAM
    // *******************************
  
    int numOfProcesses = min(m, n);    
	    
	double deltaX = (double)(rightEnd - leftEnd)/ n;
	
	pipe(masterPipe);
	
	vector<childData> trapezoids;
		
	
	// ******************************* 
    // Step 3. Create sub-intervals
    // *******************************
	for(int i = 0; i < n; i++)
	{
		childData subInterval;	
		
		subInterval.left = leftEnd + (i * deltaX);
		subInterval.right = subInterval.left + deltaX;
		subInterval.delta = deltaX;
		subInterval.identifier = i;
		
		trapezoids.push_back(subInterval);
	}
	
	int writeCount = 0;	
	for(int i = 0; i < numOfProcesses; i++)
	{
		pipe(pipes[i]);
		
		int pid = fork();
		
		if(pid == -1)
		{
			cout << "Failed to fork a child process." << endl;
			return 0;
		}
		else if(pid == 0)
		{	// Child code
			// Execute function
			execl("child","",&pipes[i][READ], &masterPipe[WRITE],NULL); 
            cout << "Failed to create new process." << endl; 
            exit(0); 
		}
		else
		{	
			// Set pipe index
			trapezoids[writeCount].pipeIndex = i;
			
			// Have parent write to pipe
			writePipe(trapezoids[writeCount]);
			writeCount++;
		}
	}
	
	// ******************************* 
    // Step 4. Have the parent listen to master pipe.
    // *******************************	
	int readCount = 0;
	double answer = 0;
	while(readCount < n)
	{
		childData temp; 
		temp = readPipe();
		
		readCount++;
		
		answer += temp.result;
		
		// Check if more work to be done.
		if(writeCount < n)
		{	
			trapezoids[writeCount].pipeIndex = temp.pipeIndex;
	
			// Place in pipe that just came back.	
			writePipe(trapezoids[writeCount]);
			writeCount++;
		}
	}
  
  // ******************************* 
  // Step 5. Close the pipes.
  // *******************************	
  for(int i = 0; i < numOfProcesses; i++)
  { 
    childData temp;
	temp.pipeIndex = i;
	
	// value is our terminate signal.
	temp.value = -1; 
	
    writePipe(temp); 
	
    close(pipes[i][READ]); 
    close(pipes[i][WRITE]); 
  } 
  
  close(masterPipe[READ]); 
  close(masterPipe[WRITE]); 
  
  // Set precision to allow more decimal places.
  cout << setprecision(9) << "Trapezoid Approximation: " << answer << endl;
  
  // ******************************* 
  // Step 6. Get counts for each process.
  // *******************************	
  vector<int> count;
  for(int i = 0; i < numOfProcesses; i++)
  {
	  count.push_back(0);
  }
  
  for(int i = 0; i < (int)trapezoids.size(); i++)
  {
	count[trapezoids[i].pipeIndex]++;
  }
  
  int max = 0;
  for(int i = 0; i < (int)count.size(); i++)
  {
	cout << "Child process " << i << " computed " << count[i] << " trapezoids." << endl;
	if(count[max] < count[i])
	{
		max = i;
	}
  }
  
  cout << "The process that did the most work was child " << max 
	   << " with " << count[max] << endl;
  

  return 0;
}

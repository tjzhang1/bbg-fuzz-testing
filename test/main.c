#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int factorial(int n);

int main(int n_args, char **args) {
   int val;

   if(n_args != 2)
   {
      printf("Usage is: %s <number>\n", args[0]);
      exit(0);
   }
   if(sscanf(args[1], "%d", &val) != 1)
   {
      printf("Failed to read value <%s>\n", args[1]);
      exit(1);
   }
   if(val < 0)
   {
      printf("Negative factorial not allowed.\n");
      exit(1);
   }
   if(val >= 17)
   {
      printf("Overflow.\n");
      exit(1);
   }
   
   printf("%d! = %d\n", val, factorial(val));
   
   return 0;
}

int factorial(int n) {
   if(n == 1 || n == 0)
      return 1;
   else
      return factorial(n-1)*n;
}

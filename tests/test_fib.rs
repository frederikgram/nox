
include "test";

{
    result: int;

    func int fib(n: int) {

        if(n==1) {
            return n;
        }
        return fib(n-1) + fib(n-2); 

    }

    result = fib(10);
    print(result);
}
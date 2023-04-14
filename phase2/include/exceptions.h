#ifndef EXCEPTIONS_H
#define EXCEPTIONS_H
// Handler for system calls
void syscall_handler(int, void *, void *, void *);

// Handler for exception
void exception_handler();

// Pass Up or Die function, if supportStruct is NULL kill the process, otherwise pass up it
void pass_up_or_die(int exep_code);

#endif /* EXCEPTIONS_H */
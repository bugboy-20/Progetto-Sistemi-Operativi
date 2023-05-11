#ifndef KLOG_H
#define KLOG_H

void klog_print_hex(unsigned int num);

void klog_print(char *str);

#define KLOG_PRETTI_PRINT(text, var) klog_print(text); klog_print_hex(var); klog_print("\n")

#endif // KLOG_H

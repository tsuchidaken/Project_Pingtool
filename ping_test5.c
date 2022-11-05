// Include kernel
#include <linux/kernel.h>
#include <pthread.h>
#include <sys/syscall.h>
#include <unistd.h>

#include <string.h>
#include "stdio.h"
#include "stdlib.h"

// Text segment

// 自己定義的資料結構
typedef struct my_IP {
   char* IP;
   int seq;
} my_IP;

// Test share memory variable  
__thread int thread_i;

// Command of ping
// 無初始值的全域變數
char* command;

// Number of Iteration
// 帶有初始值的全域變數
int size = 5;

// Add Mutex
pthread_mutex_t mutex1 = PTHREAD_MUTEX_INITIALIZER;

// Create Command function
void* create_cl(char* IP);
// Child thread
void* ping_ip(void* IP_data);

// Main thread
int main(int argc, char* argv[]) {
    // stack segment
    // pthread variables
    pthread_t t1, t2;
    // IP1
    my_IP IP_1;
    IP_1.IP = argv[1];
    IP_1.seq = 1;
    // IP2
    my_IP IP_2;
    IP_2.IP = argv[2];
    IP_2.seq = 2;
    // Create child thread
    pthread_create(&t1, NULL, ping_ip, (void*) &IP_1);
    pthread_create(&t2, NULL, ping_ip, (void*) &IP_2);
    // Waits for the thread specified by thread to terminate.
    char *str = "main";
     // dynamically allocated variable(s) in main
    char *heap_str = (char *) malloc(sizeof(char) * 100);
    strncpy(heap_str, str, 5);

    // printf("thread: main\n");
    // printf("The value of thread_i in %s: %d                    (address): %p)\n", str, thread_i, &thread_i);
    // printf("The address of char *str(include local str) in %s    [stack]: %p\n", str, &str);
    // printf("The value of int *heap_str in %s        [heap|shared_memory]: %p\n", heap_str, &heap_str);
    // printf("The address of global variable int size: %p\n\n", &size);
    
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);

    return 0;
}
// Create Command function
void* create_cl(char* IP) {
    // Heap segment
    // Memory allocation
    command = malloc(sizeof(char) * 85);
    // Command of ping combine with IP
    // We need Stdout of delay time to caluate the average of delay time
    snprintf(command, 85, "ping -c 1 %s | awk -F'time=| ms' 'NF==3{print $(NF-1)}' | sort -rn", IP);

    // return command;
}
// Child thread
void* ping_ip(void* arg) {
    // 上鎖
    pthread_mutex_lock(&mutex1);  
    // Store delay time in N array
    int *N = malloc(sizeof(int)*size);
    // Store average delay time from kernel(copy_to_user)
    int result = 0;
    // Store stdout(standard I/O stream)
    FILE* fp;
    // Store converted string 
    char buf[150] = {0};
    // Store integer from atof function(string to integer)
    float ret;

    my_IP *data=(my_IP *)arg;
    thread_i = data->seq;

    // Create command  
    create_cl(data->IP);
    // Start routine
    for (int i = 0; i < size; i++) {
        sleep(1);
        // popen opens a process by creating a pipe, forking, and invoking the shell. 
        // Execute a ping command in practice
        if ((fp = popen(command, "r")) == NULL) {
            // Exception handling
            perror("Fail to popen\n");
            printf("error!\n");
            exit(1);
        }
        // fgets get a string from a stream
        if (fgets(buf, sizeof(buf), fp) == NULL) {
            printf("%s 第%d次延遲時間 : 失敗\n", (char*)data->IP, i + 1);
        } else {
            // Covert the string into integer
            ret = atof(buf);
            printf("%s 第%d次延遲時間 : %.1f ms\n", (char*)data->IP, i + 1, ret);
        }
        N[i] = ret * 10;
    }
    // Close a pipe stream to or from a process(popen)
    pclose(fp);
    // Calculate the average with Syscall 333
    // Copy a block of data into result (user space)
    syscall(333, N, &result);  

    printf("%s 平均延遲時間: %.1f ms\n\n", (char*)data->IP, result / 10.0);
    
    // printf("thread: %s\n", data->IP);
    // printf("The value of thread_i in %s: %d                    (address): %p)\n", data->IP, data->seq, &thread_i);
    // printf("The address of char *str(include local str) in %s    [stack]: %p\n", data->IP, buf);
    // //printf("The address of char *local_str in %s [stack]: %p\n", IP, &local_str);
    // printf("The value of int *heap_str in %s        [heap|shared_memory]: %p\n", data->IP, N);
    // printf("The address of global variable int size: %p\n\n", &size);
    
    // Free the memory
    free(command);  
    // 解鎖
    pthread_mutex_unlock(&mutex1);
    // Finish the child thread
    pthread_exit(NULL);
}


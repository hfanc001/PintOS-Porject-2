#ifndef USERPROG_SYSCALL_H
#define USERPROG_SYSCALL_H

#include "lib/user/syscall.h"

void syscall_init (void);

//terminate PintOS by shutting it off, may lose some information 
void halt (void);

//terminate the user program 
void exit (int status);

//runs the executable that is passed in
pid_t exec (const char *cmd_line);

//wait for the child process to clean it up
int wait (pid_t pid);

//creates a new file
bool create (const char *file, unsigned initial_size);

//deletes the file that is passed in and return the status of deletion
bool remove (const char *file);

//open the passed in file and either return the fd or -1 
int open (const char *file);

//return the file size
int filesize (int fd);

//read the content in the file by putting it into the buffer
//read the content that are passed in from the keyboard 
int read (int fd, void *buffer, unsigned size);

//write to the file that is already opened
int write (int fd, const void *buffer, unsigned size);

//cheange the next byte to be readable or writable
void seek (int fd, unsigned position);

//return the next position that is readable or writable
unsigned tell (int fd);

//exit/terminate the process and close all the fd
void close (int fd);


#endif /* userprog/syscall.h */

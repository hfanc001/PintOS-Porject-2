#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "devices/shutdown.h"
#include "filesys/filesys.h"

static void syscall_handler (struct intr_frame *);

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f UNUSED) 
{
  printf ("system call!\n");
  thread_exit ();
}

//terminate PintOS by shutting it off, may lose some information 
void halt (void)
{
	shutdown_power_off();
}

//terminate the user program 
void exit (int status)
{
	/*
	Terminates the current user program, returning status to the kernel. 
	If the process's parent waits for it (see below), 
	this is the status that will be returned. Conventionally, 
	a status of 0 indicates success and nonzero values indicate errors. 
	*/
	
	//process_exit() to free the resource
	
	
}

//runs the executable that is passed in
pid_t exec (const char *cmd_line)
{
/*
 * Runs the executable whose name is given in cmd_line
 * passing any given arguments, and 
 * returns the new process's program id (pid)
 * 
 * Must return pid -1, which otherwise should not be a valid pid
 * if the program cannot load or run for any reason. 
 * 
 * Thus, the parent process cannot return from the exec 
 * until it knows whether the child process successfully loaded its executable. 
 * 
 * must use appropriate synchronization to ensure this.
*/
}

//wait for the child process to clean it up
int wait (pid_t pid)
{
/*
 */
}

//creates a new file
bool create (const char *file, unsigned initial_size)
{
	//return true if succeed, false otherwise
	return filesys_create (file, initial_size);
}

//deletes the file that is passed in and return the status of deletion
bool remove (const char *file)
{
	//return true if successful, false otherwise
	return filesys_remove (file); 
}

//open the passed in file and either return the fd or -1 
int open (const char *file)
{
/*
 * Opens the file called file. 
 * Returns a nonnegative integer handle called a "file descriptor" (fd)
 * Return -1 if the file could not be opened.
 * 
 * File descriptors numbered 0 and 1 are reserved for the console: 
 * 			fd 0 (STDIN_FILENO) is standard input
 * 			fd 1 (STDOUT_FILENO) is standard output. T
 * 
 * the open system call will never return either of these file descriptors, 
 * which are valid as system call arguments only as explicitly described below.
 * 
 * Each process has an independent set of file descriptors. 
 * File descriptors are not inherited by child processes. 
 * 
 * When a single file is opened more than once
 * whether by a single process or different processes
 * each open returns a new file descriptor. 
 * 
 * Different file descriptors for a single file are closed independently 
 * in separate calls to close and they do not share a file position.
*/

	struct file *file_name = filesys_open (file);
	
	//if fail to open
	if (fd = NULL)
		return -1; 
		
	//if open, return the fd
	
	//fist check the current thread
	//find the next available fd
	//in thread's fd table, give the next available the current file name
	//return the next available fd
}


//return the file size
int filesize (int fd)
{
	//go to current thread's fd table
	//find the fd 
	//return the size of the fd file using file_length (struct file *file) 
}

//read the content in the file by putting it into the buffer
//read the content that are passed in from the keyboard 
int read (int fd, void *buffer, unsigned size)
{
/*
 * Reads size bytes from the file open as fd into buffer. 
 * Returns the number of bytes actually read (0 at end of file)
 * Returns -1 if the file could not be read (due to a condition other than end of file). 
 * 
 * Fd 0 reads from the keyboard using input_getc().	
*/	
	//if fd == 0
	//read from keyboard
	
	//else open the file from fd
	//copy the content into the buffer
	//return the number of bytes opened
}

//write to the file that is already opened
int write (int fd, const void *buffer, unsigned size)
{
/*
 * Writes size bytes from buffer to the open file fd. \
 * Returns the number of bytes actually written, 
 *      which may be less than size if some bytes could not be written.
 * 
 * Writing past end-of-file would normally extend the file, 
 * but file growth is not implemented by the basic file system. 
 * 
 * The expected behavior is to write as many bytes as possible up to end-of-file and 
 * return the actual number written
 * return 0 if no bytes could be written at all
 * 
 * Fd 1 writes to the console. 
 * Your code to write to the console should write all of buffer in one call to putbuf()
 * at least as long as size is not bigger than a few hundred bytes. 
 * (It is reasonable to break up larger buffers.) 
*/

	//if fd == 1
	//write the buffer to the console
	//return
	
	//open the file at the fd table
	//
}

//cheange the next byte to be readable or writable
void seek (int fd, unsigned position)
{
/*
 * Changes the next byte to be read or written in open file fd to position, 
 * expressed in bytes from the beginning of the file. 
 * (Thus, a position of 0 is the file's start.)
 * 
 * A seek past the current end of a file is not an error. 
 * A later read obtains 0 bytes, indicating end of file. 
 * A later write extends the file, filling any unwritten gap with zeros. 
 * (However, in Pintos files have a fixed length until project 4 is complete, 
 * so writes past end of file will return an error.) 
 * 
 * These semantics are implemented in the file system and do not require any special effort in system call implementation.	
*/
}

//return the next position that is readable or writable
unsigned tell (int fd)
{
/*
 * Returns the position of the next byte to be read or written in open file fd
 * expressed in bytes from the beginning of the file.	
*/
}

//exit/terminate the process and close all the fd
void close (int fd)
{
/*
 * Closes file descriptor fd. 
 * Exiting or terminating a process implicitly closes all its open file descriptors, 
 * as if by calling this function for each one	
*/
}




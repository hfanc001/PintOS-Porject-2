#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <stdbool.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "userprog/process.h"
#include "lib/kernel/list.h"

static void syscall_handler (struct intr_frame *);

static bool
cmp_fd (const struct list_elem *a, int fd,
        void *aux UNUSED)
{
  struct pair *x = list_entry(a, struct pair, elem);
  if (x->fd == fd)
    return true;
  else
    return false;
}

void
syscall_init (void) 
{
  intr_register_int (0x30, 3, INTR_ON, syscall_handler, "syscall");
}

static void
syscall_handler (struct intr_frame *f) 
{
  printf ("system call!\n");
  thread_exit ();
}

void
halt (void)
{
  shutdown_power_off ();
}

void
exit (int status)
{
  struct thread *t = thread_current ();
  thread_exit();
}

pid_t
exec (const char *cmd_line)
{
  return process_execute (cmd_line);
}

int
wait (pid_t pid)
{
  return process_wait (pid);
}

bool
create (const char *file, unsigned initial_size)
{
  return filesys_create (file, initial_size);
}

bool
remove (const char *file)
{
  if (file == NULL)			/* need to account for removing */
    return false;			/* opened files. */
  return filesys_remove (file);
}

int
open (const char *file)
{
  struct pair p;
  p.file = filesys_open (file);
    if (p.file == NULL)
      return -1;
  struct thread *t = thread_current ();
  p.fd = t->new_fd;
  ++t->new_fd;
  list_push_back (&t->file_list, &p.elem);
  
  return p.fd;
}

int
filesize (int fd)
{
  struct thread *t = thread_current ();
  struct pair *p = list_find (&t->file_list, &cmp_fd, fd, NULL);
  if (p == NULL)
    return -1;
  return file_length (p->file);
}

int
read (int fd, void *buffer, unsigned size)
{
  return -1;
}

int
write (int fd, const void *buffer, unsigned size)
{
  return -1;
}

void
seek (int fd, unsigned position)
{
  return;
}

unsigned
tell (int fd)
{
  return 0;
}

void
close (int fd)
{
  return;
}




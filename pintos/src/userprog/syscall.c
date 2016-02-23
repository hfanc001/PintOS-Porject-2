#include "userprog/syscall.h"
#include <stdio.h>
#include <syscall-nr.h>
#include <stdbool.h>
#include "threads/interrupt.h"
#include "threads/thread.h"
#include "threads/synch.h"
#include "userprog/process.h"
#include "lib/kernel/list.h"
#include "devices/input.h"
#include "filesys/file.h"
#include "filesys/inode.h"
#include "filesys/filesys.h"
#include "devices/shutdown.h"

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
  return;
}

void
get_arguments (struct intr_frame *f, int argc, int argv[3])
{
    int *curr = NULL;
    int i;
    for (i = 0; i < argc ; ++i)
      {
        curr = (int *)(f->esp) + i + 1;
        argv[i] = *curr;
      }
}

static void
syscall_handler (struct intr_frame *f) 
{
  int argv[3];
  int c = *((int *)(f->esp));
  switch (c)
    {
      case SYS_HALT:
        halt ();
        break;
      
      case SYS_EXIT:
        get_arguments (f, 1, &argv[0]);
        exit (argv[0]);
        break;

      case SYS_EXEC:
        get_arguments (f, 1, &argv[0]);
        exec ((const char*)argv[0]);
        break;

      case SYS_WAIT:
        get_arguments (f, 1, &argv[0]);
        wait (argv[0]);
        break;

      case SYS_CREATE:
        get_arguments (f, 2, &argv[0]);
        create ((const char *)argv[0], (unsigned)argv[1]);
        break;

      case SYS_REMOVE:
        get_arguments (f, 1, &argv[0]);
        remove ((const char *)argv[0]);
        break;

      case SYS_OPEN:
        get_arguments (f, 1, &argv[0]);
        open ((const char *)argv[0]);
        break;

      case SYS_FILESIZE:
        get_arguments (f, 2, &argv[0]);
        filesize ((int)argv[0]);
        break;

      case SYS_READ:
        get_arguments (f, 3, &argv[0]);
        read ((int)argv[0], (void *)argv[1], (unsigned)argv[2]);
        break;

      case SYS_WRITE:
        get_arguments (f, 3, &argv[0]);
        write ((int)argv[0], (void *)argv[1], (unsigned)argv[2]);
        break;

      case SYS_SEEK:
        get_arguments (f, 2, &argv[0]);
        seek ((int)argv[0], (unsigned)argv[1]);
        break;

      case SYS_TELL:
        get_arguments (f, 1, &argv[0]);
        tell ((int)argv[0]);
        break;

      case SYS_CLOSE:
        get_arguments (f, 1, &argv[0]);
        close ((int)argv[0]);
        break;
    
      default:
        printf ("ERROR: syscall not found\n");
        thread_exit ();
        break;
    }
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
  remove_as_parent (t);
  remove_as_child (t);
  printf ("%s: exit(%d)\n", t->name, status);
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
  struct list_elem *e = list_find (&t->file_list, &cmp_fd, fd, NULL);
  struct pair *p = list_entry (e, struct pair, elem);
  if (p == NULL)
    return -1;
  return file_length (p->file);
}

int
read (int fd, void *buffer, unsigned size)
{
  if (fd == 1)
    return -1;
  if (fd == 0)
    return input_getc ();
    
  struct thread *t = thread_current ();
  struct list_elem *e = list_find (&t->file_list, &cmp_fd, fd, NULL);
  struct pair *p = list_entry (e, struct pair, elem);
  if (p == NULL)
    return -1;
  return file_read (p->file, buffer, size);
}

int
write (int fd, const void *buffer, unsigned size)
{
  if (fd == 0)
    return -1;
  /*need to segment buffer if it is too big */
  if (fd == 1)
    {
      putbuf (buffer, size); 	
      return size;
    }
    
  struct thread *t = thread_current ();
  struct list_elem *e = list_find (&t->file_list, &cmp_fd, fd, NULL);
  struct pair *p = list_entry (e, struct pair, elem);
  if (p == NULL)
    return -1;
  return file_write (p->file, buffer, size);
}

void
seek (int fd, unsigned position)
{
  if (fd == 0 || fd == 1)
    return;
  struct thread *t = thread_current ();
  struct list_elem *e = list_find (&t->file_list, &cmp_fd, fd, NULL);
  struct pair *p = list_entry (e, struct pair, elem);
  if (p == NULL)
    return;
  file_seek (p->file, position);
  return;
}

unsigned
tell (int fd)
{
  if (fd == 0 || fd == 1)
    return -1;
  struct thread *t = thread_current ();
  struct list_elem *e = list_find (&t->file_list, &cmp_fd, fd, NULL);
  struct pair *p = list_entry (e, struct pair, elem);
  if (p == NULL)
    return -1;
  return file_tell (p->file);

}

void
close (int fd)
{
  if (fd == 0 || fd == 1)
    return;
  struct thread *t = thread_current ();
  struct list_elem *e = list_find (&t->file_list, &cmp_fd, fd, NULL);
  struct pair *p = list_entry (e, struct pair, elem);
  if (p == NULL)
    return;

  list_remove (&p->elem);

  if (p->file->inode->open_cnt == 1)
    file_close (p->file);
  
  return;
}

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
#include "threads/vaddr.h"
#include "filesys/directory.h"

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

static inline bool
get_user (uint8_t *dst, const uint8_t *usrc)
{
  int eax;
  asm ("movl $1f, %%eax; movb %2, %%al; movb %%al, %0; 1:"
       : "=m" (*dst), "=&a" (eax) : "m" (*usrc));
  return eax != 0;
}

static void
copy_in (void *dst_, const void *usrc_ ,size_t size)
{
  uint8_t *dst = dst_;
  const uint8_t *usrc = usrc_;

  for (; size > 0; size--, dst++, usrc++)
    if (usrc >= (uint8_t *) PHYS_BASE || !get_user (dst, usrc))
      thread_exit ();
}

static void
syscall_handler (struct intr_frame *f) 
{
  int argv[3];
  int argc;
  unsigned c; 
  
  copy_in (&c, f->esp, sizeof c);

  switch (c)
    {
      case SYS_HALT:
        halt ();
        break;
      
      case SYS_EXIT:
        argc = 1;
        copy_in (argv, (uint32_t *) f->esp + 1, sizeof *argv * argc);
        exit (argv[0]);
        break;

      case SYS_EXEC:
        argc = 1;
        copy_in (argv, (uint32_t *) f->esp + 1, sizeof *argv * argc);
        f->eax = exec ((const char*)argv[0]);
        break;

      case SYS_WAIT:
        argc = 1;
        copy_in (argv, (uint32_t *) f->esp + 1, sizeof *argv * argc);
        f->eax = wait (argv[0]);
        break;

      case SYS_CREATE:
        argc = 2;
        copy_in (argv, (uint32_t *) f->esp + 1, sizeof *argv * argc);
        f->eax = create ((const char *)argv[0], (unsigned)argv[1]);
        break;

      case SYS_REMOVE:
        argc = 1;
        copy_in (argv, (uint32_t *) f->esp + 1, sizeof *argv * argc);
        f->eax = remove ((const char *)argv[0]);
        break;

      case SYS_OPEN:
        argc = 1;
        copy_in (argv, (uint32_t *) f->esp + 1, sizeof *argv * argc);
        f->eax = open ((const char *)argv[0]);
        break;

      case SYS_FILESIZE:
        argc = 1;
        copy_in (argv, (uint32_t *) f->esp + 1, sizeof *argv * argc);
        f->eax = filesize (argv[0]);
        break;

      case SYS_READ:
        argc = 3;
        copy_in (argv, (uint32_t *) f->esp + 1, sizeof *argv * argc);
        f->eax = read (argv[0], (void *)argv[1], (unsigned)argv[2]);
        break;

      case SYS_WRITE:
        argc = 3;
        copy_in (argv, (uint32_t *) f->esp + 1, sizeof *argv * argc);
        f->eax = write (argv[0], (void *)argv[1], (unsigned)argv[2]);
        break;

      case SYS_SEEK:
        argc = 2;
        copy_in (argv, (uint32_t *) f->esp + 1, sizeof *argv * argc);
        seek (argv[0], (unsigned)argv[1]);
        break;

      case SYS_TELL:
        argc = 1;
        copy_in (argv, (uint32_t *) f->esp + 1, sizeof *argv * argc);
        f->eax = tell (argv[0]);
        break;

      case SYS_CLOSE:
        argc = 1;
        copy_in (argv, (uint32_t *) f->esp + 1, sizeof *argv * argc);
        close (argv[0]);
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
  if (file == NULL)
    exit (-1);
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

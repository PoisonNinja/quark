#pragma once

#include <types.h>

namespace filesystem
{
struct stat {
    dev_t st_dev;     /* inode's device */
    ino_t st_ino;     /* inode's number */
    mode_t st_mode;   /* inode protection mode */
    nlink_t st_nlink; /* number of hard links */
    uid_t st_uid;     /* user ID of the file's owner */
    gid_t st_gid;     /* group ID of the file's group */
    dev_t st_rdev;    /* device type */
    off_t st_size;    /* file size, in bytes */
    time_t st_atime;  /* time of last access */
    time_t st_mtime;  /* time of last data modification */
    time_t st_ctime;  /* time of last file status change */
    long st_blksize;  /* optimal blocksize for I/O */
    long st_blocks;   /* blocks allocated for file */
};

#define S_ISUID 0004000 /* set user id on execution */
#define S_ISGID 0002000 /* set group id on execution */
#define S_ISTXT 0001000 /* sticky bit */

#define S_IRWXU 0000700 /* RWX mask for owner */
#define S_IRUSR 0000400 /* R for owner */
#define S_IWUSR 0000200 /* W for owner */
#define S_IXUSR 0000100 /* X for owner */

#define S_IREAD S_IRUSR
#define S_IWRITE S_IWUSR
#define S_IEXEC S_IXUSR

#define S_IRWXG 0000070 /* RWX mask for group */
#define S_IRGRP 0000040 /* R for group */
#define S_IWGRP 0000020 /* W for group */
#define S_IXGRP 0000010 /* X for group */

#define S_IRWXO 0000007 /* RWX mask for other */
#define S_IROTH 0000004 /* R for other */
#define S_IWOTH 0000002 /* W for other */
#define S_IXOTH 0000001 /* X for other */

#define S_IFMT 0170000   /* type of file */
#define S_IFIFO 0010000  /* named pipe (fifo) */
#define S_IFCHR 0020000  /* character special */
#define S_IFDIR 0040000  /* directory */
#define S_IFBLK 0060000  /* block special */
#define S_IFREG 0100000  /* regular */
#define S_IFLNK 0120000  /* symbolic link */
#define S_IFSOCK 0140000 /* socket */

#define S_ISVTX 0001000 /* save swapped text even after use */

#define S_BLKSIZE 512 /* block size used in the stat struct */

/* 0666 */
#define DEFFILEMODE (S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH)

#define S_ISDIR(m) ((m & 0170000) == 0040000)  /* directory */
#define S_ISCHR(m) ((m & 0170000) == 0020000)  /* char special */
#define S_ISBLK(m) ((m & 0170000) == 0060000)  /* block special */
#define S_ISREG(m) ((m & 0170000) == 0100000)  /* regular file */
#define S_ISFIFO(m) ((m & 0170000) == 0010000) /* fifo */
#define S_ISLNK(m) ((m & 0170000) == 0120000)  /* symbolic link */
#define S_ISSOCK(m) ((m & 0170000) == 0140000) /* socket */

} // namespace filesystem

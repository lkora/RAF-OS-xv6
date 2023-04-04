#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/fcntl.h"

int
main(int argc, char *argv[])
{
  int fd, i;
  char buf[512];

  // Open the file for writing
  fd = open("testfile", O_CREATE | O_RDWR);
  if(fd < 0){
    printf(2, "error: cannot open testfile for writing\n");
    exit();
  }

  // Write 16523 blocks of data to the file
  for(i = 0; i < 16523; i++){
    printf("Writing block %d\n", i);
    memset(buf, 0, sizeof(buf));
    write(fd, buf, sizeof(buf));
  }

  // Close the file
  close(fd);

  exit();
}
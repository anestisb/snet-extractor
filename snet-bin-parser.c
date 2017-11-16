/*

   snet-extractor
   -----------------------------------------

   Anestis Bechtsoudis <anestis@census-labs.com>
   Copyright 2017 by CENSUS S.A. All Rights Reserved.

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

     http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

*/

#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <errno.h>
#include <inttypes.h>
#include <unistd.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <stdbool.h>
#include <limits.h>

static char *kMetadataFile = "metadata_flags.txt";
static char *kPayloadFile = "payload.snet";

static uint8_t *mapFile(char *fileName, off_t *fileSz, int *fd, bool enableWrite) {
  if ((*fd = open(fileName, enableWrite ? O_RDWR : O_RDONLY)) == -1) {
    printf("[-] Couldn't open() '%s' file in R/O mode (%s)", fileName, strerror(errno));
    return NULL;
  }

  struct stat st;
  if (fstat(*fd, &st) == -1) {
    printf("[-] Couldn't stat() the '%s' file (%s)", fileName, strerror(errno));
    close(*fd);
    return NULL;
  }

  uint8_t *buf;
  int flags = PROT_READ;
  if (enableWrite) {
    flags |= PROT_WRITE;
  }
  if ((buf = mmap(NULL, st.st_size, flags, MAP_PRIVATE, *fd, 0)) == MAP_FAILED) {
    printf("[-] Couldn't mmap() the '%s' file (%s)\n", fileName, strerror(errno));
    close(*fd);
    return NULL;
  }

  *fileSz = st.st_size;
  return buf;
}

static bool writeToFd(int fd, const uint8_t * buf, size_t fileSz) {
  size_t writtenSz = 0;
  while (writtenSz < fileSz) {
    ssize_t sz = write(fd, &buf[writtenSz], fileSz - writtenSz);
    if (sz < 0 && errno == EINTR)
      continue;

    if (sz < 0)
      return false;

    writtenSz += sz;
  }
  return true;
}

static bool writeBufToFile(const char *fileName, const uint8_t * buf, size_t fileSz) {
  int fd = open(fileName, O_CREAT | O_WRONLY, 0644);
  if (fd == -1) {
    printf("[-] Couldn't open '%s' for write (%s)\n", fileName, strerror(errno));
    return false;
  }

  if (writeToFd(fd, buf, fileSz) == false) {
    printf("[-] Couldn't write '%zu' bytes to file '%s' (fd='%d') (%s)\n",
           fileSz, fileName, fd, strerror(errno));
    unlink(fileName);
    close(fd);
    return false;
  }

  return true;
}

uint32_t read4Bytes(uint8_t *cursor) {
  uint32_t x;
  x = 0xFF & cursor[3];
  x |= (0xFF & cursor[2]) << 8;
  x |= (0xFF & cursor[1]) << 16;
  x |= (uint32_t)(0xFF & cursor[0]) << 24;
  return x;
}

int main(int argc, char **argv) {
  int ret = EXIT_FAILURE;

  if (argc != 3) {
    printf("%s <input_snet.flags> <output_dir>\n", argv[0]);
    return ret;
  }

  off_t fileSz = 0;
  int srcfd = -1;
  uint8_t *buf = NULL;

  buf = mapFile(argv[1], &fileSz, &srcfd, false);
  if (buf == NULL) {
    printf("[-] Open & map failed\n");
    return ret;
  }

  uint32_t sigBlobSize = read4Bytes(buf);

  char outputFile[PATH_MAX] = { 0 };
  snprintf(outputFile, sizeof(outputFile), "%s/%s", argv[2], kMetadataFile);
  if (!writeBufToFile(outputFile, buf + 4, sigBlobSize)) {
    printf("[-] Failed to write snet metadata file");
    goto fini;
  }
  printf("[*] Successfully extracted '%s'\n", kMetadataFile);

  memset(outputFile, 0, sizeof(outputFile));
  snprintf(outputFile, sizeof(outputFile), "%s/%s", argv[2], kPayloadFile);
  if (!writeBufToFile(outputFile, buf + 4 + sigBlobSize, fileSz - 4 - sigBlobSize)) {
    printf("[-] Failed to write snet flags file");
    goto fini;
  }
  printf("[*] Successfully extracted '%s'\n", kPayloadFile);

  ret = EXIT_SUCCESS;

fini:
  close(srcfd);

  return ret;
}

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>

void print_conform(int S[], int hit[], int seeksize, struct timeval begin, struct timeval finish){
  int i = 0;
  while( i < seeksize){
    char *yes_no = "No";
    yes_no = (hit[i])?"Yes":"No";
    printf( "%12d: %s\n", S[i], yes_no);
    i++;
  }
  finish.tv_sec -= begin.tv_sec;
  finish.tv_usec -= begin.tv_usec;
  if(finish.tv_usec < 0){
    finish.tv_sec--;
    finish.tv_usec += 1000000;
  }
  printf("Time: %ld.%.06ld\n", finish.tv_sec, finish.tv_usec);
}

int file_size(FILE *input_file){
  fseek(input_file, 0, SEEK_END);
  int filesize = ftell(input_file)/sizeof(int);
  fseek(input_file, 0, SEEK_SET);
  return filesize;
}

void mem_lin( FILE *seekfile, int seeksize, int S[], int hit[], FILE *keyfile, int keysize, int K[]){
  for(int i = 0; i < seeksize; i++){
    for(int j = 0; j < keysize; j++){
      if(S[i] == K[j]){
        hit[i] = 1;
        break;
      }
    }
  }
}

void mem_bin( FILE *seekfile, int seeksize, int S[], int hit[], FILE *keyfile, int keysize, int K[]){
  for(int i = 0; i < seeksize; i++){
    int val = S[i];
    int l = 0;
    int r = keysize - 1;
    int mid;
    while(l<=r){
      mid = l + (r-l)/2;
      if(K[mid] == val){
        hit[i] = 1;
        break;
      }
      else if(K[mid]> val){
        r = mid - 1;
      }
      else{
        l = mid + 1;
      }
    }
  }
}

void disk_lin( FILE *seekfile, int seeksize, int S[], int hit[], FILE *keyfile, int keysize){
  int current;
  for(int i = 0; i < seeksize; i++){
    int val = S[i];
    for(int j = 0; j < keysize; j++){
      fread(&current, sizeof(int), 1, keyfile);
      if(current == val){
        hit[i] = 1;
        break;
      }
    }
    fseek(keyfile, 0, SEEK_SET);
  }
}

void disk_bin( FILE *seekfile, int seeksize, int S[], int hit[], FILE *keyfile, int keysize){
  int current;
  for(int i = 0; i < seeksize; i++){
    int val = S[i];
    int l = 0;
    int r = keysize - 1;
    int mid;
    while(l<=r){
      mid = l + (r-l)/2;
      fseek(keyfile, mid*sizeof(int), SEEK_SET);
      fread(&current, sizeof(int), 1, keyfile);
      if( val == current){
        hit[i] = 1;
        break;
      }
      else if(val < current){
        r = mid - 1;
      }
      else{
        l = mid + 1;
      }
    }
    fseek(keyfile, 0, SEEK_SET);
  }
}

int main(int argc, char *argv[]){
  if(argc != 4){
    printf("Arguments not provided or incorrect \n");
    exit(1);
  }
  char *search = argv[1];
  char *key = argv[2];
  char *seek = argv[3];

  struct timeval begin;
  struct timeval finish;

  FILE *seekfile = fopen(seek, "rb");
  int seeksize = file_size(seekfile);

  int S[seeksize];
  int hit[seeksize];

  for(int i = 0; i < seeksize; i++){
    fseek(seekfile, i*sizeof(int), SEEK_SET);
    fread(&S[i], sizeof(int), 1, seekfile);
  }

  if(!strcmp(search, "--mem-lin")){
    gettimeofday(&begin, NULL);
    FILE *keyfile = fopen(key, "rb");
    int keysize = file_size(keyfile);
    int K[keysize];
    for(int i = 0; i < keysize; i++){
      fseek(keyfile, i*sizeof(int), SEEK_SET);
      fread(&K[i], sizeof(int), 1, keyfile);
    }
    mem_lin(seekfile, seeksize, S, hit, keyfile, keysize, K);
    gettimeofday(&finish, NULL);
    print_conform(S, hit, seeksize, begin, finish);
    fclose(keyfile);
  }

  else if(!strcmp(search, "--mem-bin")){
    gettimeofday(&begin, NULL);
    FILE *keyfile = fopen(key, "rb");
    int keysize = file_size(keyfile);
    int K[keysize];
    for(int i = 0; i < keysize; i++){
      fseek(keyfile, i*sizeof(int), SEEK_SET);
      fread(&K[i], sizeof(int), 1, keyfile);
    }
    mem_bin(seekfile, seeksize, S, hit, keyfile, keysize, K);
    gettimeofday(&finish, NULL);
    print_conform(S, hit, seeksize, begin, finish);
    fclose(keyfile);
  }

  else if(!strcmp(search, "--disk-lin")){
    gettimeofday(&begin, NULL);
    FILE *keyfile = fopen(key, "rb");
    int keysize = file_size(keyfile);
    disk_lin(seekfile, seeksize, S, hit, keyfile, keysize);
    gettimeofday(&finish, NULL);
    print_conform(S, hit, seeksize, begin, finish);
    fclose(keyfile);
  }
  else if(!strcmp(search, "--disk-bin")){
    gettimeofday(&begin, NULL);
    FILE *keyfile = fopen(key, "rb");
    int keysize = file_size(keyfile);
    disk_bin(seekfile, seeksize, S, hit, keyfile, keysize);
    gettimeofday(&finish, NULL);
    print_conform(S, hit, seeksize, begin, finish);
    fclose(keyfile);
  }

  else{
    fclose(seekfile);
    printf("Wrong Arguments\n");
    exit(1);
  }
  fclose(seekfile);

  return(0);
}

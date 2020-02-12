#include <stdio.h>
#include <stdlib.h>

typedef struct{
  int key;
  long offset;
} index_S;

typedef struct{
  int size;
  long offset;
} avail_S;


avail_S* readAvailFile(){
  FILE *availFile;
  availFile = fopen("avail.bin", "rb");
  fseek(availFile, 0, SEEK_END);
  long len = ftell(availFile)/sizeof(avail_S);
  fseek(availFile, 0, SEEK_SET);
  index_S *retArray = malloc((len + 1)*sizeof(avail_S));
  fread(retArray, sizeof(avail_S), len, availFile);
  fclose(availFile);
  return retArray;
}

long lenOfAvail(){
  FILE *availFile;
  availFile = fopen("avail.bin", "rb");
  fseek(availFile, 0, SEEK_END);
  long len = ftell(availFile)/sizeof(avail_S);
  fclose(availFile);
  return len;
}

index_S* readIndexFile(){
  FILE *indexFile;
  indexFile = fopen("index.bin", "rb");
  fseek(indexFile, 0, SEEK_END);
  long len = ftell(indexFile)/sizeof(index_S);
  fseek(indexFile, 0, SEEK_SET);
  index_S *retArray = malloc((len + 1)*sizeof(index_S));
  fread(retArray, sizeof(index_S), len, indexFile);
  fclose(indexFile);
  return retArray;
}

long lenOfIndex(){
  FILE *indexFile;
  indexFile = fopen("index.bin", "rb");
  fseek(indexFile, 0, SEEK_END);
  long len = ftell(indexFile)/sizeof(index_S);
  fclose(indexFile);
  return len;
}

int main(int argc, char *argv[]){

  int list_order = 1;
  char *list_order = argv[1];
  char *filename = argv[2];

  index_S *pKeyList = malloc(sizeof(index_S));
  avail_S *aList = malloc(sizeof(avail_S));
  FILE *file;
  long countAvail = 0;
  long countIndex = 0;

  if(( file = fopen(filename, "r+b")) == NULL){
    file = fopen(filename, "w+b");fclose(file);
    file = fopen(filename, "r+b");
  }
  else{
    aList = readAvailFile();
    countAvail = lenOfAvail();
    pKeyList = readIndexFile();
    countIndex = lenOfIndex();

  }
  if(argc != 3){
    printf("Arguments not provided or incorrect \n FORMAT: assn_2 avail-list-order studentfile-name\n");
    exit(1);
  }

  if(!strcmp(search, "--first-fit")){
    list_order = 1;
  }

  else if(!strcmp(search, "--best-fit")){
    list_order = 2;
  }

  else if(!strcmp(search, "--worst-fit")){
    list_order = 3;
  }
  else{
    printf("Wrong Arguments\n");
    exit(1);
  }

  return(0);
}

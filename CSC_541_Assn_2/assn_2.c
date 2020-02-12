#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct{
  int key;
  long offset;
} index_S;

typedef struct{
  int siz;
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

void writeAndCloseAvailFile(index_S *aList, int countAvail){
  FILE *availFile;
  availFile = fopen("avail.bin", "r+b");
  fseek(availFile, 0, SEEK_SET);
  fwrite(aList, sizeof(avail_S), countAvail, availFile);
  fclose(availFile);
}

index_S* readIndexFile(){

  FILE *indexFile;
  indexFile = fopen("index.bin", "rb");
  fseek(indexFile, 0, SEEK_END);
  long len = ftell(indexFile)/sizeof(index_S);
  fseek(indexFile, 0, SEEK_SET);
  index_S *retArray = malloc((len + 1)*sizeof(index_S));
  fread(&retArray, sizeof(index_S), len, indexFile);
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

void writeAndCloseIndexFile(index_S *pKeyList, int countIndex){
  FILE *indexFile;
  indexFile = fopen("index.bin", "r+b");
  fseek(indexFile, 0, SEEK_SET);
  fwrite(pKeyList, sizeof(index_S), countIndex, indexFile);
  fclose(indexFile);
}

char** breakInstr(char *inp){

  char **instr = malloc(3 * sizeof(char *));
  instr[0] = strtok(inp, " ");
  instr[1] = strtok(NULL, " ");
  instr[2] = strtok(NULL, " ");

  return instr;
}

int binary_search_key(int key, int low, int high, index_S pKeyList[]){
  int mid;
  while(low <= high){
    mid = low + ((high - low)/2);
    if(pKeyList[mid].key == key){
      return mid;
    }
    else if(pKeyList[mid].key > key){
      high = mid - 1;
    }
    else{
      low = mid + 1;
    }
  }
  return -1;
}

int availHole(int len, int countAvail, avail_S aList[]){
  int i = 0;
  while(i<countAvail){
    if(aList[i].siz >= (sizeof(int) + len)){
      return i;
    }
    i++;
  }
  return -1;
}

void closeHole(int index, int countAvail, avail_S aList[]){
  if(index >= countAvail){
    return;
  }
  else {
    int i = index;
    while(i < countAvail - 1){
      aList[i] = aList[i+1];
      i++;
    }
    countAvail = countAvail - 1;
  }
}

void addHole(avail_S avail, int countAvail, avail_S aList[], int list_order){
  if(list_order == 1){
    alist[countAvail] = avail;
    countAvail = countAvail + 1;
  }
  if(list_order == 2){
    alist[countAvail] = avail;
    countAvail = countAvail + 1;
    qsort(&aList, &countAvail, sizeof(avail_S), bestFit);
  }
  if(list_order == 3){
    alist[countAvail] = avail;
    countAvail = countAvail + 1;
    qsort(&aList, &countAvail, sizeof(avail_S), worstFit);
  }
}

int bestFit( const void *a, const void *b){
  if((((avail_S*)a)->siz - ((avail_S*)b)->siz) == 0){
    return (((avail_S*)a)->offset - ((avail_S*)b)->offset);
  }
  return (((avail_S*)a)->siz - ((avail_S*)b)->siz);
}

int worstFit( const void *a, const void *b){
  if(!((avail_S*)b)->siz - ((avail_S*)a)->siz){
    return (((avail_S*)a)->offset - ((avail_S*)b)->offset);
  }
  return (((avail_S*)b)->siz - ((avail_S*)a)->siz);
}

void removeIndex(int key, index_S pKeyList[], int countIndex){
  if(key >= countIndex){
    return;
  }
  else{
    int i = search;
    while(i < countIndex - 1){
      pKeyList[i] = pKeyList[i+1];
    }
    countIndex = countIndex - 1;
  }
}

void addToFile(FILE *file, int key, size_t len, char *rec, avail_S aList[], int countAvail, index_S pKeyList[], int countIndex, int list_order){
  if(binary_search_key(key, 0, countIndex - 1, pKeyList) == -1){
    printf("Record with SID=%d exists\n", key);
    return;
  }
  long offset;
  int indexAvail = holeAvail(len, countAvail, aList);

  if(indexAvail != -1){
    fseek(file, aList[indexAvail].offset, SEEK_SET);
    fwrite(len, sizeof(int),1, file);
    fwrite(rec, sizeof(char), len, file);

    int gapFilled = (sizeof(int) + len);
    int newHoleSize = aList[indexAvail].siz - gapFilled;
    int newHoleOffset = aList[indexAvail].offset + gapFilled;

    closeHole(indexAvail, &countAvail, &aList);
    if(newHoleSize > 0){
      avail_S avail = {newHoleSize, newHoleOffset};
      addHole(avail, &countAvail, &aList, list_order);
    }
  }
  else{
    fseek(file, 0, SEEK_END);
    offset = ftell(file);
    fwrite(len, sizeof(int),1, file);
    fwrite(rec, sizeof(char), len, file);
  }

  index_S index = {key, offset};
  if(countIndex != 0){
    int low = 0;
    int high = countIndex -1;
    int mid;
    while(low <= high){
      mid = low + ((high-low)/2);
      if(pKeyList[mid].key > key){
        high = mid - 1;
      }
      else{
        low = mid + 1;
      }
    }
    int i = countIndex;
    countIndex = countIndex + 1;
    while(i > low){
      pKeyList[i] = pKeyList[i-1];
      i--;
    }
    pKeyList[low] = index;
  }
  else{
    pKeyList[countIndex] = index;
    countIndex = countIndex + 1;
  }
}

void findRecordInFile(FILE *file, int key, avail_S aList[], int countAvail, index_S pKeyList[], int countIndex){
  int search = binary_search_key(key, 0, countIndex - 1 , pKeyList);
  if(search != -1){
    fseek(file, pKeyList[search].offset, SEEK_SET);
    int recordSize;
    fread(&recordSize, sizeof(int), 1, file);
    char *record = malloc(recordSize);
    fread(record,1, recordSize, file);
    printf("%s\n", record);
  }
  else{
    printf("No record with SID=%d exists \n", key);
  }
}


void deleteRecordFromFile(FILE *file, int key, avail_S aList[], int countAvail, index_S pKeyList[], int countIndex, int list_order){
  int search = int search = binary_search_key(key, 0, countIndex - 1, pKeyList);
  if( search != -1){
    avail_S avail;
    avail.offset = pKeyList[search].offset;
    fseek(file, avail.offset, SEEK_SET);
    int recordSize;
    fread(&recordSize, sizeof(int), 1, file);
    avail.siz = recordSize + sizeof(int);
    addHole(avail,&countAvail, &aList, list_order);
    removeIndex(search, &pKeyList, &countIndex);
  }
  else{
    printf("No record with SID=%d exists\n", key);
  }
}

int main(int argc, char *argv[]){

  char *input;
  size_t lenOfInput, sizeOfInput = 1024;

  int list_order = 1;
  char *list_order = argv[1];
  char *filename = argv[2];

  index_S pKeyList[10000];
  avail_S aList[10000];
  FILE *file;
  int countAvail = 0;
  int countIndex = 0;

  if(argc != 3){
    printf("Arguments not provided or incorrect \n FORMAT: assn_2 avail-list-order studentfile-name\n");
    exit(1);
  }

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

  while(true){
    char *input = (char *)malloc(sizeOfInput*sizeof(char));
    lenOfInput = getline(&input, &sizeOfInput, stdin);
    if(lenOfInput == -1){
      continue;
    }

    char **instr = breakInstr(input);

    if(!strcmp(instr[0], "add")){
      int key = atoi(instr[1]);
      size_t len = strlen(instr[2]);
      char *rec = malloc((int) len);
      strcpy(rec, instr[2]);
      addToFile(file, key, len, rec, &aList, &countAvail, &pKeyList, &countIndex, list_order);
    }

    else if(!strcmp(instr[0], "find")){
      int key = atoi(instr[1]);
      findRecordInFile(file, key, &aList, &countAvail, &pKeyList, &countIndex);
    }

    else if(!strcmp(instr[0], "del")){
      int key = atoi(instr[1]);
      deleteRecordFromFile(file, key,&aList, &countAvail, &pKeyList, &countIndex, list_order);
    }
    else if(!strcmp(instr[0], "end")){
      fclose(file);
      writeAndCloseIndexFile(&pKeyList, &countIndex);
      writeAndCloseAvailFile(&aList, &countAvail);
      break;
    }
  }
  printf("Index:\n");
  int i = 0;
  while(i<countIndex){
    printf("key=%d: offset=%ld\n", pKeyList[i].key, pKeyList[i].offset);
    i++;
  }
  int totalHoleSize = 0;
  i = 0;
  while(i < countAvail){
    printf( "size=%d: offset=%ld\n", aList[i].siz, aList[i].offset);
    totalHoleSize = totalHoleSize + alist[i].siz;
  }
  printf( "Number of holes: %d\n", countAvail);
  printf( "Hole space: %d\n", totalHoleSize);
  return(0);
}

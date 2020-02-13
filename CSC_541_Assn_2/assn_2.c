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

size_t lenOfInput, sizeOfInput = 1024;
char *input;

avail_S aList[10000];
index_S pKeyList[10000];

int countAvail = 0;
int countIndex = 0;

FILE *indexFile, *availFile;

void readAvailFile(){

  avail_S temp;
  availFile = fopen("avail.bin", "r+b");
  fseek(availFile, 0, SEEK_END);
  int len = ftell(availFile)/sizeof(avail_S);
  fseek(availFile, 0, SEEK_SET);
  fread(&aList, sizeof(avail_S), len, availFile);
  countAvail = len;
}

/**long lenOfAvail(){
  fseek(availFile, 0, SEEK_END);
  long len = ftell(availFile)/sizeof(avail_S);
  countAvail = len;
}**/

void writeAndCloseAvailFile(){
  fseek(availFile, 0, SEEK_SET);
  fwrite(aList, sizeof(avail_S), countAvail, availFile);
  fclose(availFile);
}

void readIndexFile(){

  index_S temp;
  indexFile = fopen("index.bin", "r+b");
  fseek(indexFile, 0, SEEK_END);
  int len = ftell(indexFile)/sizeof(index_S);
  fseek(indexFile, 0, SEEK_SET);
  fread(&pKeyList, sizeof(index_S), len, indexFile);
  countIndex = len;
}

/**void lenOfIndex(){
  fseek(indexFile, 0, SEEK_END);
  long len = ftell(indexFile)/sizeof(index_S);
  countIndex = len;
}**/

void writeAndCloseIndexFile(){
  fseek(indexFile, 0, SEEK_SET);
  fwrite(pKeyList, sizeof(index_S), countIndex, indexFile);
  fclose(indexFile);
}

char** breakInstr(char *inp){

  char **instr = malloc(3 * sizeof(char *));
  instr[0] = strtok(inp, " \n");
  instr[1] = strtok(NULL, " \n");
  instr[2] = strtok(NULL, " \n");

  return instr;
}

int binary_search_key(int key, int low, int high){
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

int availHole(int len){
  int i = 0;
  while(i<countAvail){
    if(aList[i].siz >= (sizeof(int) + len)){
      return i;
    }
    i++;
  }
  return -1;
}

void closeHole(int index){
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
int bestFit( const void *a, const void *b){
  if((((avail_S*)a)->siz - ((avail_S*)b)->siz) == 0){
    return (((avail_S*)a)->offset - ((avail_S*)b)->offset);
  }
  return (((avail_S*)a)->siz - ((avail_S*)b)->siz);
}

int worstFit( const void *a, const void *b){
  if((((avail_S*)b)->siz - ((avail_S*)a)->siz)==0){
    return (((avail_S*)a)->offset - ((avail_S*)b)->offset);
  }
  return (((avail_S*)b)->siz - ((avail_S*)a)->siz);
}
void addHole(avail_S avail, int list_order){
  if(list_order == 1){
    aList[countAvail] = avail;
    countAvail = countAvail + 1;
  }
  if(list_order == 2){
    aList[countAvail] = avail;
    countAvail = countAvail + 1;
    qsort(aList, countAvail, sizeof(avail_S), bestFit);
  }
  if(list_order == 3){
    aList[countAvail] = avail;
    countAvail = countAvail + 1;
    qsort(aList, countAvail, sizeof(avail_S), worstFit);
  }
}

void removeIndex(int index){
  if(index >= countIndex){
    return;
  }
  else{
    int i = index;
    while(i < countIndex - 1){
      pKeyList[i] = pKeyList[i+1];
      i++;
    }
    countIndex = countIndex - 1;
  }
}

void addToFile(FILE *file, int key, int len, char *rec, int list_order){
  if(binary_search_key(key, 0, countIndex - 1) != -1){
    printf("Record with SID=%d exists\n", key);
    return;
  }
  long offset;
  int indexAvail = availHole(len);

  if(indexAvail != -1){
    fseek(file, aList[indexAvail].offset, SEEK_SET);
    fwrite(&len, sizeof(int),1, file);
    fwrite(rec, sizeof(char), len, file);

    int gapFilled = (sizeof(int) + len);
    int newHoleSize = aList[indexAvail].siz - gapFilled;
    int newHoleOffset = aList[indexAvail].offset + gapFilled;
    offset = newHoleOffset - gapFilled;
    closeHole(indexAvail);
    if(newHoleSize > 0){
      avail_S avail;
      avail.siz = newHoleSize;
      avail.offset = newHoleOffset;
      addHole(avail, list_order);
    }
  }
  else{
    fseek(file, 0, SEEK_END);
    offset = ftell(file);
    fwrite(&len, sizeof(int),1, file);
    fwrite(rec, sizeof(char), len, file);
  }

  index_S index;
  index.key = key;
  index.offset = offset;
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
    while(i > low){
      pKeyList[i] = pKeyList[i-1];
      i--;
    }
    pKeyList[low] = index;
    countIndex = countIndex + 1;
  }
  else{
    pKeyList[countIndex] = index;
    countIndex = countIndex + 1;
  }
}

void findRecordInFile(FILE *file, int key){
  int search = binary_search_key(key, 0, countIndex - 1);
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


void deleteRecordFromFile(FILE *file, int key, int list_order){
  int search = binary_search_key(key, 0, countIndex - 1);
  if( search != -1){
    avail_S avail;
    avail.offset = pKeyList[search].offset;
    fseek(file, avail.offset, SEEK_SET);
    int recordSize;
    fread(&recordSize, sizeof(int), 1, file);
    avail.siz = recordSize + sizeof(int);
    addHole(avail, list_order);
    removeIndex(search);
  }
  else{
    printf("No record with SID=%d exists\n", key);
  }
}



int main(int argc, char *argv[]){

  int list_order = 1;
  char *search = argv[1];
  char *filename = argv[2];

  FILE *file;

  if(( file = fopen(filename, "r+b")) == NULL){
    file = fopen(filename, "w+b");
    availFile = fopen("avail.bin", "w+b");
    indexFile = fopen("index.bin", "w+b");
  }
  else{
    readAvailFile();
    //lenOfAvail();
    readIndexFile();
    //lenOfIndex();

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

  while(1){
    input = (char *)malloc(sizeOfInput*sizeof(char));
    lenOfInput = getline(&input, &sizeOfInput, stdin);

    char **instr = breakInstr(input);

    if(!strcmp(instr[0], "add")){
      int key = atoi(instr[1]);
      int len = strlen(instr[2]);
      char *rec = malloc((int) len);
      strcpy(rec, instr[2]);
      addToFile(file, key, len, rec, list_order);
    }

    else if(!strcmp(instr[0], "find")){
      int key = atoi(instr[1]);
      findRecordInFile(file, key);
    }

    else if(!strcmp(instr[0], "del")){
      int key = atoi(instr[1]);
      deleteRecordFromFile(file, key, list_order);
    }
    else if(!strcmp(instr[0], "end")){
      fclose(file);
      writeAndCloseIndexFile();
      writeAndCloseAvailFile();
      break;
    }
    else{
      printf("wrong commands\n");
      continue;
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
    totalHoleSize = totalHoleSize + aList[i].siz;
    i++;
  }
  printf( "Number of holes: %d\n", countAvail);
  printf( "Hole space: %d\n", totalHoleSize);
  return(0);
}

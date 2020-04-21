#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <limits.h>

void print_standard(struct timeval  begin, struct timeval finish){

  finish.tv_sec = finish.tv_sec - begin.tv_sec;
  finish.tv_usec = finish.tv_usec - begin.tv_usec;

  if(finish.tv_usec < 0){
    finish.tv_usec = finish.tv_usec + 1000000;
    finish.tv_sec = finish.tv_sec - 1;
  }

  printf( "Time: %ld.%06ld\n", finish.tv_sec, finish.tv_usec );

}

void merge(int start, char input_filename[], FILE *output_file, int append_no, char append_name[], int no_of_runs){

  int size_filename = strlen(input_filename)+append_no;
  int output[1000];
  char run_filename[size_filename];
  FILE *run_files[no_of_runs];
  int inputs[no_of_runs][8];

  strcpy(run_filename, input_filename);
  strcat(run_filename, append_name);

  int i = 0;
  while(i < no_of_runs){
    char temp[size_filename];
    sprintf(temp, run_filename, i+start);
    run_files[i] = fopen(temp, "r");
    fread(inputs[i],sizeof(int), 8, run_files[i]);
    i = i + 1;
  }

  int file_read = 0;
  int index = 0;
  while(no_of_runs > file_read){
    int min = INT_MAX;
    int min_index = -1;
    i = 0;
    while(i < no_of_runs){
      int tem = -1;
      int j = 0;
      while(j < 8){
        if(inputs[i][j] != -1){
          tem = inputs[i][j];
          break;
        }
        j++;
      }

      if(tem != -1 ||(tem < min && tem != -1)){
        min_index = i;
        min = tem;
      }
      else if(run_files[i]){

        if(feof(run_files[i])){
          run_files[i] = '\0';
          file_read = file_read + 1;
          continue;
        }
        fread(inputs[i], sizeof(int), 8, run_files[i]);
        j = 0;
        while(j < 8){
          if(inputs[i][j] != -1){
            tem = inputs[i][j];
            break;
          }
          j++;
        }
        if(tem != -1 || (tem != -1 && tem < min)){
          min_index = i;
          min = tem;
        }
        else if(tem == -1){
          file_read = file_read + 1;
          run_files[i] = '\0';
        }
      }
      i++;
    }
    output[index++] = min;

    if(index >= 1000){
      index = 0;
      fwrite(output, sizeof(int), 1000, output_file);
    }

    if(min_index != -1){
      i = 0;
      while(i<8){
        if(inputs[min_index][i] != -1){
          inputs[min_index][i] = -1;
          break;
        }
        i++;
      }
    }
  }

  if(index != 0){
    size_t outIndex = (size_t)index;
    fwrite(output, sizeof(int), outIndex, output_file);
  }
}

void heapify(int heap_array[], int i, int n){

  int smallest = i;
  int left = 2*i + 1;
  int right = 2*i + 2;

  if(left < n && heap_array[left] < heap_array[i] ){
    smallest = left;
  }
  if(right < n && heap_array[right] < heap_array[smallest]){
    smallest = right;
  }
  if(smallest != i){
    int temp = heap_array[i];
    heap_array[i] = heap_array[smallest];
    heap_array[smallest] = temp;
    heapify(heap_array, smallest, n);
  }
}

void heap_sort(int heap_array[], int n){
  int i = n/2;
  while(i>=0){
    heapify(heap, i, n);
    i--;
  }
}
int comparision(const void* a, const void* b){
  return (*(int*)a - *(int*)b);
}

void sort_write_blocks(char input_filename[], FILE *input_file, int block_len, int run){
  int block[block_len];
  char run_filename[strlen(input_filename) + 4];
  char index[4];
  sprintf(index, ".%03d", run);
  strcpy(run_filename, input_filename);
  strcat(run_filename, index);

  FILE *temp_file = fopen(run_filename, "w");
  fread(block, sizeof(int), block_len, input_file);
  qsort(block,block_len, sizeof(int), comparision);
  fwrite(block, sizeof(int), block_len, temp_file);
  fclose(temp_file);
}

int main(int argc, char *argv[]){

int mergesort_method_flag = 10;
char mergesort_method[] = argv[1];
char index_filename[] = argv[2];
char sorted_index_filename[] = argv[3];

struct timeval begin;
struct timeval finish;

if(!strcmp(mergesort_method,"--basic")){
  mergesort_method_flag = 1;
}
else if (!strcmp(mergesort_method,"--multistep")){
  mergesort_method_flag = 2;
}
else if (!strcmp(mergesort_method,"--replacement")){
  mergesort_method_flag = 3;
}

FILE *index_file = fopen(index_filename, "r");
fseek(index_file, 0, SEEK_END);
size_t len_size = (size_t)ftell(index_file)/sizeof(int);

int no_of_runs = (int)len_size/1000;
if(((int)len_size)%1000 != 0){
  no_of_runs = no_of_runs + 1;
}
rewind(index_file);
int run = 0;
while(run < no_of_runs - 1){
  sort_write_blocks(index_filename, index_file, 1000, run);
  run++;
}
sort_write_blocks(index_filename, index_file, (int)len_size -(1000*no_of_runs - 1),no_of_runs - 1);
fclose(index_file);

switch(mergesort_method_flag){
  case 1: {
    gettimeofday(&begin, NULL);

    FILE *sorted_index_file = fopen(sorted_index_filename, "w");
    merge(0, index_filename, sorted_index_file, 4,".%03d", no_of_runs);
    fclose(sorted_index_file);

    gettimeofday(&finish, NULL);
    print_standard(begin, finish);
    break;
  }
  case 2: {
    gettimeofday(&begin, NULL);
    char super_run_filename[strlen(index_filename) + 10];
    FILE *sorted_index_file = fopen(sorted_index_filename, "w");

    int no_of_super_runs = no_of_runs/15;
    if(no_of_runs%15 != 0){
      no_of_super_runs = no_of_super_runs + 1;
    }

    int loop = 0;
    while(loop < no_of_super_runs - 1){
      char temp[10];
      strcpy(super_run_filename, index_filename);
      sprintf(temp, ".super.%03d", loop);
      strcat(super_run_filename, temp);
      FILE *super_run_file = fopen(super_run_filename, "w");
      merge(loop*15, index_filename, super_run_file, 4, ".%03d", 15);
      fclose(super_run_file);
      loop++;
    }
    char temp[10];
    strcpy(super_run_filename, index_filename);
    sprintf(temp, ".super.%03d", no_of_super_runs -1);
    strcat(super_run_filename, temp);
    FILE *super_run_file = fopen(super_run_filename, "w");
    merge((no_of_super_runs - 1)*15, index_filename, super_run_file, 4, ".%03d", no_of_runs - 15*(no_of_super_runs - 1));
    fclose(super_run_file);

    merge(0, index_filename, sorted_index_file, 10, ".super.%03d", no_of_super_runs);
    fclose(sorted_index_file);

    gettimeofday(&finish, NULL);
    print_standard(begin, finish);
    break;
  }
  case 3: {
    gettimeofday(&begin, NULL);
    //call replacement method
    gettimeofday(&finish, NULL);
    print_standard(begin, finish);
    break;
  }
  default: {
    break;
  }
}

return 0;
}

#include <buzz/buzzset.h>
#include <stdio.h>
#include <string.h>

/****************************************/
/****************************************/

void si_print_elem(void* data, void* params) {
   int d = *(int*)data;
   fprintf(stdout, "%d\n", d);
}

void si_print(buzzset_t si) {
   fprintf(stdout, "size: %u\n", buzzset_size(si));
   buzzset_foreach(si, si_print_elem, NULL);
   fprintf(stdout, "\n");
}

int si_cmp(const void* a, const void* b) {
   int d1 = *(const int*)a;
   int d2 = *(const int*)b;
   if(d1 < d2) return -1;
   if(d1 > d2) return  1;
   return 0;
}

int si_data[] = {
   5,7,3,1,9,10,6,6,2,1
};

/****************************************/
/****************************************/

void ss_print_elem(void* data, void* params) {
   char* d = *(char**)data;
   fprintf(stdout, "%s\n", d);
}

void ss_print(buzzset_t si) {
   fprintf(stdout, "size: %u\n", buzzset_size(si));
   buzzset_foreach(si, ss_print_elem, NULL);
   fprintf(stdout, "\n");
}

int ss_cmp(const void* a, const void* b) {
   const char* d1 = *(const char**)a;
   const char* d2 = *(const char**)b;
   return strcmp(d1, d2);
}

char* ss_data[] = {
   "rossi", "tassotti", "maldini", "albertini", "costacurta", "baresi", "lentini", "desailly", "papin", "savicevic", "massaro"
};

/****************************************/
/****************************************/

void test_int() {
   buzzset_t si = buzzset_new(sizeof(int),
                              si_cmp,
                              NULL);
   si_print(si);
   for(int i = 0; i < 10; ++i) {
      buzzset_insert(si, si_data + i);
   }
   si_print(si);
   for(int i = 0; i < 10; ++i) {
      buzzset_remove(si, &i);
   }
   si_print(si);
   buzzset_destroy(&si);
}

/****************************************/
/****************************************/

void test_str() {
   buzzset_t ss = buzzset_new(sizeof(char*),
                              ss_cmp,
                              NULL);
   ss_print(ss);
   for(int i = 0; i < 11; ++i) {
      buzzset_insert(ss, &ss_data[i]);
   }
   ss_print(ss);
   for(int i = 0; i < 4; ++i) {
      buzzset_remove(ss, &ss_data[i]);
   }
   ss_print(ss);
   buzzset_destroy(&ss);
}

/****************************************/
/****************************************/

int main() {
   test_str();
   return 0;
}

#include <buzz/buzzdarray.h>
#include <stdio.h>

void dai_print_elem(uint32_t pos, void* data, void* params) {
   fprintf(stdout, "[%u] %d\n", pos, *(int16_t*)(data));
}

void dai_print(buzzdarray_t da) {
   fprintf(stdout, "capacity: %u\n", buzzdarray_capacity(da));
   fprintf(stdout, "size: %lld\n", buzzdarray_size(da));
   buzzdarray_foreach(da, dai_print_elem, NULL);
   fprintf(stdout, "\n");
}

int dai_cmp(const void* a, const void* b) {
   int o1 = *(const int16_t*)(a);
   int o2 = *(const int16_t*)(b);
   fprintf(stdout, "  comparing %d and %d\n", o1, o2);
   if(o1 < o2) return -1;
   else if(o1 > o2) return 1;
   else return 0;
}

int main() {
   buzzdarray_t dai = buzzdarray_new(1, sizeof(int16_t), NULL);
   int x, i;
   dai_print(dai);
   for(i = 0; i < 10; ++i) {
      fprintf(stdout, "adding %d\n", i);
      buzzdarray_push(dai, &i);
      dai_print(dai);
   }

   x = 100;
   fprintf(stdout, "inserting %d at 0\n", x);
   buzzdarray_insert(dai, 0, &x);
   dai_print(dai);

   x = 200;
   fprintf(stdout, "inserting %d at 5\n", x);
   buzzdarray_insert(dai, 5, &x);
   dai_print(dai);

   x = 300;
   fprintf(stdout, "inserting %d at 7\n", x);
   buzzdarray_insert(dai, 7, &x);
   dai_print(dai);

   int y = 5;
   fprintf(stdout, "looking for 5: found at pos %u\n",
           buzzdarray_find(dai, dai_cmp, &y));
   y = 17;
   fprintf(stdout, "looking for 17: found at pos %u\n",
           buzzdarray_find(dai, dai_cmp, &y));
   fprintf(stdout, "\n");

   fprintf(stdout, "sorting\n");
   buzzdarray_sort(dai, dai_cmp);
   dai_print(dai);

   fprintf(stdout, "removing at 0\n");
   buzzdarray_remove(dai, 0);
   dai_print(dai);

   fprintf(stdout, "removing at 10\n");
   buzzdarray_remove(dai, 10);
   dai_print(dai);

   while(! buzzdarray_isempty(dai)) {
      fprintf(stdout, "pop\n");
      buzzdarray_pop(dai);
      dai_print(dai);
   }

   buzzdarray_destroy(&dai);
   return 0;
}

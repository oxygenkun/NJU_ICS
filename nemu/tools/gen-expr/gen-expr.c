#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <string.h>

// this should be enough
static char buf[65536] = {};
static char code_buf[65536 + 128] = {}; // a little larger than `buf`
static char *code_format =
"#include <stdio.h>\n"
"int main() { "
"  unsigned result = %s; "
"  printf(\"%%u\", result); "
"  return 0; "
"}";

static uint position = 0;

static u_int32_t choose(u_int32_t i){
  return rand() % i;
}

static void gen_num(){
  int len = sprintf(buf+position, "%u", choose(999));
  position += len;
}

static void gen(char c){
  buf[position] = c;
  ++position;
}

static void gen_rand_op(){
  char op[4] = {'+', '-', '*', '/'};
  gen(op[choose(4)]);
}

static void gen_rand_expr() {
   switch (choose(3)) {
    case 0: gen_num(); break;
    case 1: gen('('); gen_rand_expr(); gen(')'); break;
    //case 2: gen(' '); break;
    default: gen_rand_expr(); gen_rand_op(); gen_rand_expr(); break;
  }
}

int main(int argc, char *argv[]) {
  int seed = time(0);
  srand(seed);
  int loop = 1;
  if (argc > 1) {
    sscanf(argv[1], "%d", &loop);
  }
  int i;
  for (i = 0; i < loop; i ++) {
    //init
    memset(buf, '\0', 65536);
    memset(code_buf, '\0', 65536 + 128);
    position = 0;

    gen_rand_expr();

    sprintf(code_buf, code_format, buf);

    FILE *fp = fopen("/tmp/.code.c", "w");
    assert(fp != NULL);
    fputs(code_buf, fp);
    fclose(fp);

    int ret = system("gcc /tmp/.code.c -o /tmp/.expr");
    if (ret != 0) continue;

    fp = popen("/tmp/.expr", "r");
    assert(fp != NULL);

    int result;
    int success = fscanf(fp, "%d", &result);
    assert(success==1);
    pclose(fp);

    printf("%u %s\n", result, buf);
  }
  return 0;
}

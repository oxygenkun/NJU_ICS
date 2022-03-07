#include <isa.h>
#include <cpu/cpu.h>
#include <readline/readline.h>
#include <readline/history.h>
#include "sdb.h"
#include <utils.h>
#include <memory/paddr.h>

static int is_batch_mode = false;

void init_regex();
void init_wp_pool();
bool add_wp(char*);
void delete_wp(int);
void wp_display();

/* We use the `readline' library to provide more flexibility to read from stdin. */
static char* rl_gets() {
  static char *line_read = NULL;

  if (line_read) {
    free(line_read);
    line_read = NULL;
  }

  line_read = readline("(nemu) ");

  if (line_read && *line_read) {
    add_history(line_read);
  }

  return line_read;
}

static int cmd_c(char *args) {
  cpu_exec(-1);
  return 0;
}


static int cmd_q(char *args) {
  nemu_state.state = NEMU_QUIT;
  return -1;
}

static int cmd_si(char *args){
  if (args == NULL) {
      cpu_exec(1);
      return 0;
  }
  
  long int n = 0;
  if(sscanf(args, "%ld", &n) != 1 || n < 0){
    printf("bad args! Need a positive integer or noting.\n");
    return 0;
  }
  cpu_exec(n);
  return 0;
}

static int cmd_info(char *args){
  char c = '\0';
  if(sscanf(args, "%c", &c) != 1){
    printf("Bad args! Need 'r' or 'w'.\n");
    return 0;
  }

  switch(c){
    case 'r': isa_reg_display(); break;
    case 'w': wp_display(); break;
    default: printf("Bad args! Need 'r' or 'w'.\n"); break;
  }

  return 0;
}

static int cmd_x(char *args){
  int n;
  paddr_t addr;
  if(sscanf(args, "%d " FMT_PADDR, &n, &addr) != 2){
    printf("Bad args! Need 2 args: [N] and [EXPR].");
    return 0;
  }
  // TODO: need edge check
  for(int i=0; i<n; i++, addr +=4){
      word_t ret = paddr_read(addr, 4);
      printf( FMT_PADDR ": " FMT_WORD "\n", addr,  ret);
  }

  return 0;
}

static int cmd_p(char *args){
  if(args==NULL) {
    printf("Bad args!\n");
    return 0;
  } 

  bool success;
  int ret = expr(args, &success);
  
  if(success)
    printf("%u\n", ret);
  return 0;
}

static int cmd_w(char *args){
  if(args==NULL) {
    printf("Bad args!\n");
    return 0;
  } 

  if(!add_wp(args)){
    printf("set watch point failed\n");
  }
  return 0;
}

static int cmd_d(char *args){
  int n;
  if(args==NULL || sscanf(args, "%d", &n)!=1){
    printf("Bad args!\n");
    return 0;
  }
  delete_wp(n);
  return 0;
}

static int cmd_help(char *args);

static struct {
  const char *name;
  const char *description;
  int (*handler) (char *);
} cmd_table[] = {
    {"help", "Display informations about all supported commands", cmd_help},
    {"c", "Continue the execution of the program", cmd_c},
    {"q", "Exit NEMU", cmd_q},
    /* TODO: Add more commands */
    {"si", "Step one instruction exactly. Usage: si [N].\n"
           "\t* N: Argument N(deault is 1) means step N times",
     cmd_si},
    {"info", "Showing things about the program being debugged. "
             "Usage: info [r|w].\n"
             "\t* r: List of all registers and their contents.\n"
             "\t* w: List of all watch points and their contents.\n",
     cmd_info},
    {"x", "Examine memory. Usage: x N EXPR.\n"
          "\t* N: The repeat count.\n"
          "\t* EXPR: The address from expression.\n",
     cmd_x},
    {"p", "Print value of expression EXPR. Usage: p EXPR.\n", cmd_p},
    {"w", "Set a watchpoint for an expression. Usage: w EXPR\n", cmd_w},
    {"d", "Delete a watchpoint for a number. Usage: w Number\n", cmd_d},
  };

#define NR_CMD ARRLEN(cmd_table)

static int cmd_help(char *args) {
  /* extract the first argument */
  char *arg = strtok(NULL, " ");
  int i;

  if (arg == NULL) {
    /* no argument given */
    for (i = 0; i < NR_CMD; i ++) {
      printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
    }
  }
  else {
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(arg, cmd_table[i].name) == 0) {
        printf("%s - %s\n", cmd_table[i].name, cmd_table[i].description);
        return 0;
      }
    }
    printf("Unknown command '%s'\n", arg);
  }
  return 0;
}

void sdb_set_batch_mode() {
  is_batch_mode = true;
}

void sdb_mainloop() {
  if (is_batch_mode) {
    cmd_c(NULL);
    return;
  }

  for (char *str; (str = rl_gets()) != NULL; ) {
    char *str_end = str + strlen(str);

    /* extract the first token as the command */
    char *cmd = strtok(str, " ");
    if (cmd == NULL) { continue; }

    /* treat the remaining string as the arguments,
     * which may need further parsing
     */
    char *args = cmd + strlen(cmd) + 1;
    if (args >= str_end) {
      args = NULL;
    }

#ifdef CONFIG_DEVICE
    extern void sdl_clear_event_queue();
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; } // real exec func (including error exit)
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

void init_sdb() {
  /* Compile the regular expressions. */
  init_regex();

  /* Initialize the watchpoint pool. */
  init_wp_pool();
}

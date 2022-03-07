#include "sdb.h"

#define NR_WP 32

typedef struct watchpoint {
  int NO;
  struct watchpoint *next;

  /* DONE: Add more members if necessary */
  char expr[1024];
  u_int32_t value;
  bool is_active;
} WP;

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = (i == NR_WP - 1 ? NULL : &wp_pool[i + 1]);
  }

  head = NULL;
  free_ = wp_pool;
}

/* DONE: Implement the functionality of watchpoint */

static WP* new_wp(){
    if(free_ == NULL){
    printf("No free watch point\n");
    assert(0);
    return NULL;
  }

  WP* ret = free_;
  free_ = free_->next;
  ret->next = head;
  head = ret;
  ret->is_active = true;
  return ret;
} 

static void free_wp(WP *wp){
  if(head == wp){
    head = head->next;
  }
  memset(wp->expr, 0, 1024);
  wp->value = 0;
  wp->next = free_;
  wp->is_active = false;
  free_ = wp;
}

bool add_wp(char *e){
  bool success = true;
  u_int32_t val = expr(e, &success);
  if(success==false){
    return false;
  }
  WP* wp = new_wp();
  strcpy(wp->expr, e);
  wp->value = val;
  return true;
}

void delete_wp(int number){
  if(!wp_pool[number].is_active){
    printf("no watchpoint: %d\n", number);
    return;
  }
  free_wp(&wp_pool[number]);
}

void wp_display(){
  printf("Num\tValue\tExpr\n");
  
  for(WP* p = head; p!=NULL; p = p->next){
    printf("%d\t%u\t%s\n", p->NO, p->value, p->expr);
  }
}

bool wp_changed(){
  bool ret = false;
  for(WP *p=head; p!=NULL; p = p->next){
    bool success = true;
    u_int32_t new_val = expr(p->expr, &success);
    if(new_val!=p->value){
      printf("watchpoint %d: %s\n"
      "Old value = %u\n"
      "New value = %u\n", p->NO, p->expr, p->value, new_val);
      p->value = new_val;
      ret = true;
    }
  }
  return ret;
}
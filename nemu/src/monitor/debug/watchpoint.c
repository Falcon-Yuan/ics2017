#include "monitor/watchpoint.h"
#include "monitor/expr.h"
#include "string.h"

#define NR_WP 32

static WP wp_pool[NR_WP];
static WP *head, *free_;

WP* new_wp();
void free_wp(WP *wp);

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = 0;
    wp_pool[i].next = &wp_pool[i + 1];
    wp_pool[i].exp[0] = '\0';
    wp_pool[i].value = -1;
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}

/* TODO: Implement the functionality of watchpoint */


WP* new_wp() {
  if (free_ == NULL) {
    printf("There are no free watch points!\n");
    assert(0);
  }
  WP *wp = free_;
  free_ = free_->next;
  wp->next = NULL;

  return wp;
}
void free_wp(WP *wp) {
  wp->NO = 0;
  wp->exp[0] = '\0';
  wp->value = -1;
  wp->next = free_;
  free_ = wp;
}

void create_wp(char *args) {
  bool success = true;
  uint32_t result = expr(args, &success);

  if (!success) {
    printf("Invalid expression!\n");
    return ;
  }
  WP *wp = new_wp();
  wp->value = result;
  assert(strlen(args)<128);
  strcpy(wp->exp, args);

  if (head == NULL) {
    wp->NO = 1;
    head = wp;
  }
  else {
    WP *temp = head;
    while(temp->next != NULL) {
      temp = temp->next;
    }
    temp->next = wp;
    wp->NO = temp->NO + 1;
  }

  return ;
}

void delete_wp(int no) {

  if (head == NULL) {
    printf("There is no watchpoint to delete!\n");
    return ;
  }

  WP *wp = head,*fwp=NULL;
  while(wp->NO < no && wp->next)
  {
    fwp = wp;
    wp = wp->next;
  }
  if(wp->next == NULL && wp->NO == no){
    if(fwp == NULL) {
      head = NULL;
      free_wp(wp);
    }
    else {
      fwp->next = NULL;
      free_wp(wp);
    }
  }
  else if(wp->next == NULL) {
    printf("There is no watchpoint with number %d\n",no);
  }
  else if(wp->NO == no) {
    if(fwp == NULL) {
      head = wp->next;
      fwp = head;
      fwp->NO = 1;
      while(fwp->next) {
        fwp->next->NO = fwp->NO + 1;
        fwp = fwp->next;
      }
    }
    else {
      fwp->next = wp->next;
      while(fwp->next) {
        fwp->next->NO = fwp->NO + 1;
        fwp = fwp->next;
      }
      free_wp(wp);
    }
  }


  return ;
}
void show_wp() {
  if (head == NULL) {
    printf("There is no watchpoint!\n");
    return ;
  }

  WP *wp;
  printf("NO    expression    value\n");
  wp = head;
  while (wp != NULL) {
    printf("%-5d %-13s 0x%08x\n", wp->NO, wp->exp, wp->value);
    wp = wp->next;
  }
}
int * point_changed() {
  WP *wp = head;
  bool success;
  uint32_t val;
  static int points[NR_WP];
  int i = 0;

  while (wp != NULL) {
    val = expr(wp->exp, &success);
    if (val != wp->value) {
      wp->value = val;
      points[i++] = wp->NO;
    }
      wp = wp->next;
  }
  points[i] = -1;

  return points;
}

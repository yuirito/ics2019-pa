#include "monitor/watchpoint.h"
#include "monitor/expr.h"

#define NR_WP 32

static WP wp_pool[NR_WP] = {};
static WP *head = NULL, *free_ = NULL;

void init_wp_pool() {
  int i;
  for (i = 0; i < NR_WP; i ++) {
    wp_pool[i].NO = i;
    wp_pool[i].next = &wp_pool[i + 1];
  }
  wp_pool[NR_WP - 1].next = NULL;

  head = NULL;
  free_ = wp_pool;
}


/* TODO: Implement the functionality of watchpoint */
/*PA1.3*/
WP* new_wp(char *s){
  if(s == NULL){
    assert(0);
  }
  if(free_ == NULL){
    printf("There is no free watchpointer");
    return NULL;
  }
  WP* p = free_;
	free_ = free_ -> next;
	p -> next = head;
	head = p;
  strcpy(head->expr,s);
  bool success = true;
  uint32_t result = expr(s,&success);
  if(!success){
    printf("Invalid expression!");
    return NULL;
  }
  head->value=result;
  head->hit=0;
	return head;
}

void free_wp(WP *wp){
	if(head == NULL){
		printf("There is no watchpoint to free.\n");
		return ;
	}
	if(wp == head){
		head = head -> next;
		wp -> next = free_;
		free_ = wp;	
		return ;
	}

	WP *p = head;
	while(p -> next != wp) p = p -> next;
	p -> next = wp -> next;
	wp -> next = free_;
	free_ = wp;
}

bool del_wp(int n)
{
	WP *p = head;
	while(p != NULL){
		if(p -> NO == n){
			free_wp(p);
			break;
		}else{
			p = p -> next;
		}
	}
	return p != NULL;
}

void display_wp()
{
	if(head == NULL) {
		printf("There is no watchpoint!\n");
		return;
	}
	printf("Num     What     Value\n");
	WP *p = head;
	while(p != NULL) {
		printf("%-8d%-9s%u(%#x)\n", p -> NO, p -> expr, p->value, p->value);
		if(p -> hit > 0) printf("\tbreakpoint already hit %d time\n", p->hit);
		p = p -> next;
	}
}

bool check_wp()
{
	bool changed = false;
  WP *wp = head;
	while(wp != NULL){
		bool success = true;
		uint32_t new_val = expr(wp -> expr, &success);
		assert(success);	
		if(new_val != wp -> value){
			printf("Watchpoint %d: %s\n", wp->NO, wp->expr);
			printf("Old value = %u\n", wp->value);
			printf("New value = %u\n",  new_val);
			changed = true;
			wp -> value = new_val;
			wp -> hit ++ ;
		}
		wp = wp -> next;
	}
	return changed;
}
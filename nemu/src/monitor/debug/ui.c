#include "monitor/monitor.h"
#include "monitor/expr.h"
#include "monitor/watchpoint.h"
#include "nemu.h"

#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>

void cpu_exec(uint64_t);
void isa_reg_display();
void display_wp();
bool del_wp(int n);
WP* new_wp(char *s);

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
  return -1;
}

/* PA1 */
static int cmd_help(char *args);
static void cmd_err(int err_type,const char *command);
static int cmd_si(char *args);
static int cmd_info(char *args);
static int cmd_x(char *args);
static int cmd_p(char *args);
static int cmd_w(char *args);
static int cmd_d(char *args);

/* PA3.3 */
static int cmd_detach(char * args);
static int cmd_attach(char * args);
static int cmd_save(char * args);
static int cmd_load(char * args);
void difftest_detach();
void difftest_attach();
bool isa_save(const FILE *fp);
bool isa_load(FILE *fp);

static struct {
  char *name;
  char *description;
  int (*handler) (char *);
} cmd_table [] = {
  { "help", "Display informations about all supported commands", cmd_help },
  { "c", "Continue the execution of the program", cmd_c },
  { "q", "Exit NEMU", cmd_q },

  /* TODO: Add more commands */
  /*PA1.1*/
  { "si", "Usage: si [N]\n Execute the program with N(default: 1) step", cmd_si},
  { "info", "Usage: info [rw]\n"\
  "info r: print the values of all registers\n"\
  "info w: show information about watchpoint", cmd_info},
  { "x", "Usage: x [N] [EXPR]\n" \
	"print N(default:1) consecutive 4 bytes starting from address calculated from EXPR", cmd_x },
  { "p", "Usage: p EXPR\nPrint the value of expression", cmd_p},
  { "w", "Usage: w EXPR\nAdd watchpoint", cmd_w},
  { "d", "Usage: d N\nDelete No N watchpoint", cmd_d},
  /* PA3.3 */
  { "detach", "Usage:detach \nQuit Diff-Test mode", cmd_detach },
  { "attach", "Usage:attach \nOpen Diff-Test mode", cmd_attach },
  { "save", "Usage:save [path] \nSave current state to path", cmd_save },
  { "load", "Usage:load [path] \nLoad state from path", cmd_load }
};

#define NR_CMD (sizeof(cmd_table) / sizeof(cmd_table[0]))

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

/* PA1.1 */

static void cmd_err(int err_type,const char *command){
    switch(err_type){
        case 0:
            printf("Invalid arguments for command '%s'\n",command);
            break;
        case 1:
            printf("Lack arguments for command '%s'\n",command);
            break;
        default:
            printf("Unknown error\n");
            break;
    }
}

static int cmd_si(char *args) {
    /* extract the first argument */
    char *arg = strtok(NULL, " ");
    if (arg == NULL) {
        /* no argument given */
        cpu_exec(1);
    }
    else {
        int n = atoi(arg);
        if(n>0)
            cpu_exec(n);
        else
            cmd_err(0, "si:N<=0");
    }
    return 0;
}

static int cmd_info(char *args) {
    /* extract the first argument */

    char *arg = strtok(NULL, " ");
    if (arg == NULL) {
        /* no argument given */
        cmd_err(0, "info: no argument given\n");
    }
    else{
        if(*arg == 'r') isa_reg_display();
        else if(*arg == 'w') display_wp();
        else cmd_err(0, "info: unknown argument");
    }
    return 0;

}

static int cmd_x(char *args) {
    /* extract the first argument */
    char *arg1 = strtok(NULL, " ");
    char *arg2 = strtok(NULL, " ");
    if (arg1 == NULL || arg2 == NULL) {
        /* no argument given */
        cmd_err(1, "x");
    }
    else{
        int n = atoi(arg1);
        if (n<=0){
            cmd_err(0, "x:N<=0");
        }
        else{
            int addr;
            sscanf(arg2,"%x",&addr);
            for(int i=0;i<n;i++,addr+=4){
                uint32_t data;
                data = vaddr_read(addr,4);
                if((i & 0x3) == 0)
                    printf("0x%08x: ",addr);
                printf("0x%08x%*s", data,4,"");
                if((i & 0x3) == 0x3)
                    printf("\n");
            }
            printf("\n");
        }
    }
    return 0;
}

static int cmd_p(char *args) {
    /* extract the first argument */
    if (args == NULL) {
        /* no argument given */
        cmd_err(0, "p: no argument given\n");
    }
    else{
        bool success = true;
        uint32_t result = expr(args,&success);
        if(success) printf("0x%x(%d)\n",result,result);
        else printf("Invalid expr\n");

    }
    return 0;

}

static int cmd_w(char *args) {
    /* extract the first argument */

    char *arg = strtok(NULL, " ");
    if (arg == NULL) {
        /* no argument given */
        cmd_err(1, "w\n");
    }
    else{
        WP *p = new_wp(arg);
        printf("watchpoint %d : %s\n", p->NO,p->expr);
    }
    return 0;

}

static int cmd_d(char *args) {
    /* extract the first argument */

    char *arg = strtok(NULL, " ");
    if (arg == NULL) {
        /* no argument given */
        cmd_err(1, "d\n");
    }
    else{
        int n = atoi(arg);
        if( n<0 ){
          cmd_err(0, "d:N must be in [0,31] \n");
        }
        del_wp(n);
    }
    return 0;

}



void ui_mainloop(int is_batch_mode) {
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

#ifdef HAS_IOE
    extern void sdl_clear_event_queue(void);
    sdl_clear_event_queue();
#endif

    int i;
    for (i = 0; i < NR_CMD; i ++) {
      if (strcmp(cmd, cmd_table[i].name) == 0) {
        if (cmd_table[i].handler(args) < 0) { return; }
        break;
      }
    }

    if (i == NR_CMD) { printf("Unknown command '%s'\n", cmd); }
  }
}

static int cmd_detach(char * args) {
  difftest_detach();
  return 0;
}

static int cmd_attach(char *args) {
  difftest_attach();
  return 0;
}

static int cmd_save(char * args) {
  if (args == NULL) {
    printf("please input parameter [path].\n");
  }
  else {
    FILE *fp = fopen(args, "w");
    if (fp == NULL) {
      printf("file %s failed to open.\n", args);
    }
    else if (isa_save(fp) != true) {
      printf("file %s failed to save.\n", args);
      fclose(fp);
    }
    else {
      printf("save at : %s\n", args);
      fclose(fp);
    }
  }
  return 0;
}

static int cmd_load(char *args) {
  if (args == NULL) {
    printf("please input parameter [path].\n");
  }
  else {
    FILE *fp = fopen(args, "r");
    if (fp == NULL) {
      printf("file %s failed to open.\n", args);
    }
    else if (isa_load(fp) != true) {
      printf("file %s failed to load.\n", args);
      fclose(fp);
    }
    else {
      printf("load from : %s\n", args);
      fclose(fp);
      difftest_attach();
    }
  }
  return 0;
}

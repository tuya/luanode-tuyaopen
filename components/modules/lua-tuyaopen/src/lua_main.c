/*
** $Id: lua.c,v 1.230.1.1 2017/04/19 17:29:57 roberto Exp $
** Lua stand-alone interpreter
** See Copyright Notice in lua.h
*/

#define lua_c

#include "lprefix.h"


#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "lua.h"

#include "lauxlib.h"
#include "lualib.h"
#include "tkl_thread.h"
#include "lua_main.h"
#include "tal_log.h"
#include "lua_msgbus.h"
#include "lapi.h"
#include "uart_handler.h"

#if !defined(LUA_PROMPT)
#define LUA_PROMPT		"> "
#define LUA_PROMPT2		">> "
#endif

#if !defined(LUA_PROGNAME)
#define LUA_PROGNAME		"lua"
#endif

#if !defined(LUA_MAXINPUT)
#define LUA_MAXINPUT		512
#endif

#if !defined(LUA_INIT_VAR)
#define LUA_INIT_VAR		"LUA_INIT"
#endif

#define LUA_INITVARVERSION	LUA_INIT_VAR LUA_VERSUFFIX


// #define CLEAR() printf("\033[2J")// 清除屏幕
// #define MOVEUP(x) printf("\033[%dA", (x))// 上移光标
// #define MOVEDOWN(x) printf("\033[%dB", (x))// 下移光标
// #define MOVELEFT(y) printf("\033[%dD", (y))// 左移光标
// #define MOVERIGHT(y) printf("\033[%dC",(y))// 右移光标
// #define LUA_MAXINPUT_HIS 10
// extern void uart_init();
// extern int uart_getc(char *c);
// extern int uart_tx_one_char(char c);

/*
** lua_stdin_is_tty detects whether the standard input is a 'tty' (that
** is, whether we're running lua interactively).
*/
#if !defined(lua_stdin_is_tty)	/* { */

#if defined(LUA_USE_POSIX)	/* { */

#include <unistd.h>
#define lua_stdin_is_tty()	isatty(0)

#elif defined(LUA_USE_WINDOWS)	/* }{ */

#include <io.h>
#include <windows.h>

#define lua_stdin_is_tty()	_isatty(_fileno(stdin))

#else				/* }{ */

/* ISO C definition */
#define lua_stdin_is_tty()	1  /* assume stdin is a tty */

#endif				/* } */

#endif				/* } */


/*
** lua_readline defines how to show a prompt and then read a line from
** the standard input.
** lua_saveline defines how to "save" a read line in a "history".
** lua_freeline defines how to free a line read by lua_readline.
*/
#if !defined(lua_readline)	/* { */

#if defined(LUA_USE_READLINE)	/* { */

#include <readline/readline.h>
#include <readline/history.h>
#define lua_readline(L,b,p)	((void)L, ((b)=readline(p)) != NULL)
#define lua_saveline(L,line)	((void)L, add_history(line))
#define lua_freeline(L,b)	((void)L, free(b))

#else				/* }{ */
#if OPERATING_SYSTEM == SYSTEM_LINUX
#define lua_readline(L,b,p) \
        ((void)L, fputs(p, stdout), fflush(stdout),  /* show prompt */ \
        fgets(b, LUA_MAXINPUT, stdin) != NULL)  /* get line */
#else
#define lua_readline(L,b,p) \
        ((void)L, uart_sendStr(p), /* show prompt */ \
        uart_getline(p, b, LUA_MAXINPUT) != 0)  /* get line */
#endif
#define lua_saveline(L,line)	{ (void)L; (void)line; }
#define lua_freeline(L,b)	{ (void)L; (void)b; }

#endif				/* } */

#endif				/* } */


// typedef struct __lua_load{
//   lua_State *L;
//   int firstline;
//   char *line;
//   int line_position;
//   int curr_position;
//   size_t len;
//   int done;
//   const char *prmt;
// }lua_Load;

// lua_Load gLoad;


static lua_State *globalL = NULL; 
// // extern const char *c_getenv(const char *__string);
// char line_buffer[LUA_MAXINPUT];
// char line_buffer_his[LUA_MAXINPUT_HIS][LUA_MAXINPUT];

// static char last_nl_char = '\0';
// static uint8_t is_key_button = 0;
// static int8_t his_cnt = 0;
// static int8_t his_cnt_sel = 0;
static const char *progname = LUA_PROGNAME;


void uart_sendStr(const char *str)
{
    while(*str) {
        uart_tx_one_char(*str++);
    }
}

/*
** Hook set by signal function to stop the interpreter.
*/
static void lstop (lua_State *L, lua_Debug *ar) {
  (void)ar;  /* unused arg. */
  lua_sethook(L, NULL, 0, 0);  /* reset hook */
  luaL_error(L, "interrupted!");
}


/*
** Function to be called at a C signal. Because a C signal cannot
** just change a Lua state (as there is no proper synchronization),
** this function only sets a hook that, when called, will stop the
** interpreter.
*/
static void laction (int i) {
  signal(i, SIG_DFL); /* if another SIGINT happens, terminate process */
  lua_sethook(globalL, lstop, LUA_MASKCALL | LUA_MASKRET | LUA_MASKCOUNT, 1);
}


static void print_usage (const char *badoption) {
  lua_writestringerror("%s: ", progname);
  if (badoption[1] == 'e' || badoption[1] == 'l')
    lua_writestringerror("'%s' needs argument\n", badoption);
  else
    lua_writestringerror("unrecognized option '%s'\n", badoption);
  lua_writestringerror(
  "usage: %s [options] [script [args]]\n"
  "Available options are:\n"
  "  -e stat  execute string 'stat'\n"
  "  -i       enter interactive mode after executing 'script'\n"
  "  -l name  require library 'name' into global 'name'\n"
  "  -v       show version information\n"
  "  -E       ignore environment variables\n"
  "  --       stop handling options\n"
  "  -        stop handling options and execute stdin\n"
  ,
  progname);
}


/*
** Prints an error message, adding the program name in front of it
** (if present)
*/
static void l_message (const char *pname, const char *msg) {
  if (pname) lua_writestringerror("%s: ", pname);
  lua_writestringerror("%s\n", msg);
}


/*
** Check whether 'status' is not OK and, if so, prints the error
** message on the top of the stack. It assumes that the error object
** is a string, as it was either generated by Lua or by 'msghandler'.
*/
static int report (lua_State *L, int status) {
  if (status != LUA_OK) {
    const char *msg = lua_tostring(L, -1);
    l_message(progname, msg);
    lua_pop(L, 1);  /* remove message */
  }
  return status;
}


/*
** Message handler used to run all chunks
*/
static int msghandler (lua_State *L) {
  const char *msg = lua_tostring(L, 1);
  if (msg == NULL) {  /* is error object not a string? */
    if (luaL_callmeta(L, 1, "__tostring") &&  /* does it have a metamethod */
        lua_type(L, -1) == LUA_TSTRING)  /* that produces a string? */
      return 1;  /* that is the message */
    else
      msg = lua_pushfstring(L, "(error object is a %s value)",
                               luaL_typename(L, 1));
  }
  luaL_traceback(L, L, msg, 1);  /* append a standard traceback */
  return 1;  /* return the traceback */
}


/*
** Interface to 'lua_pcall', which sets appropriate message function
** and C-signal handler. Used to run all chunks.
*/
static int docall (lua_State *L, int narg, int nres) {
  int status;
  int base = lua_gettop(L) - narg;  /* function index */
  lua_pushcfunction(L, msghandler);  /* push message handler */
  lua_insert(L, base);  /* put it under function and args */
  globalL = L;  /* to be available to 'laction' */
  signal(SIGINT, laction);  /* set C-signal handler */
  status = lua_pcall(L, narg, nres, base);
  signal(SIGINT, SIG_DFL); /* reset C-signal handler */
  lua_remove(L, base);  /* remove message handler from the stack */
  return status;
}


static void print_version (void) {
  lua_writestring(LUA_COPYRIGHT, strlen(LUA_COPYRIGHT));
  lua_writeline();
}


/*
** Create the 'arg' table, which stores all arguments from the
** command line ('argv'). It should be aligned so that, at index 0,
** it has 'argv[script]', which is the script name. The arguments
** to the script (everything after 'script') go to positive indices;
** other arguments (before the script name) go to negative indices.
** If there is no script name, assume interpreter's name as base.
*/
static void createargtable (lua_State *L, char **argv, int argc, int script) {
  int i, narg;
  if (script == argc) script = 0;  /* no script name? */
  narg = argc - (script + 1);  /* number of positive indices */
  lua_createtable(L, narg, script + 1);
  for (i = 0; i < argc; i++) {
    lua_pushstring(L, argv[i]);
    lua_rawseti(L, -2, i - script);
  }
  lua_setglobal(L, "arg");
}


static int dochunk (lua_State *L, int status) {
  if (status == LUA_OK) status = docall(L, 0, 0);
  return report(L, status);
}


static int dofile (lua_State *L, const char *name) {
  return dochunk(L, luaL_loadfile(L, name));
}


static int dostring (lua_State *L, const char *s, const char *name) {
  return dochunk(L, luaL_loadbuffer(L, s, strlen(s), name));
}


/*
** Calls 'require(name)' and stores the result in a global variable
** with the given name.
*/
static int dolibrary (lua_State *L, const char *name) {
  int status;
  lua_getglobal(L, "require");
  lua_pushstring(L, name);
  status = docall(L, 1, 1);  /* call 'require(name)' */
  if (status == LUA_OK)
    lua_setglobal(L, name);  /* global[name] = require return */
  return report(L, status);
}


/*
** Returns the string to be used as a prompt by the interpreter.
*/
static const char *get_prompt (lua_State *L, int firstline) {
  const char *p;
  lua_getglobal(L, firstline ? "_PROMPT" : "_PROMPT2");
  p = lua_tostring(L, -1);
  if (p == NULL) p = (firstline ? LUA_PROMPT : LUA_PROMPT2);
  return p;
}

/* mark in error messages for incomplete statements */
#define EOFMARK		"<eof>"
#define marklen		(sizeof(EOFMARK)/sizeof(char) - 1)


/*
** Check whether 'status' signals a syntax error and the error
** message at the top of the stack ends with the above mark for
** incomplete statements.
*/
static int incomplete (lua_State *L, int status) {
  if (status == LUA_ERRSYNTAX) {
    size_t lmsg;
    const char *msg = lua_tolstring(L, -1, &lmsg);
    if (lmsg >= marklen && strcmp(msg + lmsg - marklen, EOFMARK) == 0) {
      lua_pop(L, 1);
      return 1;
    }
  }
  return 0;  /* else... */
}


/*
** Prompt the user, read a line, and push it into the Lua stack.
*/
static int pushline (lua_State *L, int firstline) {
  char buffer[LUA_MAXINPUT];
  char *b = buffer;
  size_t l;
  const char *prmt = get_prompt(L, firstline);
  int readstatus = lua_readline(L, b, prmt);
  if (readstatus == 0)
    return 0;  /* no input (prompt will be popped by caller) */
  lua_pop(L, 1);  /* remove prompt */
  l = strlen(b);
  if (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
    b[--l] = '\0';  /* remove it */
  if (firstline && b[0] == '=')  /* for compatibility with 5.2, ... */
    lua_pushfstring(L, "return %s", b + 1);  /* change '=' to 'return' */
  else
    lua_pushlstring(L, b, l);
  lua_freeline(L, b);
  return 1;
}


/*
** Try to compile line on the stack as 'return <line>;'; on return, stack
** has either compiled chunk or original line (if compilation failed).
*/
static int addreturn (lua_State *L) {
  const char *line = lua_tostring(L, -1);  /* original line */
  const char *retline = lua_pushfstring(L, "return %s;", line);
  int status = luaL_loadbuffer(L, retline, strlen(retline), "=stdin");
  if (status == LUA_OK) {
    lua_remove(L, -2);  /* remove modified line */
    if (line[0] != '\0')  /* non empty? */
      lua_saveline(L, line);  /* keep history */
  }
  else
    lua_pop(L, 2);  /* pop result from 'luaL_loadbuffer' and modified line */
  return status;
}


/*
** Read multiple lines until a complete Lua statement
*/
static int multiline (lua_State *L) {
  for (;;) {  /* repeat until gets a complete statement */
    size_t len;
    const char *line = lua_tolstring(L, 1, &len);  /* get what it has */
    int status = luaL_loadbuffer(L, line, len, "=stdin");  /* try it */
    if (!incomplete(L, status) || !pushline(L, 0)) {
      lua_saveline(L, line);  /* keep history */
      return status;  /* cannot or should not try to add continuation line */
    }
    lua_pushliteral(L, "\n");  /* add newline... */
    lua_insert(L, -2);  /* ...between the two lines */
    lua_concat(L, 3);  /* join them */
  }
}


/*
** Read a line and try to load (compile) it first as an expression (by
** adding "return " in front of it) and second as a statement. Return
** the final status of load/call with the resulting function (if any)
** in the top of the stack.
*/
static int loadline (lua_State *L) {
  int status;
  lua_settop(L, 0);
  if (!pushline(L, 1))
    return -1;  /* no input */
  if ((status = addreturn(L)) != LUA_OK)  /* 'return ...' did not work? */
    status = multiline(L);  /* try as command, maybe with continuation lines */
  lua_remove(L, 1);  /* remove line from the stack */
  lua_assert(lua_gettop(L) == 1);
  return status;
}


/*
** Prints (calling the Lua 'print' function) any values on the stack
*/
static void l_print (lua_State *L) {
  int n = lua_gettop(L);
  if (n > 0) {  /* any result to be printed? */
    luaL_checkstack(L, LUA_MINSTACK, "too many results to print");
    lua_getglobal(L, "print");
    lua_insert(L, 1);
    if (lua_pcall(L, n, 0, 0) != LUA_OK)
      l_message(progname, lua_pushfstring(L, "error calling 'print' (%s)",
                                             lua_tostring(L, -1)));
  }
}


/*
** Do the REPL: repeatedly read (load) a line, evaluate (call) it, and
** print any results.
*/
static void doREPL (lua_State *L) {
  int status;
  const char *oldprogname = progname;
  progname = NULL;  /* no 'progname' on errors in interactive mode */
  while ((status = loadline(L)) != -1) {
    if (status == LUA_OK)
      status = docall(L, 0, LUA_MULTRET);
    if (status == LUA_OK) l_print(L);
    else report(L, status);
  }
  lua_settop(L, 0);  /* clear stack */
  lua_writeline();
  progname = oldprogname;
}


/*
** Push on the stack the contents of table 'arg' from 1 to #arg
*/
static int pushargs (lua_State *L) {
  int i, n;
  if (lua_getglobal(L, "arg") != LUA_TTABLE)
    luaL_error(L, "'arg' is not a table");
  n = (int)luaL_len(L, -1);
  luaL_checkstack(L, n + 3, "too many arguments to script");
  for (i = 1; i <= n; i++)
    lua_rawgeti(L, -i, i);
  lua_remove(L, -i);  /* remove table from the stack */
  return n;
}


static int handle_script (lua_State *L, char **argv) {
  int status;
  const char *fname = argv[0];
  if (strcmp(fname, "-") == 0 && strcmp(argv[-1], "--") != 0)
    fname = NULL;  /* stdin */
  status = luaL_loadfile(L, fname);
  if (status == LUA_OK) {
    int n = pushargs(L);  /* push arguments to script */
    status = docall(L, n, LUA_MULTRET);
  }
  return report(L, status);
}



/* bits of various argument indicators in 'args' */
#define has_error	1	/* bad option */
#define has_i		2	/* -i */
#define has_v		4	/* -v */
#define has_e		8	/* -e */
#define has_E		16	/* -E */

/*
** Traverses all arguments from 'argv', returning a mask with those
** needed before running any Lua code (or an error code if it finds
** any invalid argument). 'first' returns the first not-handled argument
** (either the script name or a bad argument in case of error).
*/
static int collectargs (char **argv, int *first) {
  int args = 0;
  int i;
  for (i = 1; argv[i] != NULL; i++) {
    *first = i;
    if (argv[i][0] != '-')  /* not an option? */
        return args;  /* stop handling options */
    switch (argv[i][1]) {  /* else check option */
      case '-':  /* '--' */
        if (argv[i][2] != '\0')  /* extra characters after '--'? */
          return has_error;  /* invalid option */
        *first = i + 1;
        return args;
      case '\0':  /* '-' */
        return args;  /* script "name" is '-' */
      case 'E':
        if (argv[i][2] != '\0')  /* extra characters after 1st? */
          return has_error;  /* invalid option */
        args |= has_E;
        break;
      case 'i':
        args |= has_i;  /* (-i implies -v) *//* FALLTHROUGH */
      case 'v':
        if (argv[i][2] != '\0')  /* extra characters after 1st? */
          return has_error;  /* invalid option */
        args |= has_v;
        break;
      case 'e':
        args |= has_e;  /* FALLTHROUGH */
      case 'l':  /* both options need an argument */
        if (argv[i][2] == '\0') {  /* no concatenated argument? */
          i++;  /* try next 'argv' */
          if (argv[i] == NULL || argv[i][0] == '-')
            return has_error;  /* no next argument or it is another option */
        }
        break;
      default:  /* invalid option */
        return has_error;
    }
  }
  *first = i;  /* no script name */
  return args;
}


/*
** Processes options 'e' and 'l', which involve running Lua code.
** Returns 0 if some code raises an error.
*/
static int runargs (lua_State *L, char **argv, int n) {
  int i;
  for (i = 1; i < n; i++) {
    int option = argv[i][1];
    lua_assert(argv[i][0] == '-');  /* already checked */
    if (option == 'e' || option == 'l') {
      int status;
      const char *extra = argv[i] + 2;  /* both options need an argument */
      if (*extra == '\0') extra = argv[++i];
      lua_assert(extra != NULL);
      status = (option == 'e')
               ? dostring(L, extra, "=(command line)")
               : dolibrary(L, extra);
      if (status != LUA_OK) return 0;
    }
  }
  return 1;
}



static int handle_luainit (lua_State *L) {
  const char *name = "=" LUA_INITVARVERSION;
  const char *init = getenv(name + 1);
  if (init == NULL) {
    name = "=" LUA_INIT_VAR;
    init = getenv(name + 1);  /* try alternative name */
  }
  if (init == NULL) return LUA_OK;
  else if (init[0] == '@')
    return dofile(L, init+1);
  else
    return dostring(L, init, name);
}


/*
** Main body of stand-alone interpreter (to be called in protected mode).
** Reads the options and handles them all.
*/
static int pmain (lua_State *L) {
  int argc = (int)lua_tointeger(L, 1);
  char **argv = (char **)lua_touserdata(L, 2);
  int script;
  int args = collectargs(argv, &script);
  luaL_checkversion(L);  /* check that interpreter has correct version */
  if (argv[0] && argv[0][0]) progname = argv[0];
  if (args == has_error) {  /* bad arg? */
    print_usage(argv[script]);  /* 'script' has index of bad arg. */
    return 0;
  }
  if (args & has_v)  /* option '-v'? */
    print_version();
  if (args & has_E) {  /* option '-E'? */
    lua_pushboolean(L, 1);  /* signal for libraries to ignore env. vars. */
    lua_setfield(L, LUA_REGISTRYINDEX, "LUA_NOENV");
  }
  luaL_openlibs(L);  /* open standard libraries */
  createargtable(L, argv, argc, script);  /* create table 'arg' */
  if (!(args & has_E)) {  /* no option '-E'? */
    if (handle_luainit(L) != LUA_OK)  /* run LUA_INIT */
      return 0;  /* error running LUA_INIT */
  }
  if (!runargs(L, argv, script))  /* execute arguments -e and -l */
    return 0;  /* something failed */
  if (script < argc &&  /* execute main script (if there is one) */
      handle_script(L, argv + script) != LUA_OK)
    return 0;
  if (args & has_i)  /* -i option? */
    doREPL(L);  /* do read-eval-print loop */
  else if (script == argc && !(args & (has_e | has_v))) {  /* no arguments? */
    if (lua_stdin_is_tty()) {  /* running in interactive mode? */
      print_version();
      doREPL(L);  /* do read-eval-print loop */
    }
    else dofile(L, NULL);  /* executes stdin as a file */
  }
  lua_pushboolean(L, 1);  /* signal no errors */
  return 1;
}


// static bool __readline(lua_Load *load){
//   int need_dojob = false;
//   char ch = '0';
//   while (uart_getc(&ch)) {
//     PR_DEBUG("uart_getc %d", ch);
//     char tmp_last_nl_char = last_nl_char;
//     // reset marker, will be finally set below when newline is processed
//     last_nl_char = '\0';
//     /* handle CR & LF characters
//       filters second char of LF&CR (\n\r) or CR&LF (\r\n) sequences */
//     if ((ch == '\r' && tmp_last_nl_char == '\n') || // \n\r sequence -> skip \r
//           (ch == '\n' && tmp_last_nl_char == '\r'))   // \r\n sequence -> skip \n
//     {
//       continue;
//     }
    
//     if (ch == 0x1b || is_key_button > 0) { //handle direct keys
//       if (is_key_button == 2) {
//         if (ch == 0x44) { //left key
//           if (load->line_position > 0 && load->curr_position > 0) {
//             uart_tx_one_char('\b');
//             load->curr_position--;
//           }
//         } else if (ch == 0x43) { //right key
//           if (load->curr_position < load->line_position) {
//             uart_tx_one_char(line_buffer[load->curr_position]);
//             load->curr_position++;
//           }
//         } else if (ch == 0x41) { //up key
//           if (his_cnt > 0) {
//             for (int i = 0; i < load->line_position; i++) {
//               uart_tx_one_char(0x08);
//               uart_tx_one_char(' ');
//               uart_tx_one_char(0x08);
//             }
//             memcpy(line_buffer, line_buffer_his[his_cnt_sel-1], LUA_MAXINPUT);
//             load->curr_position = strlen(line_buffer_his[his_cnt_sel-1]);
//             load->line_position = strlen(line_buffer_his[his_cnt_sel-1]);
//             --his_cnt_sel;
//             his_cnt_sel = (his_cnt_sel<=0)?his_cnt:his_cnt_sel;

//             int num = load->line_position;
//             for (int i = 0; i < num; i++) {
//               uart_tx_one_char(line_buffer[i]);
//             }
//           }
//         } else if (ch == 0x42) { //down key
//           if (his_cnt > 0) {
//             for (int i = 0; i < load->line_position; i++) {
//               uart_tx_one_char(0x08);
//               uart_tx_one_char(' ');
//               uart_tx_one_char(0x08); 
//             }

//             memcpy(line_buffer, line_buffer_his[his_cnt_sel-1], LUA_MAXINPUT);
//             load->curr_position = strlen(line_buffer_his[his_cnt_sel-1]);
//             load->line_position = strlen(line_buffer_his[his_cnt_sel-1]);
//             his_cnt_sel++;
//             his_cnt_sel = (his_cnt_sel>his_cnt)?1:his_cnt_sel;
            
//             int num = load->line_position;
//             for (int i = 0; i < num; i++) {
//               uart_tx_one_char(line_buffer[i]);
//             }
//           }
//         }
//         is_key_button = 0;
//         continue;
//       }
//       is_key_button++;
//       continue;
//     }
	
//     if (ch == 0x7f || ch == 0x08) {
//       if (load->line_position > 0) {
//         uart_tx_one_char(0x08);
//         uart_tx_one_char(' ');
//         uart_tx_one_char(0x08);
//         load->line_position--;
// 		    load->curr_position--;
//         if (load->curr_position < load->line_position) { //handle case that delete character between a string
//           int num = load->line_position - load->curr_position;
//           for (int i = 0; i < num; i++) {
//             uart_tx_one_char(line_buffer[i+load->curr_position+1]);
//           }
//           uart_tx_one_char(' ');
//           for (int i = 0; i < num+1; i++) {
//             uart_tx_one_char('\b');
//           }
//           memmove(line_buffer+load->curr_position, line_buffer+load->curr_position+1, num); 
//         }
//       } 
//       line_buffer[load->line_position] = 0;
//       continue;
//     }
    
//     /* end of line */
//     if (ch == '\r' || ch == '\n') {
//       last_nl_char = ch;
//       line_buffer[load->line_position] = 0;
// 	    uart_sendStr("\r\n");
//       if (load->line_position == 0){
//         /* Get a empty line, then go to get a new line */
// 		    uart_sendStr(load->prmt);        
// 	      load->curr_position = 0;
//         continue;
//       } else {
// 		    load->done = 1;
//         need_dojob = true;

//         bool not_exist = true;
//         for (int i = 0; i < his_cnt; i++) {
//           if (0 == memcmp(line_buffer, line_buffer_his[i], LUA_MAXINPUT)) {
//             not_exist = false;
//             break;
//           }
//         }

//         if (not_exist) {
//           if (his_cnt == LUA_MAXINPUT_HIS) {
//             memmove(line_buffer_his, line_buffer_his[1], (LUA_MAXINPUT_HIS-1)*LUA_MAXINPUT); 
//             his_cnt --;
//           } 
          
//           memcpy(line_buffer_his[his_cnt], line_buffer, LUA_MAXINPUT);
//           his_cnt ++;
//           his_cnt_sel = his_cnt;
//         }
//         break;
//       }
// 	    // load->curr_position = 0;
//       // continue;
//     }

//     /* it's a large line, discard it */
//     if ( load->line_position + 1 >= LUA_MAXINPUT ){
//       load->line_position = 0;
// 	    load->curr_position = 0;
//     }
	
//     /* handle case that insert char between a string */
//     if (load->curr_position != load->line_position) {
//       uart_tx_one_char(ch);
//       int num = load->line_position - load->curr_position;
//       for (int i = 0; i < num; i++) {
//         uart_tx_one_char(line_buffer[load->curr_position+i]); 
//       }
//       for (int i = 0; i < num; i++) { //insert char, right move the rest string  
//         line_buffer[load->line_position-i] = line_buffer[load->line_position-1-i];
//         uart_tx_one_char('\b'); 
//       }
//       line_buffer[load->curr_position] = ch; 
//     } else {
//       uart_tx_one_char(ch); //add to tail directly
//       line_buffer[load->line_position] = ch;
//     }
    
//     load->line_position++;
// 	  load->curr_position++; 
//   }

//   return need_dojob;
// }

// static void __dojob(lua_Load *load) {
//   size_t l;
//   int status;
//   char *b = load->line;
//   lua_State *L = load->L;

//   const char *oldprogname = progname;
//   progname = NULL;

//   do{
//     if(load->done == 1){
//       l = strlen(b);
//       if (l > 0 && b[l-1] == '\n')  /* line ends with newline? */
//         b[l-1] = '\0';  /* remove it */
// 	  if (load->firstline && b[0] == '=') {  /* first line starts with `=' ? */
// 		lua_pushfstring(L, "return %s", b+1);  /* change it to `return' */
// 	  } else {
// 		lua_pushstring(L, b);
// 	  }
// 	  if(load->firstline != 1) {
//         lua_pushliteral(L, "\n");  /* add a new line... */
//         lua_insert(L, -2);  /* ...between the two lines */
//         lua_concat(L, 3);  /* join them */
//       }
	  
// 	  status = luaL_loadbuffer(L, lua_tostring(L, 1), lua_rawlen(L, 1), "=stdin");
// 	  if (!incomplete(L, status)) {  /* cannot try to add lines? */
//         lua_remove(L, 1);  /* remove line */
//         if (status == 0) {
//           status = docall(L, 0, 0);
//         }
//         report(L, status);
//         if (status == 0 && lua_gettop(L) > 0) {  /* any result to print? */
//           lua_getglobal(L, "print");
//           lua_insert(L, 1);
//           if (lua_pcall(L, lua_gettop(L)-1, 0, 0) != 0) {
//             l_message(progname, lua_pushfstring(L, "error calling " LUA_QL("print") " (%s)", lua_tostring(L, -1)));
// 		      }
//         }
//         load->firstline = 1;
//         load->prmt = get_prompt(L, 1);
//         lua_settop(L, 0);
//         /* force a complete garbage collection in case of errors */
//         if (status != 0) lua_gc(L, LUA_GCCOLLECT, 0);
//       } else { // not finish inputing, waiting for content
//         load->firstline = 0;
//         load->prmt = get_prompt(L, 0);
//         load->curr_position = load->line_position;
//         uart_sendStr(load->prmt);
//         return;
//       }
// 	  /*if (status) {
// 		uart_sendStr(lua_tostring(L, -1));
// 		lua_pop(L, 1);
// 	  }*/
//     }
//   }while(0);
  
//   load->done = 0;
//   load->line_position = 0;
//   load->curr_position = 0;
//   memset(load->line, 0, load->len);
//   // uart_sendStr("\r\n");
//   uart_sendStr(load->prmt);
// }

// int lua_handle_input(bool force)
// {
//   if (force || __readline(&gLoad)) {
//     __dojob (&gLoad);
//   }
// }

// static void __gload_init(lua_State *L)
// {
//     gLoad.L = L;
//     gLoad.firstline = 1;
//     gLoad.done = 0;
//     gLoad.line = line_buffer;
//     gLoad.len = LUA_MAXINPUT;
//     gLoad.line_position = 0;
//     gLoad.curr_position = 0;
//     gLoad.prmt = get_prompt(L, 1); 

// 	  // uart_init();
//     __dojob(&gLoad);
// }

#include "tal_mutex.h"

static MUTEX_HANDLE s_lua_main_mutex = NULL;
int lua_main (int argc, char **argv)
{
    int status, result;
    lua_State *L = luaL_newstate();  /* create state */
    if (L == NULL) {
      l_message(argv[0], "cannot create state: not enough memory");
      return EXIT_FAILURE;
    }

    // __gload_init(L);  /* int gload to handle uart input to Lua */

    lua_pushcfunction(L, &pmain);  /* to call 'pmain' in protected mode */
    lua_pushinteger(L, argc);  /* 1st argument, add 1 to account for the missing "lua" */
    lua_pushlightuserdata(L, argv); /* 2nd argument, shift argv back by 1 to simulate "lua" */
    status = lua_pcall(L, 2, 1, 0);  /* do the call */
   
    result = lua_toboolean(L, -1);  /* get result */
    report(L, status);
    lua_close(L);
    return (result && status == LUA_OK) ? EXIT_SUCCESS : EXIT_FAILURE;
}

void mtx_lock(void *mtx)
{
  if (mtx)
    tal_mutex_lock(mtx);
}

void mtx_unlock(void *mtx)
{
  if (mtx)
    tal_mutex_unlock(mtx);
}
int lua_run(char *luaname) {
    // 创建一个新的Lua状态
    lua_State *L = luaL_newstate();
    if (L == NULL) {
        l_message("lua_run","Failed to create Lua state.\n");
        return 1;
    }
    lua_setmutex(L,s_lua_main_mutex,mtx_lock,mtx_unlock);
    // 打开Lua标准库
    luaL_openlibs(L);

    // 加载脚本文件
    int status = luaL_loadfile(L, luaname);
    if (status != LUA_OK) {
        lua_writestringerror("Failed to load script: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }

    // 执行脚本
    status = lua_pcall(L, 0, LUA_MULTRET, 0);
    if (status != LUA_OK) {
        lua_writestringerror("Failed to execute script: %s\n", lua_tostring(L, -1));
        lua_close(L);
        return 1;
    }

    // 关闭Lua状态
    lua_close(L);
    return 0;
}

TKL_THREAD_HANDLE lua_threadhandle = NULL;
void luaTask(void *pvParameters) {

    char  *luafile = pvParameters;
    PR_DEBUG("luaTask= %s",luafile);
    lua_run(luafile);
    PR_DEBUG("lua_run end");
    tkl_thread_release(lua_threadhandle);
}
int lua_task_start(char *luafile,int stack_size)
{

    if (s_lua_main_mutex == NULL) {
      tal_mutex_create_init(&s_lua_main_mutex);
    }

    PR_DEBUG("lua_task_start");
    lua_msgbus_init();


    OPERATE_RET ret = tkl_thread_create(&lua_threadhandle, "lua_task", stack_size, 10,
                            luaTask, luafile);
    if (ret != OPRT_OK) {
        PR_ERR("tkl_thread_create lua_task failed ret:%d",ret);
    }
    return ret;
}

extern char custom_search_paths[4][128];

int lua_set_script_path(char *path,int index)
{
    if (index < 0 || index >= 4 || path == NULL) {
        PR_ERR("index %d is invalid",index);
        return OPRT_INVALID_PARM;
    }
    strncpy(custom_search_paths[index],path,127);
    PR_DEBUG("custom_search_paths[%d]=%s",index,custom_search_paths[index]);
    return 0;
}


#include "exec/exec.h"
#include "dos/dos.h"
#include "intuition/intuition.h"
#include "intuition/intuitionbase.h"
#include "intuition/screens.h"
#include "libraries/mathieeedp.h"
#include "libraries/gadtools.h"
#include "libraries/iffparse.h"
#include "datatypes/textclass.h"
#include "workbench/startup.h"
#include <libraries/commodities.h>
#include <libraries/locale.h>
#include <utility/tagitem.h>
#include <proto/mathieeedoubbas.h>
#include <proto/mathieeedoubtrans.h>
#include <stdio.h> /* For sprintf */
#include <stdlib.h> /* For rand() */

#include "proto/exec.h"
#include "proto/dos.h"
#include "proto/intuition.h"
#include "proto/graphics.h"
#include "proto/gadtools.h"
#include "proto/iffparse.h"
#include "proto/wb.h"
#include "proto/icon.h"
#include "proto/mathieeedoubbas.h"
#include "proto/mathieeedoubtrans.h"
#include "proto/commodities.h"
#include "proto/utility.h"

#include "scicalc_rev.h"

#include <string.h>
extern int atoi(const char *);
extern double atof(const char *);

/* Add proper Amiga prototypes for SPrintf and StrToLong */
#include <clib/alib_protos.h>  /* For SPrintf() */
#include <proto/dos.h>         /* For StrToLong() */

#ifndef INFINITY
#define INFINITY (1.7976931348623157e+308) /* Maximum double value */
#endif

#ifndef NAN
#define NAN (0.0/0.0) /* Not a number */
#endif

/* PI is already defined in mathieeedp.h - no need to redefine */
/* #define PI 3.141592653589793 */

/* Stats define */
#define STATS_SIZE 100

/* Additional operator defines */
#define BIT_AND (57L)

/* Operator Defines */
#define BACKSPACE (16L)
#define CE (17L)
#define CA (18L)
#define POINT (19L)
#define EXPONENT (20L)
#define BRACKET (21L)
#define END_BRACKET (22L)
#define ADD (22L) /* Brackets used to ensure references are resolved properly */
#define SUB (23L)
#define MUL (24L)
#define DIV (25L)
#define MOD (26L)
#define NEG (27L)
#define EQU (28L)
#define nPr (29L)
#define nCr (30L)
#define POW (31L)
#define INVPOW (32L)

#define LN (33L)
#define EXP (34L)
#define LOGBASE10 (35L)
#define a10X (36L) /* Log of any number eventually */

#define SQR (37L)
#define SQRT (38L)

#define SIN (39L)
#define ASIN (40L)
#define SINH (41L)
#define COS (42L)
#define ACOS (43L)
#define COSH (44L)
#define TAN (45L)
#define ATAN (46L)
#define TANH (47L)

#define RECIPROCAL (48L)
#define FACTORIAL (49L)
#define FIX (50L)
#define RANDOM (51L)
#define CONSTANT (52L)

#define DISPLAY_GAD (100L)
#define SHIFT_GAD (101L)
#define HYP_GAD (102L)

#define MIN (53L)
#define MPLUS (54L)
#define MMINUS (55L)
#define MR (56L)

/* Shift and other binary logical operators */
/* Support % and DMS */
/* Show list of mem slots with preview of values/attach name to memory slot */
/* Can combine with constant list */
/* Fractions using _ */


/* Precedence Defines */
#define PRECBASE 0L
#define PREC_EQUAL   (PRECBASE+1)
#define PREC_ADDSUB  (PRECBASE+2)
#define PREC_MULDIVMOD  (PRECBASE+3)
#define PREC_ORDER   (PRECBASE+4)
#define PREC_SCI  (PRECBASE+5)
#define PREC_BRACKET (PRECBASE+6)


/* Base Defines */
#define BASE2 (2)   /* Perhaps also define BINARY, OCTOL etc. */
#define BASE8 (8)
#define BASE10 (10)
#define BASE16 (16)


/* Trigonometry Defines */
/* Numbering is started at 20 so that it doesn't
** interfere with other menu options 
*/
#define DEG (20)
#define RAD (21)
#define GRAD (22)


/* Menu Defines */
#define MENU_CE 1
#define MENU_CA 3
#define MENU_ABOUT 4
#define MENU_QUIT 5
#define MENU_CUT 6
#define MENU_COPY 7
#define MENU_PASTE 9
#define MENU_DECIMAL BASE10
#define MENU_BINARY BASE2
#define MENU_OCTOL BASE8
#define MENU_HEXADECIMAL BASE16
#define MENU_DEGREES DEG
#define MENU_RADIANS RAD
#define MENU_GRADS GRAD
#define MENU_TAPE 11
#define MENU_SHOWHIDE 12
#define MENU_CONVERT 13
#define MENU_UNDO 14
#define MENU_REDO 15
#define MENU_WINDOW_VIS 75  // Added a unique ID for the window visibility menu item

/* #define DEBUG */


/* Stack Defines */
#define STACK_EMPTY 1
#define STACK_FULL  2
#define STACK_OK    0
#define STACKSIZE 4096

/* Data Type Declarations */
struct Operator
{
   APTR op_Prec;
   UWORD op_Type;
};

/* Incorporates the "Version String" into the program so that the 
** OS can tell one version of the program from another more recent one */
STRPTR versionstring=VERSTAG;


/* Function Declarations */
int   main(int argc, char **argv);
void  chkabort(void) { return; } /* Allows us to handle Break signals ourselves */

VOID  calculator(STRPTR,STRPTR,ULONG);
VOID  about(VOID);

VOID  buttonpressed(BOOL);
VOID  UpdateDisplay(STRPTR);
DOUBLE   DoSum(DOUBLE,DOUBLE,UWORD);
STRPTR   ConvertToText(DOUBLE,STRPTR);
DOUBLE   ConvertToValue(STRPTR);

VOID  push(struct Operator);
struct Operator *pull(VOID);
VOID  val_push(DOUBLE);
DOUBLE   val_pull(VOID);

VOID  eval(VOID);
VOID  sci_eval(struct Operator);
VOID  output_operator(UWORD);
VOID  clear_entry(VOID);
VOID  clear_all(VOID);

DOUBLE   degrees_rads(DOUBLE);
DOUBLE   rads_degrees(DOUBLE);
DOUBLE   triginit(DOUBLE *);
DOUBLE   invtriginit(DOUBLE *);

ULONG choose_mem_slot(VOID);
VOID  MIn(DOUBLE); /* MC not needed */
VOID  MPlus(DOUBLE);
VOID  MMinus(DOUBLE);
DOUBLE   MRecall(VOID);

LONG  factorial(LONG);
LONG  combination(LONG,LONG);
LONG  permutation(LONG,LONG);

VOID  error(STRPTR);
VOID  notify_error(STRPTR);

VOID  copy(VOID);
VOID  paste(VOID);

VOID  pushitem(VOID);
VOID  reset_checkboxes(VOID);
VOID  exponent(VOID);
VOID  equals(VOID);
VOID  operator_2(UWORD,APTR);
VOID  display_digit(UBYTE);
VOID  backspace(VOID);
VOID  point(VOID);

VOID solve_equation(UWORD type);

// Commodities prototypes
UWORD ParseKey(STRPTR string, STRPTR *nextptr);
VOID CxObjectType(CxObj *obj, LONG type);
VOID AttachCxObj(CxObj *headObj, CxObj *co);
VOID AttachKeyCode(CxObj *filterObj, UBYTE key, UBYTE qualifier);

// Toggle window visibility prototype
VOID toggle_window_visibility(VOID);

// Additional function prototypes
BOOL WinClosed(struct Window *window);

// Prototypes for standard C functions
extern int printf(const char *format, ...);

// Prototypes for custom functions
VOID cleanup_commodities(VOID);
VOID SetTimer(struct MsgPort *port, LONG interval, BOOL trigger);
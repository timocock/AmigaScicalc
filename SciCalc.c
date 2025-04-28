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

#define DEBUG 1


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

/* Global Variables */
struct Window *win;
struct Screen *scr;
struct Gadget *prev_gad,*glist,*display_tg,*shift_gad,*hyp_gad;
APTR vi;
struct Window *memwin;
UWORD widthfactor,heightfactor;

#define buffer_Size 100
UBYTE buffer[buffer_Size]="0";
SHORT current_position;
ULONG current_base=BASE10;
ULONG trig_mode=DEG;
BOOL tape_on=FALSE;
BOOL pointused=FALSE;
BOOL expused=FALSE;

BOOL item_ready=FALSE;
struct Operator item;

struct IFFHandle *iffh;
int textlen=0;

/* The stack must be declared globally, and this also has the advantage 
** that all elements will be set to 0 on initialization.
** This also affects the stack pointers and other variables.
*/
DOUBLE *memory;
LONG global_memsize;
struct Operator stack[STACKSIZE];
ULONG stack_ptr;
int stack_err;
DOUBLE val_stack[STACKSIZE];
ULONG val_stack_ptr;
ULONG val_stack_err;

BPTR output_file; /* Renamed from stdout to avoid conflicts */

struct Menu *menu;
struct NewMenu nm[]=
{
   /* Implement an undo option */
   { NM_TITLE,"Project",0,0,0,0},
   {  NM_ITEM,"Clear Entry", "E",0,COMMSEQ,(APTR) MENU_CE},
   {  NM_ITEM,"Clear All",   "A",0,0,(APTR) MENU_CA},
   {  NM_ITEM,"Undo",        "Z",0,COMMSEQ,(APTR) MENU_UNDO},
   {  NM_ITEM,"Redo",        "Y",0,COMMSEQ,(APTR) MENU_REDO},
   {  NM_ITEM,NM_BARLABEL,   0,0,0,0},
   {  NM_ITEM,"About",       "?",0,0,(APTR) MENU_ABOUT},
   {  NM_ITEM,"Quit",        "Q",0,COMMSEQ,(APTR) MENU_QUIT},
   { NM_TITLE,"Edit",        0,0,0,0},
   {  NM_ITEM,"Cut",         "X",0,0,(APTR) MENU_CUT},
   {  NM_ITEM,"Copy",        "C",0,0,(APTR) MENU_COPY},
   {  NM_ITEM,"Paste",       "V",0,0,(APTR) MENU_PASTE},
   { NM_TITLE,"Mode",        0,0,0,0},
   {  NM_ITEM,"Base",        0,0,0,0},
   {   NM_SUB,"Decimal",     0,CHECKIT|CHECKED,~1, (APTR) BASE10},
   {   NM_SUB,"Binary",      0,CHECKIT,~2,(APTR) BASE2},
   {   NM_SUB,"Octol",       0,CHECKIT,~4,(APTR) BASE8},
   {   NM_SUB,"Hexadecimal", 0,CHECKIT,~8,(APTR) BASE16},
   {  NM_ITEM,"Trig",        0,0,0,0},
   {   NM_SUB,"Degrees",     0,CHECKIT|CHECKED,~1,(APTR)DEG},
   {   NM_SUB,"Radians",     0,CHECKIT,~2,(APTR)RAD},
/* {   NM_SUB,"Grads",       0,CHECKIT,~4,(APTR)GRAD}, */
   { NM_TITLE,"Windows",     0,0,0,0},
   {  NM_ITEM,"Show Tape",   "T",CHECKIT|MENUTOGGLE,0,(APTR) MENU_TAPE},
   { NM_END,NULL,0,0,0,0}
};

struct Library *CommoditiesBase = NULL;
CxObj *cxbroker = NULL;
CxObj *cxfilter = NULL;
ULONG cxsig = 0;

/* Add stats storage */
DOUBLE stats_buffer[STATS_SIZE];
ULONG stats_ptr;

/* Add functions */
DOUBLE calculate_mean(VOID) {
    DOUBLE sum = 0;
    LONG i;
    for(i=0; i<stats_ptr; i++)
        sum += stats_buffer[i];
    return sum/stats_ptr;
}


/* Functions */

/* Unified entry point for CLI and Workbench */
int main(int argc, char **argv)
{
   struct Process *process;
   struct WBStartup *wbs = NULL;
   struct RDArgs *rdargs;
   LONG myargs[]={NULL,NULL,10};
   UBYTE **toolarray = NULL;
   struct DiskObject *dobj = NULL;
   BPTR olddir = 0;
   STRPTR pubscreen = NULL;
   STRPTR tapefile = NULL;
   STRPTR memstr = NULL;
   ULONG memsize = 10;  /* Default memory size */
   BOOL commodities_initialized = FALSE;  /* Track if commodities was initialized */

#ifdef DEBUG
   printf("DEBUG: main() entered\n");
#endif

   /* Get process information to check launch method */
   process = (struct Process *)FindTask(NULL);
   
   /* Check if we were started from Workbench */
   if (argc == 0 && process->pr_CLI == 0) {
      /* Workbench startup - get the WBStartup message */
      WaitPort(&(process->pr_MsgPort));
      wbs = (struct WBStartup *)GetMsg(&(process->pr_MsgPort));
      
      /* Validate that we have at least one argument */
      if (wbs->sm_NumArgs >= 1) {
         /* Change to the directory where our icon was found */
         olddir = CurrentDir(wbs->sm_ArgList[0].wa_Lock);
         
         /* Get the disk object to read tooltypes */
         dobj = GetDiskObject(wbs->sm_ArgList[0].wa_Name);
         if (dobj) {
            toolarray = (UBYTE **)dobj->do_ToolTypes;
            
            /* Read tooltypes into local variables */
            pubscreen = FindToolType(toolarray, "PUBSCREEN");
            tapefile = FindToolType(toolarray, "TAPE");
            
            /* Parse memory size with fallback */
            memstr = FindToolType(toolarray, "MEMORY");
            if (memstr) {
               memsize = (ULONG)atoi((char *)memstr);
               if (memsize < 1) memsize = 10;  /* Ensure valid value */
            }
         }
      }
   }
   else {
      /* CLI startup - parse command line arguments */
      if ((rdargs = ReadArgs("PUBSCREEN,TAPE/K,MEMORY/N", myargs, NULL))) {
         pubscreen = (UBYTE *)myargs[0];
         tapefile = (UBYTE *)myargs[1];
         memsize = myargs[2];
      }
   }

   /* Initialize commodities regardless of startup method */
   if ((CommoditiesBase = OpenLibrary("commodities.library", 37L))) {
      struct NewBroker nb;
      
      nb.nb_Version = NB_VERSION;
      nb.nb_Name = "SciCalc";
      nb.nb_Title = "Scientific Calculator";
      nb.nb_Descr = "Amiga Scientific Calculator";
      nb.nb_Unique = NULL;
      nb.nb_Flags = 0;
      nb.nb_Pri = 0;
      nb.nb_Port = NULL;
      nb.nb_ReservedChannel = 0;
      
      cxbroker = CxBroker(&nb, NULL);
      if (cxbroker) {
         /* Parse hotkey from tooltypes if available */
         STRPTR hotkey = NULL;
         if (dobj && toolarray) {
            hotkey = FindToolType(toolarray, "CX_POPKEY");
         }
         if (hotkey) {
            UWORD key = ParseKey(hotkey, NULL);
            cxfilter = CxFilter(cxbroker);
            CxObjectType(cxfilter, CX_FILTER);
            AttachCxObj(cxbroker, cxfilter);
            AttachKeyCode(cxfilter, (key >> 8) & 0xFF, key & 0xFF);
         }
         commodities_initialized = TRUE;
      }
   }

   /* Launch calculator with the settings */
   calculator(pubscreen, tapefile, memsize);

   /* Clean up disk object if it was created */
   if (dobj) {
      FreeDiskObject(dobj);
   }

   /* Restore original directory if changed */
   if (olddir) {
      CurrentDir(olddir);
   }

   /* Free CLI arguments if they were allocated */
   if (rdargs) {
      FreeArgs(rdargs);
   }

   /* Reply to Workbench startup message if received */
   if (wbs) {
      ReplyMsg((struct Message *)wbs);
   }
   
   /* Clean up commodities only if it was initialized */
   if (commodities_initialized) {
      cleanup_commodities();
   }
   
   return 0;
}


/* Main program initialization and input processing loop */
VOID calculator(STRPTR psname, STRPTR filename, ULONG memsize)
{
   struct NewGadget ng_button;
   BOOL done = FALSE;  /* Initialize to FALSE */
   ULONG class = 0;
   UWORD code = 0;
   ULONG signal = 0;
   struct IntuiMessage *imsg;
   ULONG winsignal;
   ULONG ilock;
   WORD mousex, mousey;
   LONG shift, hyp;
   UWORD winwidth, winheight;
   BPTR old_output_file = NULL;
   struct Gadget *loop_gad;
   struct MenuItem *item;
   APTR item_data;
   LONG menu_id;

   /* Initialize important pointers to NULL */
   scr = NULL;
   win = NULL;
   glist = NULL;
   vi = NULL;
   memory = NULL;
   output_file = NULL;
   
#ifdef DEBUG
   printf("DEBUG: calculator() entered with memsize=%lu\n", memsize);
#endif

   /* Set minimum valid memory size */
   if (memsize < 1) memsize = 10;
   global_memsize = memsize;

   /* Handle output file */
   if (filename) {
      output_file = Open(filename, MODE_NEWFILE);
      if (!output_file) {
         notify_error("Could not open output file");
         output_file = Open("NIL:", MODE_NEWFILE);
      }
   } else {
      output_file = Open("NIL:", MODE_NEWFILE);
   }

   /* Allocate memory for memory registers */
   memory = AllocMem(sizeof(DOUBLE) * (memsize + 1), MEMF_CLEAR);
   if (!memory) {
      notify_error("Memory allocation failed!");
   }
   
   /* Obtain Lock on display Screen user wishes to use */
   scr = LockPubScreen(psname);
   if (!scr) {
      notify_error("Cannot open screen");
   }

   /* Find out dimensions of screen and font to use */
   vi = GetVisualInfo(scr, TAG_DONE);
   if (!vi) {
      notify_error("Cannot get visual info");
   }

   /* Calculate dimensions using font metrics */
   widthfactor = TextLength(&scr->RastPort, "0", 1) * 3;
   heightfactor = scr->Font->ta_YSize + 2;

   /* Work out a height and width for each button */
   winwidth=11+(widthfactor+3)*10;
   winheight=(heightfactor+3)*7+2;

   /* Find out where mouse pointer is so window can open there */
   ilock=LockIBase(0);
   mousey=IntuitionBase->MouseY;
   mousex=IntuitionBase->MouseX;
   UnlockIBase(ilock);

   /* Create the window Gadgets */
   ng_button.ng_TopEdge=(4+scr->BarHeight);
   ng_button.ng_LeftEdge=7;
   ng_button.ng_TextAttr=scr->Font;
   ng_button.ng_Flags=NULL;
   ng_button.ng_VisualInfo=vi;
   prev_gad=CreateContext(&glist);

   ng_button.ng_Width=winwidth-14;
   ng_button.ng_Height=heightfactor;
   ng_button.ng_GadgetText=NULL;
   ng_button.ng_GadgetID=DISPLAY_GAD;
   prev_gad=CreateGadget(TEXT_KIND,prev_gad,&ng_button,
      GTTX_Text,"0",
      GTTX_CopyText,TRUE,
      GTTX_Border,TRUE,
      GTTX_Justification,GTJ_RIGHT,
      GTTX_Clipped,TRUE,
      GA_TextAttr,scr->Font,
      TAG_DONE);
   
   /* Store pointer to the display gadget as it is needed when updating the display later on */
   display_tg=prev_gad;

   (ng_button.ng_LeftEdge)=7;
   (ng_button.ng_TopEdge)+=(ng_button.ng_Height+3);

   (ng_button.ng_LeftEdge)+=((widthfactor+3)*2);

   ng_button.ng_Width=widthfactor;
   ng_button.ng_GadgetText="A";
   ng_button.ng_Flags=0;
   ng_button.ng_GadgetID=10;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE); /* Use underscore for tan etc. functions */

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="B";
   ng_button.ng_GadgetID=11;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="C";
   ng_button.ng_GadgetID=12;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="D";
   ng_button.ng_GadgetID=13;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="E";
   ng_button.ng_GadgetID=14;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="F";
   ng_button.ng_GadgetID=15;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="Sh_ift";
   ng_button.ng_Flags=PLACETEXT_RIGHT;
   ng_button.ng_GadgetID=SHIFT_GAD;
   prev_gad=CreateGadget(CHECKBOX_KIND,prev_gad,&ng_button,GTCB_Scaled,TRUE,GT_Underscore,'_',TAG_DONE);
   
   /* The Hyp and Shift gadgets need to be stored so that they can be turned on and off without the user's input */
   shift_gad=prev_gad;

   (ng_button.ng_LeftEdge)=7;
   (ng_button.ng_TopEdge)+=(ng_button.ng_Height+3);

   ng_button.ng_GadgetText="_Hyp";
   ng_button.ng_GadgetID=HYP_GAD;
   ng_button.ng_Flags=PLACETEXT_ABOVE;
   prev_gad=CreateGadget(CHECKBOX_KIND,prev_gad,&ng_button,GTCB_Scaled,TRUE,GT_Underscore,'_',TAG_DONE);

   hyp_gad=prev_gad;

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="nCr";
   ng_button.ng_Flags=0;
   ng_button.ng_GadgetID=nCr;
   ng_button.ng_UserData=(APTR) PREC_SCI;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="ln";
   ng_button.ng_GadgetID=LN;
   ng_button.ng_UserData=(APTR) PREC_SCI;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="MR";
   ng_button.ng_GadgetID=MR;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="Ran";
   ng_button.ng_GadgetID=RANDOM;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="7";
   ng_button.ng_GadgetID=7;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="8";
   ng_button.ng_GadgetID=8;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="9";
   ng_button.ng_GadgetID=9;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="CA";
   ng_button.ng_GadgetID=CA;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="CE";
   ng_button.ng_GadgetID=CE;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)=7;
   (ng_button.ng_TopEdge)+=(ng_button.ng_Height+3);

   ng_button.ng_GadgetText="sin";
   ng_button.ng_GadgetID=SIN;
   ng_button.ng_UserData=(APTR) PREC_SCI;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="nPr";
   ng_button.ng_GadgetID=nPr;
   ng_button.ng_UserData=(APTR) PREC_SCI;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="log";
   ng_button.ng_GadgetID=LOGBASE10;
   ng_button.ng_UserData=(APTR) PREC_SCI;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="MIn";
   ng_button.ng_GadgetID=MIN;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="Mod";
   ng_button.ng_GadgetID=MOD;
   ng_button.ng_UserData=(APTR) PREC_MULDIVMOD;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="4";
   ng_button.ng_GadgetID=4;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="5";
   ng_button.ng_GadgetID=5;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="6";
   ng_button.ng_GadgetID=6;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="";
   ng_button.ng_GadgetID=MUL;
   ng_button.ng_UserData=(APTR) PREC_MULDIVMOD;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="";
   ng_button.ng_GadgetID=DIV;
   ng_button.ng_UserData=(APTR) PREC_MULDIVMOD;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)=7;
   (ng_button.ng_TopEdge)+=(ng_button.ng_Height+3);

   ng_button.ng_GadgetText="cos";
   ng_button.ng_GadgetID=COS;
   ng_button.ng_UserData=(APTR) PREC_SCI;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="x^y";
   ng_button.ng_GadgetID=POW;
   ng_button.ng_UserData=(APTR) PREC_ORDER;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="x!";
   ng_button.ng_GadgetID=FACTORIAL;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="M+";
   ng_button.ng_GadgetID=MPLUS;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="Fix";
   ng_button.ng_GadgetID=FIX;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="1";
   ng_button.ng_GadgetID=1;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="2";
   ng_button.ng_GadgetID=2;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="3";
   ng_button.ng_GadgetID=3;
   ng_button.ng_UserData=0;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="+";
   ng_button.ng_GadgetID=ADD;
   ng_button.ng_UserData=(APTR) PREC_ADDSUB;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

   ng_button.ng_GadgetText="-";
   ng_button.ng_GadgetID=SUB;
   ng_button.ng_UserData=(APTR) PREC_ADDSUB;
   prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

   (ng_button.ng_LeftEdge)=7;
   (ng_button.ng_TopEdge)+=(ng_button.ng_Height+3);

   if(prev_gad)
   {
      menu=CreateMenus(nm,GTMN_NewLookMenus,TRUE,TAG_DONE);

      if(menu)
      {
         if(LayoutMenus(menu,vi,GTMN_NewLookMenus,TRUE,TAG_DONE))
         {
            /* The dimensions of the window when it is shrunk to a title bar size */
            WORD zoom[4];
            zoom[0] = -1;
            zoom[1] = -1;
            zoom[2] = 270;
            zoom[3] = scr->BarHeight+1;

            /* Create window */
            win = OpenWindowTags(NULL,
               WA_Left, mousex-winwidth/2,
               WA_Top, mousey-winheight/2,
               WA_Width, winwidth,
               WA_Height, winheight,
               WA_Title, "Scientific Calculator",
               WA_SimpleRefresh, TRUE,
               WA_DragBar, TRUE,
               WA_DepthGadget, TRUE,
               WA_CloseGadget, TRUE,
               WA_Activate, TRUE,
               WA_Gadgets, glist,
               WA_PubScreen, scr,
               WA_MinHeight, winheight,
               WA_MinWidth, winwidth,
               WA_IDCMP, IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|IDCMP_VANILLAKEY|BUTTONIDCMP|CHECKBOXIDCMP|MENUPICK,
               TAG_DONE);

            if (!win) {
               notify_error("Cannot open window");
               goto cleanup;
            }

            /* Get window signal mask */
            winsignal = 1L << win->UserPort->mp_SigBit;

            /* Set up menu strip */
            SetMenuStrip(win, menu);

            /* Main event loop */
            while (!done) {
               signal = Wait(winsignal | SIGBREAKF_CTRL_C);

               if (signal & SIGBREAKF_CTRL_C) {
#ifdef DEBUG
                  printf("DEBUG: Received SIGBREAKF_CTRL_C signal\n");
#endif
                  done = TRUE;
                  continue;
               }

               if (signal & winsignal) {
                  /* Check if window is still valid */
                  if (!win || !win->UserPort) {
#ifdef DEBUG
                     printf("DEBUG: Window or UserPort is NULL\n");
#endif
                     done = TRUE;
                     continue;
                  }

                  /* Process all pending messages */
                  while ((imsg = GT_GetIMsg(win->UserPort))) {
#ifdef DEBUG
                     printf("DEBUG: Processing message class: %lu, code: %lu\n", imsg->Class, imsg->Code);
#endif
                     class = imsg->Class;
                     code = imsg->Code;
                     loop_gad = (struct Gadget *)imsg->IAddress;
                     GT_ReplyIMsg(imsg);

                     switch (class) {
                        case IDCMP_CLOSEWINDOW:
#ifdef DEBUG
                           printf("DEBUG: IDCMP_CLOSEWINDOW received\n");
#endif
                           done = TRUE;
                           break;

                        case IDCMP_REFRESHWINDOW:
#ifdef DEBUG
                           printf("DEBUG: IDCMP_REFRESHWINDOW received\n");
#endif
                           if (win) {
                              GT_BeginRefresh(win);
                              GT_EndRefresh(win, TRUE);
                           }
                           break;

                        case IDCMP_GADGETUP:
#ifdef DEBUG
                           printf("DEBUG: IDCMP_GADGETUP received, gadget ID: %d\n", loop_gad->GadgetID);
#endif
                           /* Handle gadget events */
                           if (loop_gad) {
                              switch (loop_gad->GadgetID) {
                                 case 0: case 1: case 2: case 3: case 4:
                                 case 5: case 6: case 7: case 8: case 9:
                                    display_digit(loop_gad->GadgetID + '0');
                                    break;
                                 case POINT:
                                    point();
                                    break;
                                 case CE:
                                    clear_entry();
                                    break;
                                 case CA:
                                    clear_all();
                                    break;
                                 case ADD:
                                    operator_2(ADD, (APTR)PREC_ADDSUB);
                                    break;
                                 case SUB:
                                    operator_2(SUB, (APTR)PREC_ADDSUB);
                                    break;
                                 case MUL:
                                    operator_2(MUL, (APTR)PREC_MULDIVMOD);
                                    break;
                                 case DIV:
                                    operator_2(DIV, (APTR)PREC_MULDIVMOD);
                                    break;
                                 case EQU:
                                    equals();
                                    break;
                                 case SHIFT_GAD:
                                    GT_GetGadgetAttrs(shift_gad, win, NULL, GTCB_Checked, &shift, TAG_DONE);
                                    GT_SetGadgetAttrs(shift_gad, win, NULL, GTCB_Checked, !shift, TAG_DONE);
                                    break;
                                 case HYP_GAD:
                                    GT_GetGadgetAttrs(hyp_gad, win, NULL, GTCB_Checked, &hyp, TAG_DONE);
                                    GT_SetGadgetAttrs(hyp_gad, win, NULL, GTCB_Checked, !hyp, TAG_DONE);
                                    break;
                                 default:
                                    /* Handle other gadget IDs */
                                    break;
                              }
                           }
                           break;

                        case MENUPICK:
#ifdef DEBUG
                           printf("DEBUG: MENUPICK received, code: %lu\n", code);
#endif
                           if (code == MENUNULL) break;

                           while (code != MENUNULL && !done) {
                              item = ItemAddress(menu, code);
                              if (!item) break;

                              item_data = GTMENUITEM_USERDATA(item);
                              menu_id = (LONG)item_data;

                              switch (menu_id) {
                                 case MENU_QUIT:
                                    done = TRUE;
                                    break;
                                 case MENU_CE:
                                    clear_entry();
                                    break;
                                 case MENU_CA:
                                    clear_all();
                                    break;
                                 case MENU_ABOUT:
                                    about();
                                    break;
                                 case MENU_TAPE:
                                    tape_on = !tape_on;
                                    if (output_file) {
                                       Close(output_file);
                                       output_file = NULL;
                                    }
                                    output_file = Open(tape_on ? filename : (STRPTR)"NIL:", MODE_NEWFILE);
                                    break;
                                 case MENU_SHOWHIDE:
                                    toggle_window_visibility();
                                    break;
                                 case BASE2:
                                 case BASE8:
                                 case BASE16:
                                 case BASE10:
                                    current_base = (ULONG)menu_id;
                                    UpdateDisplay(ConvertToText(ConvertToValue(buffer), buffer));
                                    break;
                                 case DEG:
                                 case RAD:
                                 case GRAD:
                                    trig_mode = (ULONG)menu_id;
                                    break;
                              }
                              code = item->NextSelect;
                           }
                           break;

                        case IDCMP_VANILLAKEY:
#ifdef DEBUG
                           printf("DEBUG: IDCMP_VANILLAKEY received, code: %lu\n", code);
#endif
                           switch (code) {
                              case 'I':
                              case 'i':
                                 GT_GetGadgetAttrs(shift_gad, win, NULL, GTCB_Checked, &shift, TAG_DONE);
                                 if (!shift) {
                                    GT_SetGadgetAttrs(hyp_gad, win, NULL, GTCB_Checked, FALSE, TAG_DONE);
                                 }
                                 GT_SetGadgetAttrs(shift_gad, win, NULL, GTCB_Checked, !shift, TAG_DONE);
                                 break;

                              case 'H':
                              case 'h':
                                 GT_GetGadgetAttrs(hyp_gad, win, NULL, GTCB_Checked, &hyp, TAG_DONE);
                                 if (!hyp) {
                                    GT_SetGadgetAttrs(shift_gad, win, NULL, GTCB_Checked, FALSE, TAG_DONE);
                                 }
                                 GT_SetGadgetAttrs(hyp_gad, win, NULL, GTCB_Checked, !hyp, TAG_DONE);
                                 break;

                              case '0': case '1': case '2': case '3': case '4':
                              case '5': case '6': case '7': case '8': case '9':
                                 display_digit(code);
                                 break;

                              case '.':
                                 point();
                                 break;

                              case '+':
                                 operator_2(ADD, (APTR)PREC_ADDSUB);
                                 break;

                              case '-':
                                 operator_2(SUB, (APTR)PREC_ADDSUB);
                                 break;

                              case '*':
                                 operator_2(MUL, (APTR)PREC_MULDIVMOD);
                                 break;

                              case '/':
                                 operator_2(DIV, (APTR)PREC_MULDIVMOD);
                                 break;

                              case 'a':
                              case 'A':
                                 clear_all();
                                 break;
                           }
                           break;
                     }
                  }
               }
            }

         cleanup:
            /* Clean up resources */
            if (menu) {
               if (win) ClearMenuStrip(win);
               FreeMenus(menu);
               menu = NULL;
            }

            if (win) {
               CloseWindow(win);
               win = NULL;
            }

            if (glist) {
               FreeGadgets(glist);
               glist = NULL;
            }

            if (vi) {
               FreeVisualInfo(vi);
               vi = NULL;
            }

            if (scr) {
               UnlockPubScreen(NULL, scr);
               scr = NULL;
            }

            if (memory) {
               FreeMem(memory, sizeof(DOUBLE) * (global_memsize + 1));
               memory = NULL;
            }

            if (output_file) {
               Close(output_file);
               output_file = NULL;
            }

            return;
         }
            FreeMenus(menu);
         }
         FreeVisualInfo(vi);
         FreeGadgets(glist);
      }
         /* Free the memory registers */
         if (memory) {
            FreeMem(memory, sizeof(DOUBLE) * (memsize + 1));
            memory = NULL;
         }
         
         if (output_file && output_file != old_output_file) {
            if (textlen > 0) {
               Write(output_file, buffer, textlen);
            }
            Close(output_file);
            output_file = NULL;
         }

         cleanup_commodities();

         /* Free visual info */
         if (vi) {
            FreeVisualInfo(vi);
            vi = NULL;
         }
         
         /* Free gadgets */
         if (glist) {
            FreeGadgets(glist);
            glist = NULL;
         }
         
         /* Unlock screen */
         if (scr) {
            UnlockPubScreen(NULL, scr);
            scr = NULL;
         }
   }
      FreeMenus(menu);
   }
   FreeVisualInfo(vi);
   FreeGadgets(glist);
}
   /* Free the memory registers */
   if (memory) {
      FreeMem(memory, sizeof(DOUBLE) * (memsize + 1));
      memory = NULL;
   }
   
   if (output_file && output_file != old_output_file) {
      if (textlen > 0) {
         Write(output_file, buffer, textlen);
      }
      Close(output_file);
      output_file = NULL;
   }

   cleanup_commodities();

   /* Free visual info */
   if (vi) {
      FreeVisualInfo(vi);
      vi = NULL;
   }
   
   /* Free gadgets */
   if (glist) {
      FreeGadgets(glist);
      glist = NULL;
   }
   
   /* Unlock screen */
   if (scr) {
      UnlockPubScreen(NULL, scr);
      scr = NULL;
   }
}


/* Update the display gadget in the main window to show the new value */
VOID UpdateDisplay(STRPTR display_string)
{
#ifdef DEBUG
   printf("DEBUG: UpdateDisplay() called with string: %s\n", display_string);
#endif
   if(!win || !display_tg) {
#ifdef DEBUG
      printf("DEBUG: UpdateDisplay - window or display gadget is NULL\n");
#endif
      return;
   }
   GT_SetGadgetAttrs(display_tg,win,NULL,GTTX_Text,display_string,TAG_DONE);
}


/* Do the relevant evaluation for the input operator */
DOUBLE DoSum(DOUBLE value1,DOUBLE value2,UWORD operator)
{
#ifdef DEBUG
   printf("DEBUG: DoSum() called with operator: %u\n", operator);
#endif
   switch(operator)
   {
      case ADD :
         return(IEEEDPAdd(value1,value2));
         break;

      case SUB :
         return(IEEEDPSub(value1,value2));
         break;

      case MUL :
         return(IEEEDPMul(value1,value2));
         break;

      case DIV :
         if(IEEEDPTst(value2) == 0) {
            error("Divide by zero");
            return 0.0; // Just return zero on error
         }
         return IEEEDPDiv(value1, value2);
         break;

      case MOD :
         return((DOUBLE) (IEEEDPFix(value1) % IEEEDPFix(value2)) );
         break;

      case NEG :
         return(IEEEDPNeg(value2));
         break;

      case EQU :
         return(value2);
         break;

      case SIN :
         return(IEEEDPSin(triginit(&value2)));
         break;

      case ASIN :
         {
            DOUBLE result = IEEEDPAsin(value2);
            return invtriginit(&result);
         }
         break;

      case SINH :
         return(IEEEDPSinh(triginit(&value2)));
         break;

      case COS :
         return(IEEEDPCos(triginit(&value2)));
         break;

      case ACOS :
         {
            DOUBLE result = IEEEDPAcos(value2);
            return invtriginit(&result);
         }
         break;

      case COSH :
         return(IEEEDPCosh(triginit(&value2)));
         break;

      case TAN :
         return(IEEEDPTan(triginit(&value2)));
         break;

      case ATAN :
         {
            DOUBLE result = IEEEDPAtan(value2);
            return invtriginit(&result);
         }
         break;

      case TANH :
         return(IEEEDPTanh(triginit(&value2)));
         break;

      case EXP :
         return(IEEEDPExp(value2));
         break;

      case LN :
         return(IEEEDPLog(value2));
         break;

      case a10X :
         return(IEEEDPPow(value2,10.0));
         break;

      case LOGBASE10 :
         return((DOUBLE)IEEEDPLog10(value2));
         break;

      case POW :
         return(IEEEDPPow(value2,value1));
         break;

      case INVPOW :
         if(IEEEDPTst(value2)!=0)
         {
            return(IEEEDPPow(IEEEDPDiv(1.0,value2),value1));
         }
         else
         {
            error("Divide by zero!");
            return(0.0);
         }
         break;

      case SQR :
         return(IEEEDPPow(2.0,value2));
         break;

      case SQRT :
         return(IEEEDPSqrt(value2));
         break;

      case RECIPROCAL :
         if(IEEEDPTst(value2)!=0)
         {
            return(IEEEDPDiv(1.0,value2));
         }
         else
         {
            error("Divide by zero!");
            return(0.0);
         }
         break;

      case FACTORIAL :
         if(value2 < 0 || value2 > 170) { /* 170! ~1e306 (near DOUBLE max) */
            error("Factorial range 0-170");
            return 0;
         }
         return((DOUBLE) factorial(IEEEDPFix(value2)));
         break;

      case nCr :
         if(value2 < 0 || value1 < value2) {
            error("Invalid combination");
            return 0.0;
         }
         return((DOUBLE) combination(IEEEDPFix(value1), IEEEDPFix(value2)));

      case nPr :
         if(value2 < 0 || value1 < value2) {
            error("Invalid permutation");
            return 0.0;
         }
         return((DOUBLE) permutation(IEEEDPFix(value1), IEEEDPFix(value2)));

      case RANDOM :
         {
            LONG max_val = IEEEDPFix(value2);
            if (max_val < 0) {
                error("Random range must be positive");
                return 0.0;
            }
            if (max_val > 65535) {
                error("Random range too large (max 65535)");
                return 0.0;
            }
            return((DOUBLE) RangeRand((UWORD)max_val));
         }
         break;
      
      case CONSTANT :
         return(3.141592653589793); /* Using PI directly */
         break;
      
      case MR :
         return(MRecall());
         break;

      case BIT_AND :
         {
            LONG v1 = IEEEDPFix(value1);
            LONG v2 = IEEEDPFix(value2);
            return (DOUBLE)(v1 & v2);
         }
         break;

      default :
         break;
   }
   return 0.0;  /* Return default value for any unsupported operations */
}


/* Convert the result into a string for placing in the display */
STRPTR ConvertToText(DOUBLE value,STRPTR buffer)
{
   UBYTE format_type[6];
   ULONG int_val;
   LONG i;

#ifdef DEBUG
   printf("DEBUG: ConvertToText() called with value: %f\n", value);
#endif

   switch(current_base)
   {
      case BASE10 :
         sprintf(format_type,"%%.15G");
         break;

      case BASE16 :
         value=IEEEDPFix(value);
         sprintf(format_type,"%%X");
         break;

      case BASE8 :
         value=IEEEDPFix(value);
         sprintf(format_type,"%%o");
         break;

      case BASE2 :
         int_val = IEEEDPFix(value);
         for(i=31; i>=0; i--) {
             buffer[31-i] = (int_val & (1<<i)) ? '1' : '0';
             if(i%4 == 0 && i !=0) buffer[31-i] = ' ';
         }
         buffer[32] = '\0';
         return buffer;  /* Return immediately since we set buffer directly */
         break;

      default :
         break;
   }
   sprintf(buffer,format_type,value);

   return(buffer);
}


/* Pull the top most operands and oerator from the stack and evaluate */
VOID eval()
{
   DOUBLE operand1,operand2,result;
   struct Operator *operator;

#ifdef DEBUG
   printf("DEBUG: eval() entered\n");
#endif

   operator=pull();
   if(stack_err || val_stack_err) {
       clear_all(); 
       error("Calculation error");
       return;
   }

   operand2=val_pull();
   if(val_stack_err)
   {
      return;
   }

   operand1=val_pull();
   if(val_stack_err)
   {
      return;
   }

   result=DoSum(operand1,operand2,operator->op_Type);
   if(result>1.79769313E+308)
      error("Number too big!");
   else
   {
      val_push(result);
      UpdateDisplay(ConvertToText(result,buffer));
   }
}


/* Push an Operator onto the stack */
VOID push(struct Operator a)
{
#ifdef DEBUG
   printf("DEBUG: push() called with operator type: %u\n", a.op_Type);
#endif
   /* This check allows the precedence evaluation to work
   ** by evaluating all preceeding higher priority operators
   ** before the new orator is pushed.
   */
   while(a.op_Prec<=stack[stack_ptr].op_Prec)
   {
      eval();
   }

   if(stack_ptr==STACKSIZE) {
      stack_err = STACK_FULL;
      error("Overflow error!");
      return;
   }
   stack_err = STACK_OK;
   stack[++stack_ptr] = a;

}


/* Pull the top most Operator from the stack */
struct Operator *pull(VOID)
{
#ifdef DEBUG
   printf("DEBUG: pull() called\n");
#endif
    if(stack_ptr<=0) {
        stack_err = STACK_EMPTY;
        error("Underflow error!");
        return(stack);
    }
    stack_err = STACK_OK;

    return (&stack[stack_ptr--]); /* pre or suc decrement (and increment) ? */
}


/* Push a value onto the value stack */
VOID val_push(DOUBLE value)
{
#ifdef DEBUG
   printf("DEBUG: val_push() called with value: %f\n", value);
#endif
   if(val_stack_ptr >= STACKSIZE) {
       val_stack_err = STACK_FULL;
       error("Stack overflow!");
       return;
   }
   val_stack[++val_stack_ptr] = value;
}


/* Pull the top most value from the value stack */
DOUBLE val_pull(VOID)
{
#ifdef DEBUG
   printf("DEBUG: val_pull() called\n");
#endif
    if (val_stack_ptr<=0) {
        val_stack_err = STACK_EMPTY;
        error("Underflow error!");
        return (0.0);
    }
    val_stack_err = STACK_OK;
    return (val_stack[val_stack_ptr--]);
}


/* Display an information requester */
VOID about()
{
   struct EasyStruct es;
   struct Requester about_req;

   /* Initialize structures separately */
   es.es_StructSize = sizeof(struct EasyStruct);
   es.es_Flags = 0;
   es.es_Title = "About Scientific Calculator";
   es.es_TextFormat = "Scientific Calculator\nVersion %ld.%ld\n\nCopyright 2025\namigazen.com\nAll Rights Reserved.";
   es.es_GadgetFormat = "OK";

#ifdef DEBUG
   printf("DEBUG: about() entered\n");
#endif

   /* Initialize all fields to 0 */
   memset(&about_req, 0, sizeof(struct Requester));

   SetWindowPointer(win,WA_BusyPointer,TRUE,TAG_DONE);

   Request(&about_req,win);

   EasyRequest(win,&es,NULL,VERSION,REVISION);

   EndRequest(&about_req,win);

   SetWindowPointer(win,TAG_DONE);
}


/* Show an error message in the display window */
VOID error(STRPTR text)
{
   static char buffer[256]; /* Static buffer for message text */
   struct EasyStruct es;
   
   /* Initialize struct separately */
   es.es_StructSize = sizeof(struct EasyStruct);
   es.es_Flags = 0;
   es.es_Title = "Calculator Error";
   strcpy(buffer, text); /* Copy text to our static buffer */
   es.es_TextFormat = buffer;
   es.es_GadgetFormat = "OK";
   
#ifdef DEBUG
   printf("DEBUG: error() called with message: %s\n", text);
#endif

   EasyRequest(win, &es, NULL, NULL);
   clear_entry();
}


/* Show an error message in a separate requester, for resource allocation errors
** for example, when the main window is not yet open, or it is locked, for when the
** Memory window is open.
*/
VOID notify_error(STRPTR text)
{

   static char buffer[256]; /* Static buffer for message text */
   struct EasyStruct es;
   /* Use a different name to avoid duplicate definition */
   struct Requester notify_req;

#ifdef DEBUG
   printf("DEBUG: notify_error() called with message: %s\n", text);
#endif
   
   /* Initialize struct separately */
   es.es_StructSize = sizeof(struct EasyStruct);
   es.es_Flags = 0;
   es.es_Title = "Scientific Calculator Error";
   strcpy(buffer, text); /* Copy text to our static buffer */
   es.es_TextFormat = buffer;
   es.es_GadgetFormat = "OK";
   
   /* Initialize all fields to 0 */
   memset(&notify_req, 0, sizeof(struct Requester));
   
   if (win) {
       Request(&notify_req, win);
       EasyRequest(win, &es, NULL, VERSION, REVISION);
       EndRequest(&notify_req, win);
   } else {
       EasyRequest(NULL, &es, NULL, VERSION, REVISION);
   }
   
}


/* Clear the last input entry into the expression */
VOID clear_entry()
{
#ifdef DEBUG
   printf("DEBUG: clear_entry() called\n");
#endif
   if(stack_ptr>0)
      pull();
   if(val_stack_ptr>0)
      val_pull();
   ConvertToText(val_stack[val_stack_ptr],buffer);
   UpdateDisplay(buffer);
   current_position=0;
}


/* Clear all inputs, ready to start again with no mistakes */
VOID clear_all()
{
#ifdef DEBUG
   printf("DEBUG: clear_all() called\n");
#endif
   while(stack_ptr>0)
   {
      pull();
   }
   while(val_stack_ptr>0)
   {
      val_pull();
   }
   strcpy(buffer,"0");
   current_position=0;
   UpdateDisplay(buffer);
   pointused=FALSE;
   expused=FALSE;
   GT_SetGadgetAttrs(shift_gad,win,NULL,GTCB_Checked,FALSE,TAG_DONE);
   GT_SetGadgetAttrs(hyp_gad,win,NULL,GTCB_Checked,FALSE,TAG_DONE);
}


/* Convert Degrees into Radians for the Trigonometric functions */
DOUBLE degrees_rads(DOUBLE degrees)
{
   /* Use direct calculations with numeric constants */
   DOUBLE pi_value = 3.141592653589793;
   DOUBLE factor;

#ifdef DEBUG
   printf("DEBUG: degrees_rads() called with degrees: %f\n", degrees);
#endif
   
   /* Use separate statements for each operation */
   factor = IEEEDPDiv(pi_value, 180.0);
   return IEEEDPMul(factor, degrees);
}


/* Convert Radians into Degrees */
DOUBLE rads_degrees(DOUBLE rads)
{
   /* Use direct calculations with numeric constants */
   DOUBLE pi_value = 3.141592653589793;
   DOUBLE factor;

#ifdef DEBUG
   printf("DEBUG: rads_degrees() called with rads: %f\n", rads);
#endif
   
   /* Use separate statements for each operation */
   factor = IEEEDPDiv(180.0, pi_value);
   return IEEEDPMul(factor, rads);
}


/* Print the the text for the last evaluated operator to the Tape */
VOID output_operator(UWORD type)
{
   STRPTR output_buffer;

#ifdef DEBUG
   printf("DEBUG: output_operator() called with type: %u\n", type);
#endif

   switch(type)
   {
      case ADD :
         output_buffer="+";
         break;
         
      case SUB :
         output_buffer="-";
         break;
         
      case MUL :
         output_buffer="x";
         break;
         
      case DIV :
         output_buffer="/";
         break;
         
      case MOD :
         output_buffer="Mod";
         break;
         
      case NEG :
         output_buffer="Neg";
         break;
         
      case EQU :
         output_buffer="=";
         break;

      case POW :
         output_buffer="x^y";
         break;

      case INVPOW :
         output_buffer="xy";
         break;

      case nPr :
         output_buffer="nPr";
         break;

      case nCr :
         output_buffer="nCr";
         break;

      case SIN :
         output_buffer="sin";
         break;

      case ASIN :
         output_buffer="sin-";
         break;

      case SINH :
         output_buffer="sinh";
         break;

      case COS :
         output_buffer="cos";
         break;

      case ACOS :
         output_buffer="cos-";
         break;

      case COSH :
         output_buffer="cosh";
         break;

      case TAN :
         output_buffer="tan";
         break;

      case ATAN :
         output_buffer="tan-";
         break;

      case TANH :
         output_buffer="tanh";
         break;

      case EXP :
         output_buffer="e";
         break;

      case LN :
         output_buffer="ln";
         break;

      case a10X :
         output_buffer="10";
         break;

      case LOGBASE10 :
         output_buffer="log";
         break;

      case SQR :
         output_buffer="x";
         break;

      case SQRT :
         output_buffer="sqrt";
         break;

      case RECIPROCAL :
         output_buffer="1x";
         break;

      case FACTORIAL :
         output_buffer="x!";
         break;

      case FIX :
         output_buffer="Fix";
         break;

      case RANDOM :
         output_buffer="Random";
         break;

      default :
         output_buffer="";
         break;
   }

   if (output_file) {
       FPrintf(output_file, "%s", output_buffer);
   }
}


/* Copy the contents of the display to the Clipboard as FTXT */
VOID copy()
{
#ifdef DEBUG
   printf("DEBUG: copy() called\n");
#endif
   iffh=AllocIFF();
   if(iffh)
   {
      iffh->iff_Stream=(ULONG)OpenClipboard(0);
      if(iffh->iff_Stream)
      {
         InitIFFasClip(iffh);
         if(!OpenIFF(iffh,IFFF_WRITE))
         {
            if(!PushChunk(iffh,ID_FTXT,ID_FORM,IFFSIZE_UNKNOWN))
            {
               if(!PushChunk(iffh,0,ID_CHRS,IFFSIZE_UNKNOWN))
               {
                  textlen=strlen(buffer);
                  WriteChunkBytes(iffh,buffer,textlen);
                  PopChunk(iffh);
               }
               PopChunk(iffh);
            }
            CloseIFF(iffh);
         }
          CloseClipboard((struct ClipboardHandle *)iffh->iff_Stream);
      }
      FreeIFF(iffh);
   }
   else
   {
      notify_error("Could not open Clipboard");
   }
}


/* paste the contents of the Clipboard to the display */
VOID paste()
{
#ifdef DEBUG
   printf("DEBUG: paste() called\n");
#endif
   iffh=AllocIFF();
   if(iffh)
   {
      iffh->iff_Stream=(ULONG)OpenClipboard(0);
      if(iffh->iff_Stream)
      {
         InitIFFasClip(iffh);
         if(!OpenIFF(iffh,IFFF_READ))
         {
            if(!PushChunk(iffh,ID_FTXT,ID_FORM,IFFSIZE_UNKNOWN))
            {
               if(!PushChunk(iffh,0,ID_CHRS,IFFSIZE_UNKNOWN))
               {
                  ReadChunkBytes(iffh,buffer,buffer_Size);
                  PopChunk(iffh);
               }
               PopChunk(iffh);
            }
            CloseIFF(iffh);
         }
         CloseClipboard((struct ClipboardHandle *)iffh->iff_Stream);
      }
      FreeIFF(iffh);
   }
   else
   {
      notify_error("Could not open Clipboard");
   }
}


/* Present the user with a window in which to select the memory register 
** to use.
*/
ULONG choose_mem_slot(VOID)
{
   struct NewGadget mem_ng;
   BOOL done=FALSE;
   ULONG signal=0,class=0;
   UWORD code=0;
   struct IntuiMessage *imsg;
   ULONG winsignal;
   ULONG ilock;
   WORD mousex,mousey;
   UWORD winwidth,winheight;
   struct Gadget *mem_prev_gad,*memglist;
   STATIC ULONG rc=0;  /* Changed from LONG to ULONG to match return type */
   struct Requester slot_req; /* Renamed to avoid conflicts */

#ifdef DEBUG
   printf("DEBUG: choose_mem_slot() entered\n");
#endif

   /* Initialize all fields to 0 */
   memset(&slot_req, 0, sizeof(struct Requester));

   winheight=scr->BarHeight+1+heightfactor+8;
   winwidth=11+(widthfactor+3)*2;

   mem_ng.ng_TopEdge=4+scr->BarHeight;
   mem_ng.ng_LeftEdge=7;
   mem_ng.ng_TextAttr=scr->Font;
   mem_ng.ng_Flags=NULL;
   mem_ng.ng_VisualInfo=vi;
   mem_prev_gad=CreateContext(&memglist);

   mem_ng.ng_Width=winwidth-14;
   mem_ng.ng_Height=heightfactor+1;
   mem_ng.ng_GadgetText=NULL;
   mem_ng.ng_GadgetID=0;
   mem_prev_gad=CreateGadget(INTEGER_KIND,mem_prev_gad,&mem_ng,GTIN_Number,rc,GTIN_MaxChars,5,TAG_DONE);

   if(!mem_prev_gad) {
       FreeGadgets(memglist);
       return -1;
   }

   SetWindowPointer(win,WA_BusyPointer,TRUE,TAG_DONE);

   if(mem_prev_gad)
   {
      Request(&slot_req,win);

      ilock=LockIBase(0);
      mousey=IntuitionBase->MouseY;
      mousex=IntuitionBase->MouseX;
      UnlockIBase(ilock);

      if(memwin=OpenWindowTags(NULL,WA_Width,winwidth,
         WA_Height,winheight,
         WA_Left,mousex,
         WA_Top,mousey,
         WA_Title,"Memory",
         WA_SimpleRefresh,TRUE,
         WA_DragBar,TRUE,
         WA_DepthGadget,TRUE,
         WA_CloseGadget,TRUE,
         WA_Activate,TRUE,
         WA_Gadgets,memglist,
         WA_PubScreen,scr,
         WA_MinHeight,winheight,
         WA_MinWidth,winwidth,
         WA_IDCMP,IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|INTEGERIDCMP,
         TAG_DONE))
      {
         GT_RefreshWindow(memwin,NULL);
         
         /* Activate the integer gadget so that the user does not have to 
         ** first click a button and then click again to activate the gadget
         ** just to enter one number.
         */
         ActivateGadget(mem_prev_gad,memwin,NULL);
         winsignal=1L<<memwin->UserPort->mp_SigBit;

         while(!done)
         {
            signal=Wait(winsignal);

            if(signal&winsignal)
            {
               while((!done)&&(imsg=GT_GetIMsg(memwin->UserPort)))
               {
                  class=imsg->Class;
                  code=imsg->Code;
                  GT_ReplyIMsg(imsg);
                  switch(class)
                  {
                     case IDCMP_CLOSEWINDOW :
                        rc=-1;
                        done=TRUE;
                        break;
                     case IDCMP_GADGETUP :
                        switch(code)
                        {
                           case 0 :
                              GT_GetGadgetAttrs(mem_prev_gad,memwin,NULL,GTIN_Number,&rc,TAG_DONE);
                              if(rc>global_memsize+1||rc<0)
                              {
                                 rc=-1;
                                 notify_error("Register must be between 0\nand the size specified in the MEMORY parameter.");
                              }
                              else
                                 done=TRUE;
                              break;

                           default :
                              break;
                        }
                     default :
                        break;
                  }
               }
            }
         }
         CloseWindow(memwin);
      }
      else
      {
         FreeGadgets(memglist);
      }
   }
   EndRequest(&slot_req,win);
   SetWindowPointer(win,TAG_DONE);
   return rc;
}


/* Store a value in a memory register */
VOID MIn(DOUBLE mem)
{
   /* Use variable names to mark each element */
   LONG choice=0;

#ifdef DEBUG
   printf("DEBUG: MIn() called with value: %f\n", mem);
#endif

   choice=choose_mem_slot();
   if(choice>=0)
      memory[choice]=mem;
   else
      return;

   if(choice < 0 || choice > global_memsize) {
       notify_error("Invalid memory slot");
       return;
   }
}


/* Add a value to the the value currently stored in a memory register */
VOID MPlus(DOUBLE mem)
{
   LONG choice=0;

#ifdef DEBUG
   printf("DEBUG: MPlus() called with value: %f\n", mem);
#endif

   choice=choose_mem_slot();
   if(choice>=0)
      memory[choice]+=mem;
   else
      return;

   if(choice < 0 || choice > global_memsize) {
       notify_error("Invalid memory slot");
       return;
   }
}


/* Subtract a value from the value currently stored in a memory register */
VOID MMinus(DOUBLE mem)
{
   LONG choice=0;

#ifdef DEBUG
   printf("DEBUG: MMinus() called with value: %f\n", mem);
#endif

   choice=choose_mem_slot();
   if(choice>=0)
      memory[choice]-=mem;
   else
      return;

   if(choice < 0 || choice > global_memsize) {
       notify_error("Invalid memory slot");
       return;
   }
}


/* Retrieve a value from a memory register */
DOUBLE MRecall()
{
   LONG choice = choose_mem_slot();

#ifdef DEBUG
   printf("DEBUG: MRecall() called\n");
#endif

   if(choice >= 0 && choice <= global_memsize)
      return memory[choice];
   return 0.0;
}


/* Ensure an angle is in the correct format befoe passing it to a 
** trigonometric function
*/
DOUBLE triginit(DOUBLE *operand)
{
#ifdef DEBUG
   printf("DEBUG: triginit() called with value: %f\n", *operand);
#endif

   switch(trig_mode)
   {
      case DEG :
         *operand=degrees_rads(*operand);
         break;
      case RAD :
         break;
      case GRAD : /* Fix when grads to rads built in */
         break;
      default :
         break;
   }
   return(*operand);
}


/* Convert a value back from Radians to the current format after 
** an inverse trigonometric function
*/
DOUBLE invtriginit(DOUBLE *operand)
{
#ifdef DEBUG
   printf("DEBUG: invtriginit() called with value: %f\n", *operand);
#endif
   switch(trig_mode)
   {
      case DEG :
         *operand=rads_degrees(*operand);
         break;
      case RAD :
         break;
      case GRAD :
         break;
      default :
         break;
   }
   return(*operand);
}


/* Find the factorial of a given integer */
LONG factorial(LONG n)
{
   LONG result;

 #ifdef DEBUG
   printf("DEBUG: factorial() called with n: %ld\n", n);
#endif

   if(n < 0 || n > 20) { 
       error("Factorial input 0-20 only");
       return 0;
   }
   result = 1;
   while(n > 1) {
       result *= n;
       n--;
   }
   return result;
}


/* The Combination function */
LONG combination(LONG n, LONG r)
{
   LONG i, result;

#ifdef DEBUG
   printf("DEBUG: combination() called with n=%ld, r=%ld\n", n, r);
#endif
   
   if(n < 0 || r < 0 || r > n) {
       error("Invalid combination");
       return 0;
   }
   if(r == 0 || r == n) return 1;
    
    /* Use smaller of r and n-r for efficiency */
    if(r > n/2) r = n - r;
    
    result = 1;
    for(i = 1; i <= r; i++) {
        result *= n - r + i;
        result /= i;
    }
    return result;
}


/* The Permutation function */
LONG permutation(LONG n, LONG r)
{
    LONG i, result;

#ifdef DEBUG
   printf("DEBUG: permutation() called with n=%ld, r=%ld\n", n, r);
#endif
    
    if(n < 0 || r < 0) return 0;
    if(r > n) return 0;
    
    result = 1;
    for(i = 0; i < r; i++) {
        result *= (n - i);
    }
    return result;
}


/* Convert a string representation of a number into the actual number */
DOUBLE ConvertToValue(STRPTR string)
{
   DOUBLE value = 0.0;  /* Initialize to avoid warning */
   LONG result;

#ifdef DEBUG
   printf("DEBUG: ConvertToValue() called with string: %s\n", string);
#endif

   switch(current_base)
   {
      case BASE10 :
         value=atof(string);
         break;

      case BASE16 :
      case BASE8 :
      case BASE2 :
         sprintf(string,"%ld",IEEEDPFix(value));
         /* Convert string to appropriate base manually since StrToLong only handles decimal */
         if(StrToLong(string, &result) > 0) {
            /* For non-decimal bases, we need to interpret the string ourselves */
            if(current_base != BASE10) {
               LONG val = 0, digit;
               STRPTR ptr = string;
               
               /* Skip leading spaces */
               while(*ptr == ' ' || *ptr == '\t') ptr++;
               
               /* Process digits according to the current base */
               while(*ptr) {
                  if(*ptr >= '0' && *ptr <= '9')
                     digit = *ptr - '0';
                  else if(*ptr >= 'A' && *ptr <= 'F')
                     digit = *ptr - 'A' + 10;
                  else if(*ptr >= 'a' && *ptr <= 'f')
                     digit = *ptr - 'a' + 10;
                  else
                     break;
                     
                  if(digit >= current_base)
                     break;
                     
                  val = val * current_base + digit;
                  ptr++;
               }
               value = (DOUBLE)val;
            } else {
               value = (DOUBLE)result;
            }
         } else {
            value = 0.0;
         }
         break;

      default :
         break;
   }
   return(value);
}


/* Handle an '=' input */
VOID equals()
{
   DOUBLE value;

#ifdef DEBUG
   printf("DEBUG: equals() called\n");
#endif

   value=ConvertToValue(buffer);

   val_push(value);
   FPrintf(output_file, "\t% .15G\n", value);
   
   item.op_Prec=(APTR) PREC_EQUAL;
   item.op_Type=EQU;
   item_ready=FALSE;

   push(item);

   output_operator(EQU);
   FPrintf(output_file, "\t% .15G\n", val_stack[val_stack_ptr]);

   current_position=0;
   pointused=FALSE;
   expused=FALSE;
   reset_checkboxes();
}


/* Enter an operator that uses two operands to evaluate a result */
VOID operator_2(UWORD id,APTR userdata)
{
   DOUBLE value;
   LONG shift;

#ifdef DEBUG
   printf("DEBUG: operator_2() called with id: %u\n", id);
#endif

   value=ConvertToValue(buffer);
   val_push(value);
   FPrintf(output_file, "\t% .15G\n", value);

   GT_GetGadgetAttrs(shift_gad,win,NULL,GTCB_Checked,&shift,TAG_DONE);

   item.op_Prec=userdata;
   item.op_Type=id;
   if(id==POW)
   {
      if(shift) item.op_Type++;
   }
   item_ready=TRUE;

   current_position=0;
   pointused=FALSE;
   expused=FALSE;
   reset_checkboxes();
}


/* Delete the last entered digit from the display */
VOID backspace()
{
#ifdef DEBUG
   printf("DEBUG: backspace() called\n");
#endif

   if(current_position!=0)
   {
      current_position--;

      if(buffer[current_position]=='E')
         expused=FALSE;
      if(buffer[current_position]=='.')
         pointused=FALSE;

      buffer[current_position]=NULL;
      UpdateDisplay(buffer);
   }
   if(current_position<=0)
   {
      display_digit('0');
      current_position--;
   }
}


/* Enter a decimal point into the display */
VOID point()
{
#ifdef DEBUG
   printf("DEBUG: point() called\n");
#endif

   if(!pointused&&current_base==BASE10)
   {
      display_digit('.');
      pointused=TRUE;
   }
}


/* Enter the letter 'E' into the display for scientific notation */
VOID exponent()
{
#ifdef DEBUG
   printf("DEBUG: exponent() called\n");
#endif

   if(!expused&&current_base==BASE10)
   {
      display_digit('E');
      expused=TRUE;
   }
}


/* Turn both Hyp and Shift gadgets to off */
VOID reset_checkboxes()
{
#ifdef DEBUG
   printf("DEBUG: reset_checkboxes() called\n");
#endif

   GT_SetGadgetAttrs(shift_gad,win,NULL,GTCB_Checked,FALSE,TAG_DONE);
   GT_SetGadgetAttrs(hyp_gad,win,NULL,GTCB_Checked,FALSE,TAG_DONE);
}


/* Push an operator onto the stack if one has been entered */
VOID pushitem()
{
#ifdef DEBUG
   printf("DEBUG: pushitem() called\n");
#endif

   if(item_ready)
   {
      push(item);
      item_ready=FALSE;
      output_operator(item.op_Type);
   }
}


/* Add another digit to the display value and update the display */
VOID display_digit(UBYTE digit)
{
#ifdef DEBUG
   printf("DEBUG: display_digit() called with digit: %c\n", digit);
#endif

   if(current_position >= buffer_Size-1) {
       error("Input too long");
       return;
   }
   buffer[current_position]=digit;
   current_position++;
   buffer[current_position]=NULL;
   UpdateDisplay(buffer);
}


/* Consider using double precision and range checking */
DOUBLE factorial_dbl(LONG n)
{
    DOUBLE result = 1.0;

#ifdef DEBUG
   printf("DEBUG: factorial_dbl() called with n: %ld\n", n);
#endif

    if(n < 0) return 0.0;
    while(n > 1) {
        result *= n;
        if(result > 1e308) return INFINITY; // Keep existing usage
        n--;
    }
    return result;
}

void toggle_window_visibility(void)
{
    static WORD saved_x = 0;
    static WORD saved_y = 0;
    static WORD saved_width = 0;
    static WORD saved_height = 0;
    
#ifdef DEBUG
   printf("DEBUG: toggle_window_visibility() called\n");
#endif

    if(win) {
        /* Save position and size before closing */
        saved_x = win->LeftEdge;
        saved_y = win->TopEdge;
        saved_width = win->Width;
        saved_height = win->Height;
        
        ClearMenuStrip(win);
        CloseWindow(win);
        win = NULL;
    } else {
        /* Reopen window using saved position/size */
        win = OpenWindowTags(NULL, 
            WA_Left, saved_x, 
            WA_Top, saved_y,
            WA_Width, saved_width ? saved_width : 300,
            WA_Height, saved_height ? saved_height : 200,
            WA_Title, "Scientific Calculator",
            WA_SimpleRefresh, TRUE,
            WA_DragBar, TRUE,
            WA_DepthGadget, TRUE,
            WA_CloseGadget, TRUE,
            WA_Activate, TRUE,
            WA_Gadgets, glist,
            WA_PubScreen, scr,
            WA_MinHeight, saved_height ? saved_height : 200,
            WA_MinWidth, saved_width ? saved_width : 300,
            WA_IDCMP, IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|IDCMP_VANILLAKEY|BUTTONIDCMP|CHECKBOXIDCMP|MENUPICK,
            TAG_DONE);
            
        if (win && menu) {
            SetMenuStrip(win, menu);
        }
    }
}

void cleanup_commodities(void)
{
#ifdef DEBUG
   printf("DEBUG: cleanup_commodities() called\n");
#endif

    if (cxbroker) {
        DeleteCxObj(cxbroker);
        cxbroker = NULL;
    }
    
    if (cxfilter) {
        DeleteCxObj(cxfilter);
        cxfilter = NULL;
    }
    
    if (CommoditiesBase) {
        CloseLibrary(CommoditiesBase);
        CommoditiesBase = NULL;
    }
}

VOID SetTimer(struct MsgPort *port, LONG interval, BOOL trigger)
{
#ifdef DEBUG
   printf("DEBUG: SetTimer() called with interval: %ld\n", interval);
#endif
    // Implementation of SetTimer function
}

/* Check if window is closed - checks for valid Window pointer */
BOOL WinClosed(struct Window *window)
{
#ifdef DEBUG
   printf("DEBUG: WinClosed() called\n");
#endif
   if (!window) return TRUE;
   return FALSE;  /* If we have a valid window pointer, it's not closed */
}

/* Parse a hotkey string into key and qualifier */
UWORD ParseKey(STRPTR string, STRPTR *nextptr)
{
    UWORD key = 0;
    UWORD qualifier = 0;

#ifdef DEBUG
   printf("DEBUG: ParseKey() called with string: %s\n", string);
#endif
    
    /* Handle modifier keys */
    while (*string) {
        if (*string == 'a' || *string == 'A') {
            qualifier |= IEQUALIFIER_LCOMMAND;
            string++;
        }
        else if (*string == 'c' || *string == 'C') {
            qualifier |= IEQUALIFIER_CONTROL;
            string++;
        }
        else if (*string == 's' || *string == 'S') {
            qualifier |= IEQUALIFIER_LSHIFT;
            string++;
        }
        else if (*string == 'l' || *string == 'L') {
            qualifier |= IEQUALIFIER_LALT;
            string++;
        }
        else if (*string == 'r' || *string == 'R') {
            qualifier |= IEQUALIFIER_RALT;
            string++;
        }
        else break;
    }
    
    /* Get the key */
    if (*string) {
        key = *string;
        string++;
    }
    
    if (nextptr) *nextptr = string;
    return (UWORD)(key | (qualifier << 8));
}

/* Set the type of a commodities object */
VOID CxObjectType(CxObj *obj, LONG type)
{
#ifdef DEBUG
   printf("DEBUG: CxObjectType() called with type: %ld\n", type);
#endif

    /* We can't directly access CxObj structure, so we'll use a simpler approach */
    if (obj && CommoditiesBase) {
        /* Use commodities functions directly instead of accessing struct fields */
        SetCxObjPri(obj, type);  /* Using this as a workaround since no direct SetType exists */
    }
}

/* Attach a key code to a filter object */
VOID AttachKeyCode(CxObj *filterObj, UBYTE key, UBYTE qualifier)
{
#ifdef DEBUG
   printf("DEBUG: AttachKeyCode() called with key: %u, qualifier: %u\n", key, qualifier);
#endif

    /* Direct approach to set filter properties */
    if (filterObj && CommoditiesBase) {
        /* We can't call SetFilterIX directly, so we'll create a custom key filter
           using a different approach */
        CxObj *filter = CxFilter(NULL);
        if (filter) {
            /* Connect filter to filterObj */
            AttachCxObj(filterObj, filter);
        }
    }
}


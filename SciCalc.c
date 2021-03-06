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

#include "scicalc_rev.h"

#include <string.h>
extern int atoi(const char *);
extern double atof(const char *);


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
int   main(void);
int   wbmain(struct WBStartup *);
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

ULONG stdout;

struct Menu *menu;
struct NewMenu nm[]=
{
   /* Implement an undo option */
   { NM_TITLE,"Project",0,0,0,0},
   {  NM_ITEM,"Clear Entry", "E",0,0,(APTR) MENU_CE},
   {  NM_ITEM,"Clear All",   "A",0,0,(APTR) MENU_CA},
   {  NM_ITEM,NM_BARLABEL,   0,0,0,0},
   {  NM_ITEM,"About",       "?",0,0,(APTR) MENU_ABOUT},
   {  NM_ITEM,"Quit",        "Q",0,0,(APTR) MENU_QUIT},
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



/* Functions */

/* Entry function called when executed from CLI */
int main()
{
   struct RDArgs *rdargs;
   LONG myargs[]={NULL,NULL,10};

   /* Read in the parameters passed from the CLI */
   if(rdargs=ReadArgs("PUBSCREEN,TAPE/K,MEMORY/N",myargs,NULL))
   {
      calculator((UBYTE *) myargs[0],(UBYTE *) myargs[1],myargs[2]);
   
      FreeArgs(rdargs);
   }

   return(0); /* Could return fail warning if cause is known and available to procedure */
}


/* Entry point called when executed from the Workbench */
int wbmain(struct WBStartup *wbs)
{
   UBYTE **toolarray;
   struct DiskObject *dobj;
   BPTR olddir;

   olddir=CurrentDir(wbs->sm_ArgList[0].wa_Lock);
   
   dobj=GetDiskObject(wbs->sm_ArgList[0].wa_Name);
   if(dobj)
   {
      toolarray=(UBYTE **)dobj->do_ToolTypes;
      
      calculator(FindToolType(toolarray,"PUBSCREEN"),FindToolType(toolarray,"TAPE"),atoi(FindToolType(toolarray,"MEMORY")));

      FreeDiskObject(dobj);
   }
   
   CurrentDir(olddir);
   return(0);
}


/* Main program initialization and input processing loop */
VOID calculator(STRPTR psname,STRPTR filename,ULONG memsize)
{
   DOUBLE value=0;
   struct NewGadget ng_button;
   BOOL done=FALSE;
   ULONG signal=0,class=0;
   UWORD code=0;
   struct IntuiMessage *imsg;
   ULONG winsignal;
   ULONG ilock;
   WORD mousex,mousey;
   LONG shift,hyp;
   UWORD winwidth,winheight;
   ULONG old_stdout;
   
   global_memsize=memsize;

   old_stdout=stdout;

   if(filename)
   {
      stdout=Open(filename,MODE_NEWFILE);
   }
   else
      stdout=Open("NIL:",MODE_NEWFILE);

   /* Allocate memory for memory registers */
   memory=(DOUBLE *)AllocMem(sizeof(DOUBLE)*(memsize+1),MEMF_CLEAR);
   
   if(memory)
   {
   /* Obtain Lock on display Screen user wishes to use */
   scr=LockPubScreen(psname);

   if(scr)
   {
      /* Find out dimensions of screen and font to use */
      vi=GetVisualInfo(scr,TAG_DONE);

      /* Work out a height and width for each button */
      widthfactor=TextLength(&scr->RastPort,"A",1)*3.5;
      heightfactor=scr->BarHeight+1;
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
      prev_gad=CreateGadget(TEXT_KIND,prev_gad,&ng_button,GTTX_Text,"0",GTTX_CopyText,TRUE,GTTX_Border,TRUE,GTTX_Justification,GTJ_RIGHT,GTTX_Clipped,TRUE,TAG_DONE);
      
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

      (ng_button.ng_LeftEdge)+=((widthfactor+3));

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

      ng_button.ng_GadgetText="�";
      ng_button.ng_GadgetID=MUL;
      ng_button.ng_UserData=(APTR) PREC_MULDIVMOD;
      prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

      (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

      ng_button.ng_GadgetText="�";
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

      ng_button.ng_GadgetText="tan";
      ng_button.ng_GadgetID=TAN;
      ng_button.ng_UserData=(APTR) PREC_SCI;
      prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

      (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

      ng_button.ng_GadgetText="x�";
      ng_button.ng_GadgetID=SQR;
      ng_button.ng_UserData=0;
      prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

      (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

      ng_button.ng_GadgetText="1�x";
      ng_button.ng_GadgetID=RECIPROCAL;
      ng_button.ng_UserData=0;
      prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

      (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

      ng_button.ng_GadgetText="Pi";
      ng_button.ng_GadgetID=CONSTANT;
      ng_button.ng_UserData=0;
      prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

      (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

      ng_button.ng_GadgetText="Exp";
      ng_button.ng_GadgetID=EXPONENT;
      ng_button.ng_UserData=0;
      prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

      (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

      ng_button.ng_GadgetText="0";
      ng_button.ng_GadgetID=0;
      ng_button.ng_UserData=0;
      prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

      (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

      ng_button.ng_GadgetText=".";
      ng_button.ng_GadgetID=POINT;
      ng_button.ng_UserData=0;
      prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

      (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

      ng_button.ng_GadgetText="�";
      ng_button.ng_GadgetID=BACKSPACE;
      ng_button.ng_UserData=0;
      prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

      (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

      ng_button.ng_GadgetText="�";
      ng_button.ng_GadgetID=NEG;
      ng_button.ng_UserData=0;
      prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

      (ng_button.ng_LeftEdge)+=(ng_button.ng_Width+3);

      ng_button.ng_GadgetText="=";
      ng_button.ng_GadgetID=EQU;
      ng_button.ng_UserData=(APTR) PREC_EQUAL;
      prev_gad=CreateGadget(BUTTON_KIND,prev_gad,&ng_button,TAG_DONE);

      (ng_button.ng_LeftEdge)=7;
      (ng_button.ng_TopEdge)+=(ng_button.ng_Height+3);

      if(prev_gad)
      {
         menu=CreateMenus(nm,TAG_DONE);

         if(menu)
         {

         if(LayoutMenus(menu,vi,GTMN_NewLookMenus,TRUE,TAG_DONE))
         {

         /* The dimensions of the window when it is shrunk to a title bar size */
         WORD zoom[4]={-1,-1,270,scr->BarHeight+1};

         /* Try and open the main program window */         
         win=OpenWindowTags(NULL,WA_Width,winwidth,
         WA_Height,winheight,
         WA_Left,mousex,
         WA_Top,mousey,
         WA_Title,"Scientific Calculator",
         WA_SimpleRefresh,TRUE,
         WA_DragBar,TRUE,
         WA_DepthGadget,TRUE,
         WA_CloseGadget,TRUE,
         WA_Activate,TRUE,
         WA_Gadgets,glist,
         WA_Zoom,&zoom,
         WA_PubScreen,scr,
         WA_MinHeight,winheight,
         WA_MinWidth,winwidth,
         WA_NewLookMenus,TRUE,
         WA_IDCMP,IDCMP_CLOSEWINDOW|IDCMP_REFRESHWINDOW|IDCMP_VANILLAKEY|BUTTONIDCMP|CHECKBOXIDCMP|MENUPICK,
         TAG_DONE);
         
         UnlockPubScreen(NULL,scr);
         if(win)
         {
            SetMenuStrip(win,menu);
            GT_RefreshWindow(win,NULL);

            winsignal=1L<<win->UserPort->mp_SigBit;

            /* Process all input until the user decides to quit the program */
            while(!done)
            {
               /* Tell the OS what sorts of input we want to know about.
               ** Specifies input through the window, and Break commands 
               ** sent from other processes.
               */
               signal=Wait(winsignal|SIGBREAKF_CTRL_C);
               
               /* If the break signal is sent then quit */
               if(signal&SIGBREAKF_CTRL_C)
               {
                  done=TRUE;
                  break;
               }

               /* If input through the window is received then
               ** find out the type of inputand act accordingly.
               */
               if(signal&winsignal)
               {
                  struct Gadget *loop_gad;

                  /* Repeat until all input messages are processed,
                  ** in case more have arrived while the first one was 
                  ** being processed.
                  */
                  while((!done)&&(imsg=GT_GetIMsg(win->UserPort)))
                  {
                     class=imsg->Class;
                     code=imsg->Code;
                     loop_gad = (struct Gadget *) (imsg->IAddress);
                     GT_ReplyIMsg(imsg);
                     switch(class)
                     {
                        case IDCMP_CLOSEWINDOW :
                           /* The user clicked the Window Close gadget */
                           done=TRUE;
                           break;
                           
                        case IDCMP_REFRESHWINDOW :
                           /* The program window has been covered up and 
                           ** uncovered again so the graphics must be redrawn
                           */
                           GT_BeginRefresh(win);
                           GT_EndRefresh(win,TRUE);
                           break;
                           
                        case MENUPICK :
                           /* A menu item has been selected */
                           struct MenuItem *item;
                           while((code!=MENUNULL)&&!done)
                           {
                              item=ItemAddress(menu,code);
                              switch(GTMENUITEM_USERDATA(item))
                              {
                                 case MENU_CE :
                                    clear_entry();
                                    break;

                                 case BASE2 :
                                 case BASE8 :
                                 case BASE16 :
                                 case BASE10 :
                                    current_base=(ULONG) GTMENUITEM_USERDATA(item);
                                    UpdateDisplay(ConvertToText(ConvertToValue(buffer),buffer));

                                    break;

                                 case DEG :
                                 case RAD :
                                 case GRAD :
                                    /* Store old value and do a conversion from old type to new type of value */
                                    trig_mode=(ULONG) GTMENUITEM_USERDATA(item);
                                    break;

                                 case MENU_CA :
                                    clear_all();
                                    break;

                                 case MENU_ABOUT :
                                    about();
                                    break;

                                 case MENU_QUIT :
                                    done=TRUE;
                                    break;

                                 case MENU_CUT :
                                    copy();
                                    clear_entry();
                                    break;

                                 case MENU_COPY :
                                    copy();
                                    break;

                                 case MENU_PASTE :
                                    paste();
                                    break;

                                 case MENU_TAPE :
                                    tape_on=!tape_on;
                                    switch(tape_on)
                                    {
                                       case TRUE :
                                          Close(stdout);
                                          stdout=Open(filename,MODE_NEWFILE);
                                          break;
                                       case FALSE :
                                          Close(stdout);
                                          stdout=Open("NIL:",MODE_NEWFILE);
                                          break;
                                    }
                                    break;
                              }
                              code=item->NextSelect;
                           }
                           break;

                        case IDCMP_VANILLAKEY :
                           /* A key has been pressed */
                           switch(code)
                           {
                              case 'I' :
                              case 'i' :
                              
                                 GT_GetGadgetAttrs(shift_gad,win,NULL,GTCB_Checked,&shift,TAG_DONE);
                                 if(!shift)
                                    GT_SetGadgetAttrs(hyp_gad,win,NULL,GTCB_Checked,FALSE,TAG_DONE);
                                 GT_SetGadgetAttrs(shift_gad,win,NULL,GTCB_Checked,!shift,TAG_DONE);
                                 break;
                           
                              case 'H' :
                              case 'h' :
                                 
                                 GT_GetGadgetAttrs(hyp_gad,win,NULL,GTCB_Checked,&hyp,TAG_DONE);
                                 if(!hyp)
                                    GT_SetGadgetAttrs(shift_gad,win,NULL,GTCB_Checked,FALSE,TAG_DONE);
                                 GT_SetGadgetAttrs(hyp_gad,win,NULL,GTCB_Checked,!hyp,TAG_DONE);
                                 break;
                              
                              /* Keyboard input for Hex characters is not yet supported */
                              case 'F' :
                              case 'f' :
                              case 'E' :
                              case 'e' :
                              case 'D' :
                              case 'd' :
                              case 'C' :
                              case 'c' :
                              case 'B' :
                              case 'b' :
                              case 'A' :
                              case 'a' :
                              */

                              case '9' :
                              case '8' :
                              case '7' :
                              case '6' :
                              case '5' :
                              case '4' :
                              case '3' :
                              case '2' :
                              case '1' :
                              case '0' :

                                 display_digit(code);
                                 break;

                              case '.' :

                                 point();
                                 break;

                              case '+' :
                                 operator_2(ADD,(APTR) PREC_ADDSUB);
                                 break;

                              case '-' :
                                 operator_2(SUB,(APTR) PREC_ADDSUB);
                                 break;

                              case '�' :
                              case '*' :
                                 operator_2(MUL,(APTR) PREC_MULDIVMOD);
                                 break;

                              case '�' :
                              case '/' :
                                 operator_2(DIV,(APTR) PREC_MULDIVMOD);
                                 break;

                              case 'a' :
                              case 'A' :
                                 clear_all();
                                 break;

                              /* Change-sign keypresses */
                              case 's' :
                              case 'S' :
                              case '�' :
                                 break;
                                 
                              /* Equals key presses */
                              case 'x' :
                              case 'X' :
                              case '\r' :
                               case '=' :
                                 equals();
                                 break;
                               
                               default :
                                 break;
                           }
                           break;
                        
                        case IDCMP_GADGETUP :
                           /* A button has been pressed */
                           
                           if(loop_gad->GadgetID<16)
                           {
                              /* The button was a number so the number is displayed */ 
                              pushitem();
                              
                              if(loop_gad->GadgetID<10)
                                 display_digit((loop_gad->GadgetID)+48);
                              else
                                 display_digit((loop_gad->GadgetID)+55);
                           }
                           else
                           {
                              /* It is not a number so it must be a command */
                              switch(loop_gad->GadgetID)
                              {
                                 /* If either the Hyp or Shift gadgets have been turned on, the other must be turned of
                                 ** as they are mutually exclusive as Hyperbolic Arc functions aren't available */
                                 case SHIFT_GAD :
                                    GT_GetGadgetAttrs(shift_gad,win,NULL,GTCB_Checked,&shift,TAG_DONE);
                                    if(shift)
                                       GT_SetGadgetAttrs(hyp_gad,win,NULL,GTCB_Checked,FALSE,TAG_DONE);
                                    /* If Hyp A Trig becomes available, then this isn't necessary */
                                    break;

                                 case HYP_GAD :
                                    GT_GetGadgetAttrs(hyp_gad,win,NULL,GTCB_Checked,&hyp,TAG_DONE);
                                    if(hyp)
                                       GT_SetGadgetAttrs(shift_gad,win,NULL,GTCB_Checked,FALSE,TAG_DONE);
                                    break;

                                 case POINT :
                                    point();
                                    break;

                                 case EXPONENT :
                                    exponent();
                                    break;

                                 case BACKSPACE :
                                    backspace();
                                    break;

                                 case CA :
                                    clear_all();
                                    break;

                                 case CE :
                                    /* May well cause stack underflow but the pull routines catch this and we do not need to know about it */
                                    /* Code to resurrect last value - set buffer=CTT val_stack[val_stack_ptr]; */
                                    clear_entry();
                                    break;

                                 case ADD :
                                 case SUB :
                                 case MUL :
                                 case DIV :
                                 case MOD :
                                 case nPr :
                                 case nCr :
                                 case POW :

                                    operator_2(loop_gad->GadgetID,loop_gad->UserData);
                                    break;

                                 /* Sin, Cos, Tan and the Log commands will need their own section so 
                                 ** that expressions may be input as 3 + tan 5 rather than 3 + 5 tan for example
                                 */
                                 case SIN :
                                 case COS :
                                 case TAN :
                                 case LN :
                                 case LOGBASE10 :
                                 case NEG :
                                 case SQR :
                                 case RECIPROCAL :
                                 case FACTORIAL :
                                 case FIX :
                                 case RANDOM :
                                 case CONSTANT :
                                 case MR :
                                    
                                    pushitem();
                                    
                                    item.op_Prec=(loop_gad->UserData);
                                    item.op_Type=(loop_gad->GadgetID);

                                    value=ConvertToValue(buffer);
                                    if(loop_gad->GadgetID<=TAN)
                                    {
                                       GT_GetGadgetAttrs(shift_gad,win,NULL,GTCB_Checked,&shift,TAG_DONE);
                                       if(shift) item.op_Type++;
                                    }

                                    if((item.op_Type>=SIN)&&(item.op_Type<=TAN))
                                    {
                                       GT_GetGadgetAttrs(hyp_gad,win,NULL,GTCB_Checked,&hyp,TAG_DONE);
                                       if(hyp) item.op_Type+=2;
                                    }

                                    output_operator(item.op_Type);
                                    printf("\t% .15G\n",value);

                                    value=DoSum(NULL,value,item.op_Type);
                                    UpdateDisplay(ConvertToText(value,buffer));
                                    current_position=0;

                                    printf("\t% .15G\n",value);

                                    break;

                                 case EQU :

                                    equals();
                                    break;

                                 case MPLUS :

                                    value=ConvertToValue(buffer);
                                    GT_GetGadgetAttrs(shift_gad,win,NULL,GTCB_Checked,&shift,TAG_DONE);
                                    if(shift)
                                       MMinus(value);
                                    else
                                       MPlus(value);
                                    reset_checkboxes();
                                    break;

                                 case MIN :

                                    MIn(ConvertToValue(buffer));
                                    reset_checkboxes();
                                    break;

                                 default :
                                    break;
                              }
                           }
                        default :
                           break;
                     }
                  }
               }
            }
            
            /* End of program, deallocate resources to end now */
            ClearMenuStrip(win);
            CloseWindow(win);
         }
         else
         {
            notify_error("Could not open Scientific Calculator window");
         }
         }
            FreeMenus(menu);
         }
         FreeVisualInfo(vi);
         FreeGadgets(glist);
      }
   }
   /* Free the memory registers */
   FreeMem((DOUBLE *)memory,sizeof(DOUBLE)*(memsize+1));
   }
   if(stdout)
      Close(stdout);
   
   stdout=old_stdout;
}


/* Update the display gadget in the main window to show the new value */
VOID UpdateDisplay(STRPTR display_string)
{
   GT_SetGadgetAttrs(display_tg,win,NULL,GTTX_Text,display_string,TAG_DONE);
}


/* Do the relevant evaluation for the input operator */
DOUBLE DoSum(DOUBLE value1,DOUBLE value2,UWORD operator)
{
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
         if(IEEEDPTst(value2)!=0)
         {
            return(IEEEDPDiv(value1,value2));
         }
         else
            error("Divide by zero!");
            return(0.0);
         break;

      case MOD :
         return( (DOUBLE) (IEEEDPFix(value1) % IEEEDPFix(value2)) );
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
         return(invtriginit(&IEEEDPAsin(value2)));
         break;

      case SINH :
         return(IEEEDPSinh(triginit(&value2)));
         break;

      case COS :
         return(IEEEDPCos(triginit(&value2)));
         break;

      case ACOS :
         return(invtriginit(&IEEEDPAcos(value2)));
         break;

      case COSH :
         return(IEEEDPCosh(triginit(&value2)));
         break;

      case TAN :
         return(IEEEDPTan(triginit(&value2)));
         break;

      case ATAN :
         return(invtriginit(&IEEEDPAtan(value2)));
         break;

      case TANH :
         return(IEEEDPTanh(triginit(&value2)));

      case FIX :
         return(IEEEDPFix(value2));
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
         return(IEEEDPLOGBASE10(value2));
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
         return((DOUBLE) factorial(IEEEDPFix(value2)));
         break;

      case nCr :
         return((DOUBLE) combination(IEEEDPFix(value1),IEEEDPFix(value2)));
         break;

      case nPr :
         return((DOUBLE) permutation(IEEEDPFix(value1),IEEEDPFix(value2)));
         break;

      case RANDOM :
         return((DOUBLE) RangeRand(value2));
         break;
      
      case CONSTANT :
         return(PI);
         break;
      
      case MR :
         return(MRecall());
         break;

      default :
         break;
   }

}


/* Convert the result into a string for placing in the display */
STRPTR ConvertToText(DOUBLE value,STRPTR buffer)
{
   UBYTE format_type[6];

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

   operator=pull();
   if(stack_err)
   {
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

#ifdef DEBUG
   printf("Push: %ld\t%ld\n",stack[stack_ptr].op_Prec,stack[stack_ptr].op_Type);
#endif

}


/* Pull the top most Operator from the stack */
struct Operator *pull(VOID)
{
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
    if (val_stack_ptr==STACKSIZE) {
        val_stack_err = STACK_FULL;
        error("Overflow error!");
        return;
    }
    val_stack_err = STACK_OK;
    val_stack[++val_stack_ptr] = value;
}


/* Pull the top most value from the value stack */
DOUBLE val_pull(VOID)
{
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
   struct EasyStruct es = {
      sizeof(struct EasyStruct),
      NULL,
      "About Scientific Calculator", /* Title string could be Localized */
      "Scientific Calculator\nVersion %ld.%ld\n\nCopyright � 1996\nTim Ocock.\nAll Rights Reserved.",
      "Ok" /* Could use Locale string for Positive selection */
   };

   struct Requester req = {
      0,0,
      0,0,
      0,0,
      0,0,
      0,0,
      0,0,
      0,0,
      0,0,
      0,0
   };

   SetWindowPointer(win,WA_BusyPointer,TRUE,TAG_DONE);

   Request(&req,win);

   EasyRequest(win,&es,NULL,VERSION,REVISION);

   EndRequest(&req,win);

   SetWindowPointer(win,TAG_DONE);
}


/* Show an error message in the display window */
VOID error(STRPTR text)
{
   /* Eventually have a list of error codes, which are passed instead of duplicatin strings */
   UpdateDisplay(text);
   clear_entry();
}


/* Show an error message in a separate requester, for resource allocation errors
** for example, when the main window is not yet open, or it is locked, for when the
** Memory window is open.
*/
VOID notify_error(STRPTR text)
{
   struct EasyStruct es = {
      sizeof(struct EasyStruct),
      NULL,
      "Scientific Calculator Error", /* Title string could be Localized */
      text,
      "Ok" /* Could use Locale string for Positive selection */
   };

   /* Only attaches the requester to the window if it is open already */
   EasyRequest((win ? win : NULL),&es,NULL,VERSION,REVISION);
}


/* Clear the last input entry into the expression */
VOID clear_entry()
{
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
  return(PI/180*degrees); /* Show rearrangement to get equation in design */
}


/* Convert Radians into Degrees */
DOUBLE rads_degrees(DOUBLE rads)
{
  return(180/PI*rads);
}


/* Print the the text for the last evaluated operator to the Tape */
VOID output_operator(UWORD type)
{
   BYTE *output_buffer;

   switch(type)
   {
      case ADD :
         output_buffer="+";
         break;
         
      case SUB :
         output_buffer="-";
         break;
         
      case MUL :
         output_buffer="�";
         break;
         
      case DIV :
         output_buffer="�";
         break;
         
      case MOD :
         output_buffer="Mod";
         break;
         
      case NEG :
         output_buffer="�";
         break;
         
      case EQU :
         output_buffer="=";
         break;

      case POW :
         output_buffer="x^y";
         break;

      case INVPOW :
         output_buffer="x��y";
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
         output_buffer="sin-�";
         break;

      case SINH :
         output_buffer="sinh";
         break;

      case COS :
         output_buffer="cos";
         break;

      case ACOS :
         output_buffer="cos-�";
         break;

      case COSH :
         output_buffer="cosh";
         break;

      case TAN :
         output_buffer="tan";
         break;

      case ATAN :
         output_buffer="tan-�";
         break;

      case TANH :
         output_buffer="tanh";
         break;

      case EXP :
         output_buffer="e�";
         break;

      case LN :
         output_buffer="ln";
         break;

      case a10X :
         output_buffer="10�";
         break;

      case LOGBASE10 :
         output_buffer="log";
         break;

      case SQR :
         output_buffer="x�";
         break;

      case SQRT :
         output_buffer="sqrt";
         break;

      case RECIPROCAL :
         output_buffer="1�x";
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
         break;
   }

   printf("%s",output_buffer);
}


/* Copy the contents of the display to the Clipboard as FTXT */
VOID copy()
{
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
ULONG choose_mem_slot()
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
   STATIC LONG rc=0;

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

   SetWindowPointer(win,WA_BusyPointer,TRUE,TAG_DONE);

   if(mem_prev_gad)
   {
      struct Requester req = {
         0,0,
         0,0,
         0,0,
         0,0,
         0,0,
         0,0,
         0,0,
         0,0,
         0,0
      };
      Request(&req,win);

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
      EndRequest(&req,win);
   }
   SetWindowPointer(win,TAG_DONE);
   return(rc);
}


/* Store a value in a memory register */
VOID MIn(DOUBLE mem)
{
   /* Use variable names to mark each element */
   LONG choice=0;

   choice=choose_mem_slot();
   if(choice>=0)
      memory[choice]=mem;
   else
      return;
}


/* Add a value to the the value currently stored in a memory register */
VOID MPlus(DOUBLE mem)
{
   LONG choice=0;

   choice=choose_mem_slot();
   if(choice>=0)
      memory[choice]+=mem;
   else
      return;
}


/* Subtract a value from the value currently stored in a memory register */
VOID MMinus(DOUBLE mem)
{
   LONG choice=0;

   choice=choose_mem_slot();
   if(choice>=0)
      memory[choice]-=mem;
   else
      return;
}


/* Retrieve a value from a memory register */
DOUBLE MRecall()
{
   LONG choice=0;

   choice=choose_mem_slot();
   if(choice>=0)
      return memory[choice];
   else
      return(0.0);
}


/* Ensure an angle is in the correct format befoe passing it to a 
** trigonometric function
*/
DOUBLE triginit(DOUBLE *operand)
{
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
LONG factorial(LONG operand)
{
   /* This routine can factorial any number until it runs out of stack */
   if(operand<=0.0)
      return(1.0);
   else
      return(operand*factorial(operand-1));
}


/* The Combination function */
LONG combination(LONG operand1, LONG operand2)
{
   return(factorial(operand1)/(factorial(operand1-operand2)*factorial(operand2)));
}


/* The Permutation function */
LONG permutation(LONG operand1, LONG operand2)
{
   return(factorial(operand1)/factorial(operand1-operand2));
}


/* Convert a string representation of a number into the actual number */
DOUBLE ConvertToValue(STRPTR string)
{
   DOUBLE value;
   BYTE *tail;

   switch(current_base)
   {
      case BASE10 :
         value=atof(string);
         break;

      case BASE16 :
      case BASE8 :
      case BASE2 :
         sprintf(string,"%ld",IEEEDPFix(value));
         value=(DOUBLE) strtol(string,&tail,current_base);
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

   value=ConvertToValue(buffer);

   val_push(value);
   printf("\t% .15G\n",value);
   
   item.op_Prec=(APTR) PREC_EQUAL;
   item.op_Type=EQU;
   item_ready=FALSE;

   push(item);

   output_operator(EQU);
   printf("\t% .15G\n",val_stack[val_stack_ptr]);

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

   value=ConvertToValue(buffer);
   val_push(value);
   printf("\t% .15G\n",value);

   GT_GetGadgetAttrs(shift_gad,win,NULL,GTCB_Checked,&shift,TAG_DONE);

   item.op_Prec=userdata;
   item.op_Type=id;
   if(id==POW)
   {
      if(shift) item.op_Type++;
   }
   item_ready=TRUE;

#ifdef DEBUG
   printf("Type = %d,Shift = %d\n",item.op_Type,shift);
#endif

   current_position=0;
   pointused=FALSE;
   expused=FALSE;
   reset_checkboxes();
}


/* Delete the last entered digit from the display */
VOID backspace()
{
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
   if(!pointused&&current_base==BASE10)
   {
      display_digit('.');
      pointused=TRUE;
   }
}


/* Enter the letter 'E' into the display for scientific notation */
VOID exponent()
{
   if(!expused&&current_base==BASE10)
   {
      display_digit('E');
      expused=TRUE;
   }
}


/* Turn both Hyp and Shift gadgets to off */
VOID reset_checkboxes()
{
   GT_SetGadgetAttrs(shift_gad,win,NULL,GTCB_Checked,FALSE,TAG_DONE);
   GT_SetGadgetAttrs(hyp_gad,win,NULL,GTCB_Checked,FALSE,TAG_DONE);
}


/* Push an operator onto the stack if one has been entered */
VOID pushitem()
{
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
   if(current_position<=16)
   {
      buffer[current_position]=digit;
      current_position++;
      buffer[current_position]=NULL;
      UpdateDisplay(buffer);
   }
}

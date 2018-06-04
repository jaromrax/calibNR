/* A tiny BASIC interpreter */
/*   i  started with windows c++ program :
 * http://www.programmersheaven.com/download/16060/download.aspx
 *Submitted By: WEBMASTER
 **
 *
 * problems - one character variables! (but it is fast)
 *          - if expresions  >=  <=
 *               help with these  @ ~ $ _ [ ] ' \ : ? ! { } `
 *               already used   , ; " + - *  ^ %  & |
 *               not good candidates:
 *                         [] can be used by mistake or ARRAY
 *                         {} can be used by mistake
 *                         \  backslash...problems of bkslashing it
 *                         '` problems of bkslashing it 
 *                         
 *                         
 *******************************************
 *  I added
 *******************************************
 *
 *          CONTINUE    
 *          # , COMMENT
 *          evaluation of conditions arithmeticaly
 *2byte symbol ... 1byte internal representation  
 *          == ... ~
 *          && ... @
 *          || ... $
 *          >= ... :
 *          <= ... _
 *          != ... !
 *******************************************
 *   long variable names:
 *         max 26 single letters  
 *         max 100 additional words, words+numbers
 *             max 12 characters length
 ********************************************
 *  Performance
 *         parsing 10-20 var names + 5-7 conditions + division ~ 10us on 1.6GHz
 *
 */
/* stdio = printf
 * stdlib = calloc, exit, free 
 */
#include <stdio.h>
#include <stdlib.h>
//
#include <setjmp.h>
//  return from a long jump - take care when u use signals 
#include <math.h>
//  pow() 
#include <ctype.h>
// ctype: isalnum, isalpha, isdigit....
#include <string.h>
// critical


#define NUM_LAB 100
#define LAB_LEN 10 
#define FOR_NEST 25
#define SUB_NEST 25
#define PROG_SIZE 10000

// tok_type
#define DELIMITER  1
#define VARIABLE  2
#define NUMBER    3
#define COMMAND   4
#define STRING	  5
#define QUOTE	  6

//bas_tok
#define PRINT 1
#define INPUT 2
#define IF    3
#define THEN  4
#define FOR   5
#define NEXT  6
#define TO    7
#define GOTO  8
#define EOL   9
#define FINISHED  10
#define GOSUB 11
#define RETURN 12
#define END 13

#define COMMENT 14
#define CONTINUE 15


char *prog;  /* holds expression to be analyzed */
jmp_buf e_buf; /* hold environment for longjmp() */

/*
double variables[26]= {    // 26 user variables,  A-Z 
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  0, 0, 0, 0, 0, 0
};
*/


//  #define  MAXVARNAMES 100
#define MAXVARIABLES 400

int varnamelast=0;  // keeps in mind the end of the var buffer
int currentline=0;
/*
 * CORRECT  DECLARATIONS  OF  CHAR *
 */
//char achar[5]={"ahoj"};  //ok
//char achar[5]={"ahoj"};//ok
//char achar[5]={"ahojkut"};//not ok
//char  varname[MAXVARNAMES][12]={   //ok  each field has 11 chars+null at max

char  varname[MAXVARIABLES][13];
double variables[MAXVARIABLES];


struct commands { /* keyword lookup table */
  char command[20];
  char tok;
} table[] = { /* Commands must be entered lowercase */
  {"print", PRINT}, /* in this table. */
  { "input", INPUT},
  { "if", IF},
  { "then", THEN},
   {"goto", GOTO},
   {"for", FOR},
   { "next", NEXT},
   { "to", TO},
   { "gosub", GOSUB},
   { "return", RETURN},
  {  "end", END},
   { "comment", COMMENT},
  {  "continue", CONTINUE},
   { "", END}  /* mark end of table */
};
//    #   makes  comment





char bas_token[80];
char bas_token_type, bas_tok;

struct label {
  char name[LAB_LEN];
  char *p;  /* points to place to go in source file*/
};
struct label label_table[NUM_LAB];


// I do not understand this declaration.
// works ok with g++
//  fails under root
//char *find_label(), *gpop();



struct for_stack {
  int var; /* counter variable */
  double target;  /* target value */
  char *loc;
} fstack[FOR_NEST]; /* stack for FOR/NEXT loop */



struct for_stack fpop();



char *gstack[SUB_NEST];	/* stack for gosub */



int ftos;  /* index to top of FOR stack */
int gtos;  /* index to top of GOSUB stack */



  double find_var(char* s);
  void unary(char o, double* r);
  void arith(char o, double* r, double* h);
  void primitive(double* result);
  void level6(double* result);
  void level5(double* result);
  void level4(double* result);
  void level3(double* result);
  void level2(double* result);
  int iswhite(char c);
  int isdelim(char c);
//----------    
  double lookup_table2(char* s);         //get formula value,calls lookup_table(s)
  int lookup_table_tk(const char* token);//find var pos.(not in basic)
  int lookup_table();                    //get/set var in table
//---------

  int look_up(char* s);


  void putback(); 
  int get_token();
  void serror(int error);
  void get_exp(double* result);
  char *gpop();
  void gpush(char* s);
  void greturn();
  void gosub();
  void input();
  struct for_stack fpop();
  void fpush(void* i);
  void next();
  void exec_for();
  void exec_if();
  void label_init();
  void exec_goto();
  char *find_label(char* s);
  int get_next_label(char* s);
  void find_eol();
  void scan_labels();
  void print();
//  int assignment(int var, double value);
  int assignment();
  int  load_program(char* p, char* fname);







/*
 * when used within a body, you need to reset the basic environment
 */
int init_namespace(){
  int i;
  for ( i=0;i<MAXVARIABLES;i++){    strcpy(varname[i],(char*)"            \0");   }

  for ( i=0;i<MAXVARIABLES;i++){    variables[i]=0;   }
  //  for ( i=0;i<MAXVARIABLES;i++){    strncpy(varname[i],"            ",12);  }
  varnamelast=0;
  ftos = 0; /* initialize the FOR stack index */
  gtos = 0; /* initialize the GOSUB stack index */
  return 0;
}//init_namespace







int run_program(char* p_buf){
  int i;

  //  I REMOVED THIS AND PUT IT IN FRONT OF THE CALL...
  //NMS-REMOVAL  init_namespace();// Do you need it? May be you want to use the same namespace

  prog = p_buf;
  scan_labels(); /* find the labels in the program */
  ftos = 0; /* initialize the FOR stack index */
  gtos = 0; /* initialize the GOSUB stack index */
  do {
    bas_token_type = get_token();
    // printf("ttype=%d bas_tok=%d <%s>\n", bas_token_type, bas_tok, bas_token);
    /* check for assignment statement */
    if(bas_token_type==VARIABLE) {
      putback(); /* return the var to the input stream */
      assignment(); /* must be assignment statement */
    }
    else /* is command */
      switch(bas_tok) {
        case PRINT:
	  print();
  	  break;
        case GOTO:
	  exec_goto();
	  break;
	case IF:
	  exec_if();
	  break;
	case FOR:
	  exec_for();
	  break;
	case NEXT:
	  next();
	  break;
  	case INPUT:
	  input();
	  break;
        case GOSUB:
	  gosub();
	  break;
	case RETURN:
	  greturn();
	  break;
	case COMMENT:
	  do{
	  get_token();
	  if(bas_tok==EOL || bas_tok==FINISHED) break; 
	  } while (1==1);
	  break;
	case CONTINUE:
	  do{
	  get_token();
	  if(bas_tok==EOL || bas_tok==FINISHED) break; 
	  } while (1==1);
	  break;
        case END:
	  printf("variables----------------------------%s\n","");
	  for (i=0;i<varnamelast;i++){
	    printf("%14s - %19.7f\n", varname[i] , variables[i] );
	  }
	  exit(0);
      }
  } while (bas_tok != FINISHED);
  return 0;
}// run_program






 /************/
 /* M A I N  */
 /************/





int qmain(int argc, char* argv[]){

  //  char in[80];
  // int answer;
  char *p_buf;
  // char *t;

  if(argc!=2) {
    printf("usage: run <filename>\n");
    exit(1);
  }

  /* allocate memory for the program */
  if(!(p_buf=(char *) malloc(PROG_SIZE))) {
    printf("allocation failure");
    exit(1);
  }
  
  /* load the program to execute */
  if(!load_program(p_buf,argv[1])) exit(1);

  // serror can return here and die
  if(setjmp(e_buf)) exit(1); /* initialize the long jump buffer */

  init_namespace();
  run_program(p_buf);// THIS RUNS THE PROGRAM

  /*  run_program("print \"THIS IS END\"\n");// THIS RUNS THE PROGRAM
  run_program("print 1+2+3+4+5");// THIS RUNS THE PROGRAM
  run_program("print 1.01+2.02+3.03+4.04+5.05");// THIS RUNS THE PROGRAM
  run_program("print \"THIS IS END\"\n");// THIS RUNS THE PROGRAM
  */
  return 0;
}//MAIN-------------------------------






/* Load a program. */
int  load_program(char* p, char* fname){
  //char *p;
  //char *fname;
  //{
  FILE *fp;
  int i=0;

  if(!(fp=fopen(fname, "rb"))) return 0;
  
  i = 0;
  do {
    *p = getc(fp);
    p++; i++;
  } while(!feof(fp) && i<PROG_SIZE);
  *(p-1) = '\0'; /* null terminate the program */
  fclose(fp);
  return 1;
}//-------------------load













/***************************************************
 *
 *    find   a   value   of  a variable:
 *     do not used in basic
 */


int lookup_table_tk(const char* token){

  int position=-1;
  int i;

  for (i=0;i<varnamelast;i++){
    if ( !strcmp(varname[i],token) ){  break;  } 
    if ( i+1>=MAXVARIABLES ){  break;  } //save your ass and pileup the 100s variable
  }
// printf("%s broke at pos. %d \n",   token,  i );
  if (i>=varnamelast){
    // DEBUG   PRINTF:
    //      printf("defining a new variable <%s> at pos %d\n",  token, varnamelast  );
    strncpy( varname[ varnamelast ], token, 12 );// TAKE 1st 12 CHARACTERS FOR NAME
    varnamelast++;
    position=i;
  }else{// ALREADY EXISTS,  FOUND
    position=i;
  }
  return position; // position in variables[126]
}//variable lookup  ---- easy way



/*********************************
 *     lookup_table (s)     enables to pick a variable from/to a namespace
 *     lookup_table2(s)     enables to interpret a simple expression
 *   NOT to be used in  basic.....
 */

/************************************************
 * this must not be used inside the basic - this is
 * only interpolation of the variable
 *          OR
 *  solving a formula
 *
 * Look up a a bas_token's internal representation in the
   bas_token table.
   *
   * HO HO HO. What if s is formula? with   *+-/
   *   
   *
*/
double lookup_table2(char* s){
  //  register int i,j;
   int i;

  /*
   *   solve a formula or just call lookup_table( s )
   */
  //    printf("...........%s..............\n", s );
  if (   (strpbrk( s ,  "+" )!=NULL)||   
	 (strpbrk( s ,  "-" )!=NULL)||  
	 (strpbrk( s ,  "*" )!=NULL)||    
	 (strpbrk( s ,  "/" )!=NULL)    	 
     ){
    //   printf("............. found a formula\n%s","");
  double value;
  prog=s;
  bas_token_type = get_token(); putback();
  get_exp(&value);
  //  printf("toktyp=%d, value = %lf\n",bas_token_type, value );
  return value;
  }// expr
  else{ // standard   lookup_table( s )
	int io=lookup_table_tk(s);
	//  printf("............. NOT a formula, the var sits at pos %d\n",  io  );
	return variables[io];
  }

  return 0.0; /* unknown command */
}//------------------------lookup_table  2 







int lookup_table(){

  int position=-1;
  int i;

  //  if ( strlen(bas_token)==1  ){   return toupper(*bas_token)-'A';   } // all 26 default  a..z

  // multicharacter variables names
  for (i=0;i<varnamelast;i++){
    if ( !strcmp(varname[i],bas_token) ){  break;  } 
    if ( i+1>=MAXVARIABLES ){  break;  } //save your ass and pileup the 100s variable
  }
// printf("%s broke at pos. %d \n",   bas_token,  i );

  if (i>=varnamelast){
    // DEBUG   PRINTF:
    //      printf("defining a new variable <%s> at pos %d\n",  bas_token, varnamelast  );
    strncpy( varname[ varnamelast ], bas_token, 12 );// TAKE 1st 12 CHARACTERS FOR NAME
    varnamelast++;
    position=i;
  }else{// ALREADY EXISTS,  FOUND
    position=i;
  }
  //  return position+26; // position in variables[126]
  return position; // position in variables[126]
}//variable lookup 








/* Assign a variable a value. */
int assignment(){
  int var; double value;

  /* get the variable name */
  get_token();
  // printf("new assignement <%s>\n", bas_token );
  if(!isalpha(*bas_token)) {
    serror(4);
    return 0;
  }
  /*
   *  howto make a table of variables?
   */
  var = lookup_table();// toupper(*bas_token)-'A';

  //  printf("assign <%s> retvar=%d ", bas_token, var );

  /* get the equals sign */
  get_token();
  // printf(" <%s> ", bas_token );
  if(*bas_token!='=') {
    //    printf("assignment%s\n","");
    serror(3);
    return 0;
  }

  /* get the value to assign to var */
  //  printf(" going to get_exp with bas_token <%s> \n ", bas_token );
 get_exp(&value);
 //  printf(" = <%lf> (outcome)\n ", value );

  /* assign the value */
  variables[var] = value;
  //   printf("#<%d> val=<%lf>\n",  var, value );
  return 0; // ????? 
}//--------------------------assignment




/* Execute a simple version of the BASIC PRINT statement */
void print()
{
  double answer;
  int len=0, spaces;
  char last_delim; //  warning from compile-may be used uninited
  // I TRY TO MAKE PUT SOME VALUE FOR COMPILETIME
  last_delim = *bas_token; 

  do {
    get_token(); /* get next list item */
    if(bas_tok==EOL || bas_tok==FINISHED) break;
    if(bas_token_type==QUOTE) { /* is string */
      printf("%s\n",bas_token);
      len += strlen(bas_token);
      get_token();
    }
    else { /* is expression */
      putback();
      get_exp(&answer);
      get_token();
      len += printf("%g", answer);
    }
    last_delim = *bas_token; 

    if(*bas_token==';') {
      /* compute number of spaces to move to next tab */
      spaces = 8 - (len % 8); 
      len += spaces; /* add in the tabbing position */
      while(spaces) { 
	printf(" ");
        spaces--;
      }
    }
    else if(*bas_token==',') /* do nothing */;
    else if(bas_tok!=EOL && bas_tok!=FINISHED) serror(0); 
  } while (*bas_token==';' || *bas_token==',');

  if(bas_tok==EOL || bas_tok==FINISHED) {
    if(last_delim != ';' && last_delim!=',') printf("\n");
  }
  else serror(0); /* error is not , or ; */
  //  return 0;
}//print--------------------------------------------









/* Find all labels. */
void scan_labels()
{
  int addr;
  char *temp;

  label_init();  /* zero all labels */
  temp = prog;   /* save pointer to top of program */

  /* if the first bas_token in the file is a label */
  get_token();
  if(bas_token_type==NUMBER) {
    strcpy(label_table[0].name,bas_token);
    label_table[0].p=prog;
  }

  find_eol();
  do {     
    get_token();
    if(bas_token_type==NUMBER) {
      addr = get_next_label(bas_token);
      if(addr==-1 || addr==-2) {
          (addr==-1) ?serror(5):serror(6);
      }
      strcpy(label_table[addr].name, bas_token);
      label_table[addr].p = prog;  /* current point in program */
    }
    /* if not on a blank line, find next line */
    if(bas_tok!=EOL) find_eol();
  } while(bas_tok!=FINISHED);
  prog = temp;  /* restore to original */
}





/* Find the start of the next line. */
void find_eol()
{
  while(*prog!='\n'  && *prog!='\0') ++prog;
  if(*prog) prog++;
}







/* Return index of next free position in label array. 
   A -1 is returned if the array is full.
   A -2 is returned when duplicate label is found.
*/
int get_next_label(char* s){
   int t;

  for(t=0;t<NUM_LAB;++t) {
    if(label_table[t].name[0]==0) return t;
    if(!strcmp(label_table[t].name,s)) return -2; /* dup */
  }

  return -1;
}





/* Find location of given label.  A null is returned if
   label is not found; otherwise a pointer to the position
   of the label is returned.
*/
char *find_label(char* s)
{
   int t;

  for(t=0; t<NUM_LAB; ++t) 
    if(!strcmp(label_table[t].name,s)) return label_table[t].p;
  char ret[2]="\0";
  // ----- i tried to return \0  but i needed to allocate \0. 
  //char *StrResult = malloc( 0 + 2 );
  char *StrResult = new char[2];
  StrResult[0]='\0';
  return StrResult;
  //return ret;
  //return NULL;
  // *return '\0'; /* error condition */ 
}






/* Execute a GOTO statement. */
void exec_goto()
{

  char *loc;

  get_token(); /* get label to go to */
  /* find the location of the label */
  loc = find_label(bas_token);
  if(loc[0]=='\0')
    serror(7); /* label not defined */

  else prog=loc;  /* start program running at that loc */
}





/* Initialize the array that holds the labels. 
   By convention, a null label name indicates that
   array position is unused.
*/
void label_init()
{
   int t;

  for(t=0; t<NUM_LAB; ++t) label_table[t].name[0]='\0';
}






/* Execute an IF statement. */
void exec_if()
{
  int   cond;
  double x;
  //  char op;

  //  printf("if:ini bas_token=%s \n", bas_token );
  get_exp(&x); // get left expression 

  // ORIGINAL
  /*
  get_token(); // get the operator 
  if(!strchr("=<>", *bas_token)) {
    serror(0); // not a legal operator 
    return;
  }
  op=*bas_token;  

  get_exp(&y); // get right expression 
*/
  //  printf("if: bas_token=%s  x=%4d \n", bas_token,  x );

  //NEUMIME =
  cond = (int) x;  //20100422
  // cond=int(x);  //  expresion x gives 1 or 0
  // determine the outcome 
  /*  cond = 0;
  switch(op) {
    case '<':
      if(x<y) cond=1;
      break;
    case '>':
      if(x>y) cond=1;
      break;
    case '=':
      if(x==y) cond=1;
      break;
  }
  */


  if(cond) { /* is true so process target of IF */
    get_token();
    if(bas_tok!=THEN) {
      serror(8);
      return;
    }/* else program execution starts on next line */
  }
  else find_eol(); /* find start of next line */
}









/* Execute a FOR loop. */
void exec_for()
{
  struct for_stack i;
  double value;

  get_token(); /* read the control variable */
  if(!isalpha(*bas_token)) {
    serror(4);
    return;
  }

  i.var=lookup_table();//toupper(*bas_token)-'A'; /* save its index */

  get_token(); /* read the equals sign */
  if(*bas_token!='=') {
    serror(3);
    return;
  }

  get_exp(&value); /* get initial value */

  variables[i.var]=value;

  get_token();
  if(bas_tok!=TO) serror(9); /* read and discard the TO */

  get_exp(&i.target); /* get target value */

  /* if loop can execute at least once, push info on stack */
  if(value>=variables[i.var]) { 
    i.loc = prog;
    fpush( &i);// 20100422  -  i
  }
  else  /* otherwise, skip loop code altogether */
    while(bas_tok!=NEXT) get_token();
}





/* Execute a NEXT statement. */
void next()
{
  struct for_stack i;

  i = fpop(); /* read the loop info */

  variables[i.var]++; /* increment control variable */
  if(variables[i.var]>i.target) return;  /* all done */
  fpush( &i );  /* otherwise, restore the info */
  prog = i.loc;  /* loop */
}





/* Push function for the FOR stack. */
/*
 *  20100422 - fpush( for_stack i)
 */
void fpush(void * arg){
//struct for_stack i;
  struct for_stack *i = (struct for_stack *) arg;


   if(ftos>FOR_NEST)
    serror(10);

   fstack[ftos]=*i;// 20100422  -  i
  ftos++;
}

struct for_stack fpop()
{
  ftos--;
  if(ftos<0) serror(11);
  return(fstack[ftos]);
}





/* Execute a simple form of the BASIC INPUT command */
void input()
{
  int  var;
  double i;

  get_token(); /* see if prompt string is present */
  if(bas_token_type==QUOTE) {
    printf("%s\n",bas_token); /* if so, print it and check for comma */
    get_token();
    if(*bas_token!=',') serror(1);
    get_token();
  }
  else printf("? "); /* otherwise, prompt with / */
  var = lookup_table();//toupper(*bas_token)-'A'; /* get the input var */

  //HA - UNUSED INPUT!!
  int shit=scanf("%lf", &i); /* read input */
  printf( " %d :/\n", shit );

  variables[var] = i; /* store it */
}//-----input




/* Execute a GOSUB command. */
void gosub()
{
  char *loc;
  char zeroc='\0';
  
  get_token();
  /* find the label to call */
  loc = find_label(bas_token);
  if( loc[0]=='\0')
    serror(7); /* label not defined */
  else {
    gpush(prog); /* save place to return to */
    prog = loc;  /* start program running at that loc */
  }
}





/* Return from GOSUB. */
void greturn()
{
   prog = gpop();
}





/* GOSUB stack push function. */
void gpush(char* s){
  gtos++;

  if(gtos==SUB_NEST) {
    serror(12);
    return;
  }

  gstack[gtos]=s;

}





/* GOSUB stack pop function. */
char *gpop()
{
  if(gtos==0) {
    serror(13);
    return 0;
  }

  return(gstack[gtos--]);
}





/* Entry point into parser. */
void get_exp(double* result){

  get_token();  
  //  printf("getexpparser: <%s> ...to level2 \n", bas_token );

  if(!*bas_token) {
    serror(2);
    return;
  }
  level2(result);
  //  printf("Parser: res=<%lf> \n", result );
  putback(); /* return last bas_token read to input stream */
}






/* display an error message */
void serror(int error){
  static const char *e[]= {   
    "syntax error", 
    "unbalanced parentheses", 
    "no expression present",
    "equals sign expected",
    "not a variable",
    "Label table full",
    "duplicate label",
    "undefined label",
    "THEN expected",
    "TO expected",
    "too many nested FOR loops",
    "NEXT without FOR",
    "too many nested GOSUBs",
    "RETURN without GOSUB"
  }; 
  printf("last token--->>> tok=%d, type==%d, %s\n",  bas_tok, bas_token_type, bas_token );
  printf("last var  --->>> %s\n", varname[varnamelast-1]  );
  printf("line      --->>> %d(doesnt work)\n", currentline  );
  printf("%s\n", e[error]); 
  longjmp(e_buf, 1); /* return to save point */
}








/******************************/
/* VERY IMPORTANT  GET  BAS_TOKEN */
/******************************/


/* Get a bas_token. */
int get_token(){

   char *temp;

  bas_token_type=0; bas_tok=0;
  temp=bas_token;

  //    printf( "gettoken: <%c>\n", *prog );

  if(*prog=='\0') { /* end of file */
    *bas_token=0;
    bas_tok = FINISHED;
    return(bas_token_type=DELIMITER);
  }

  while(iswhite(*prog)) ++prog;  /* skip over white space */
  /*
  if(*prog=='\r') { /
    ++prog; ++prog;
    bas_tok = EOL; *bas_token='\r';
    bas_token[1]='\n'; bas_token[2]=0;
    return (bas_token_type = DELIMITER);
  }

*/
    if(*prog=='\n') { // lf 
    ++prog;
    //currentline++; // just for debug
    bas_tok = EOL; *bas_token='\n';
    bas_token[1]=0; 
    // printf("...EOL found ...%s" , "");
    return (bas_token_type = DELIMITER);
  }


  if(*prog=='#') { /* lf */
    ++prog;
    bas_tok = COMMENT; *bas_token='#';
    bas_token[1]=0;
    return (bas_token_type = DELIMITER);
  }



 
  if(strchr("!+-*^/%=;(),><&|", *prog)){ /* delimiter */
    *temp=*prog;

    //if there is NOT double delimiter
    if(strchr("+-*^/%;(),", *prog)){ /*  delimiter */
      // if  (strchr("(", *prog)){ printf("delim found %c \n", *prog); }
      //  if  (strchr(")", *prog)){ printf("delim found %c \n", *prog); }
      prog++; /* advance to next position */
      temp++;
      *temp=0; 
       return (bas_token_type=DELIMITER);
    }// was single delimiter
 




    /*  DOUBLE DELIMITERS WILL BE SINGLFIED.......HERE
     *
     */
    // printf("case of dd may come <%c> <%c>\n", *prog, prog[1] );

    if((strchr("=", *prog))&&(strchr("=",prog[1]) )){ /* double delimiter   == is ~  */
      bas_token[0]='~'; bas_token[1]='\0';
      prog++; /* advance to next position */
      temp++;
        prog++; /* advance to next position */
      temp++;
    *temp=0; 
    return (bas_token_type=DELIMITER);
  }

    if((strchr("!", *prog))&&(strchr("=",prog[1]) )){ /* double delimiter   != is !  */
      bas_token[0]='!'; bas_token[1]='\0';
      prog++; /* advance to next position */
      temp++;
       prog++; /* advance to next position */
      temp++;
    *temp=0; 
    return (bas_token_type=DELIMITER);
   }

    if((strchr("&", *prog))&&(strchr("&",prog[1]) )){ /* double delimiter && is @ */
      bas_token[0]='@'; bas_token[1]='\0';
      prog++; /* advance to next position */
      temp++;
      prog++; /* advance to next position */
      temp++; 
    *temp=0; 
    return (bas_token_type=DELIMITER);
    }

    if((strchr("|", *prog))&&(strchr("|",prog[1]) )){ /* double delimiter  || is $*/
      bas_token[0]='$'; bas_token[1]='\0';
      prog++; /* advance to next position */
      temp++;
        prog++; /* advance to next position */
      temp++;
    *temp=0; 
    return (bas_token_type=DELIMITER);
  }

    if((strchr(">", *prog))&&(strchr("=",prog[1]) )){ /* double delimiter  >= is :*/
      bas_token[0]=':'; bas_token[1]='\0';
      prog++; /* advance to next position */
      temp++;
        prog++; /* advance to next position */
      temp++;
    *temp=0; 
    return (bas_token_type=DELIMITER);
  }

    if((strchr("<", *prog))&&(strchr("=",prog[1]) )){ /* double delimiter  <= is _ */
      bas_token[0]='_'; bas_token[1]='\0';
      prog++; /* advance to next position */
      temp++;
        prog++; /* advance to next position */
      temp++;
    *temp=0; 
    return (bas_token_type=DELIMITER);
  }


    
      prog++; /* advance to next position */
      temp++;
    *temp=0; 
    return (bas_token_type=DELIMITER);
  }// DELIMITERS - DOUBLE DELIMITERS-------------------END-----DOUBLE
    


  /*   20100419 - now  |  and  &  are unabiguous
   *
   */

    //if there is NOT double delimiter
    if(strchr("|&", *prog)){ /*  delimiter */
      // if  (strchr("(", *prog)){ printf("delim found %c \n", *prog); }
      //  if  (strchr(")", *prog)){ printf("delim found %c \n", *prog); }
      prog++; /* advance to next position */
      temp++;
      *temp=0; 
       return (bas_token_type=DELIMITER);
    }// was single delimiter
 







  if(*prog=='"') { /* quoted string */
    prog++;
    while(*prog!='"'&& *prog!='\n') *temp++=*prog++;
    if(*prog=='\n') serror(1);
    prog++;*temp=0;
    return(bas_token_type=QUOTE);
  }
  

  if(isdigit(*prog)) { /* number */
    while(!isdelim(*prog)) *temp++=*prog++;
    *temp = '\0';
    // printf("temp=%s\n", bas_token);
    return(bas_token_type = NUMBER);
  }


  if(isalpha(*prog)) { /* var or command */
    while(!isdelim(*prog)) *temp++=*prog++;
    bas_token_type=STRING;
  }

  *temp = '\0';

  /* see if a string is a command or a variable */
  if(bas_token_type==STRING) {
    bas_tok=look_up(bas_token); /* convert to internal rep */
    if(!bas_tok) bas_token_type = VARIABLE;
    else bas_token_type = COMMAND; /* is a command */
  }
  return bas_token_type;
}//------------------------get_token










/* Return a bas_token to input stream. */
void putback() 
{

  char *t; 

  t = bas_token; 
  for(; *t; t++) prog--; 
}//-------------------------putback




















int look_up(char* s){
  //  register int i,j;
   int i;
  char *p;

  /* convert to lowercase */
  p = s;
  while(*p){ *p = tolower(*p); p++; }

  /* see if bas_token is in table */
  for(i=0; *table[i].command; i++)
      if(!strcmp(table[i].command, s)) return table[i].tok;



  return 0; /* unknown command */
}//------------------------look up












/* Return true if c is a delimiter. */
int isdelim(char c){
  //  stop number or variable if delimiter
  // I have added ! to stop in front of !=
  if(strchr(" ;,+-<>/*%^=()&|!", c) || c==9 || c=='\n' || c==0) 
    return 1;  
  return 0;
}




/* Return 1 if c is space or tab. */
int iswhite(char c){
  if(c==' ' || c=='\t') return 1;
  else return 0;
}//------------------iswhite





/*  Add or subtract two terms. */
void level2(double* result){
   char  op; 
  double hold; 

  // printf("level2:  came with bas_token %s\n",  bas_token );

  level3(result); 
  //printf("level2:  ret lev3  bas_token %s, result=%lf\n",  *result);
  //  ==    >=   <=  !=
  //  ~     :    _    !
  while((op = *bas_token) == '+' || op == '-' || op == '<' || op == '>' || op == '~' || op == ':' || op == '_' || op == '!'  ) {
    //printf("arith: op=%s    \n", bas_token );
    get_token(); 
    //printf("arith: op=%s,    \n", bas_token );
    level3(&hold); 
    arith(op, result, &hold);
  }
}

/* Multiply or divide two factors. */
void level3(double* result){
   char  op; 
  double hold;

  level4(result); 
  while((op = *bas_token) == '*' || op == '/' || op == '%' || op == '@' || op == '$' || op == '|' || op == '&') {
    get_token(); 
    level4(&hold); 
    arith(op, result, &hold); 
  }
}

/* Process integer exponent. */
void level4(double* result){
  double hold; 

  level5(result); 
  if(*bas_token== '^') {
    get_token(); 
    level4(&hold); 
    arith('^', result, &hold); 
  }
}

/* Is a unary + or -. */
void level5(double* result){
   char  op; 

  op = 0; 
  if((bas_token_type==DELIMITER) && (*bas_token=='+' || *bas_token=='-') ) {
    op = *bas_token; 
    get_token(); 
  }
  level6(result); 
  if(op)
    unary(op, result); 
}

/* Process parenthesized expression. */
void level6(double* result){
  if((*bas_token == '(') && (bas_token_type == DELIMITER)) {
    get_token(); 
    //  printf("level6 <%s>, descending into level2\n", bas_token );
    level2(result); 
    //  printf("level6 <%s>, back       from level2\n", bas_token );
    if(*bas_token != ')')
      serror(1);
    get_token(); 
  }
  else
    primitive(result);
}

/* Find value of number or variable. */
void primitive(double* result){

  //  printf("primit:  came with bas_token %s\n",  bas_token );

  switch(bas_token_type) {
  case VARIABLE:
    *result = find_var(bas_token);
    get_token(); 
    return; 
  case NUMBER:
    //   printf("primi2:  came with bas_token %s\n",  bas_token );  
    //    printf("primi2:  came with bas_token %lf\n",  atof(bas_token) );  
    *result = atof(bas_token);
    get_token();
    return;
  default:
    serror(0);
  }
}

/* Perform the specified arithmetic. */
void arith(char o, double* r, double* h){
   int t, ex;
  int a,b;

  //  printf("arith operator : %c\n", o);

  switch(o) {
  case '@': // logical and @   &&
       if ((*r==1)&&(*h==1)){ *r=1; }else{*r=0;}; 
      break; 
  case '$': // logical or  $  ||
       if ((*r==1)||(*h==1)){ *r=1; }else{*r=0;}; 
      break; 

  case '!':  //   !=
      *r =  (*r!=*h); 
      break; 
  case ':':  //   >=
      *r =  (*r>=*h); 
      break; 
  case '_': //   <=
      *r =  (*r<=*h); 
      break; 

  case '~'://     ==
      *r =  (*r==*h); 
      break; 
  case '>'://      >
      *r =  (*r>*h); 
      break; 
  case '<'://      <
      *r =  (*r<*h); 
      break; 

    case '-':
      *r = *r-*h; 
      break; 
    case '+':
      *r = *r+*h; 
      break; 

    case '*':  
      *r = *r * *h; 
      break; 

    case '/':
      *r = (*r)/(*h);
      break; 

  case '%'://     modulo
      t = (*r)/(*h); 
      *r = *r-(t*(*h)); 
      break; 

  case '|'://     
    a=(int)*r;
    b=(int)*h;
    *r = 1.0* (a|b); 
      break; 

  case '&'://     
    a=(int)*r;
    b=(int)*h;
    *r = 1.0* (a&b); 
    //      *r = (*r)&(*h); 
      break; 

  case '^'://   exponential
      ex = *r; 
      if(*h==0) {
        *r = 1; 
        break; 
      }
      for(t=*h-1; t>0; --t) *r = (*r) * ex;
      break;       
  }
}

/* Reverse the sign. */
void unary(char o, double* r){
  if(o=='-') *r = -(*r);
}




/* Find the value of a variable. */
double find_var(char* s){
  int position;
  if(!isalpha(*s)){
    serror(4); /* not a variable */
    return 0;
  }
  position=lookup_table();
  //  return variables[toupper(*bas_token)-'A'];
  return variables[position];
}


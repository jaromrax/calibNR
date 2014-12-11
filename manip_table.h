//#include <TString.h>
//#include <TObjString.h>
//#include <TObject.h>
// interensting - needs this else doesnot  compile..... Whaaaauuuuu
// moreover - no TString  TObjString  needed anymore if TChain declared
#include <TChain.h>
// .............    waste.......
//#include <TROOT.h>
//#include <TFile.h>
//#include <TSelector.h>
//-------------tminuit-----fit
#include <TMinuit.h>
//----   tgraph plot
#include <TH2F.h>
#include <TLine.h>
#include <TMarker.h>

/*
 *
 * manip_table.h
 *
 *   - manipulations and evaluations of strings - reactions
 *   - projectile/ejectile identification
 *   - interpret variables inside the table 
 */



/*
 *  specific language
 * #define var T=11.8     replace all T with 11.8
 * #free var dmylar       can fit all dmylar
 */




/****************************************************************/
/* #   sample.txt ---- file with definitions		        */
/* #   definitions of variables and assignement  of variables   */
/* #define var T=11.8					        */
/* #define var dmylar=12.0				        */
/* #define var angd1=0.0				        */
/* #define var de1a=0.2					        */
/* #define var de1b=382.0				        */
/* #  calibration for  dE1				        */
/* #define var pde1_1=1.0				        */
/* #define var pde1_0=0.0				        */
/* ############################				        */
/* #							        */
/* # all variables are fixed by default			        */
/* #							        */
/* #free var dmylar					        */
/* #							        */
/* #							        */
/* #							        */
/* #  all the defined variables will be replaced by the numbers */
/****************************************************************/



// this is maximum lines in the table = maximum number of points - #definitions...
const int MAXLINES=1000;      
const int MAXLINELENGTH=200;    // 113 is usual for 14 items
//
/*******************************************************
 *      line[]       keeps   all the file in memory
 */
TString line[MAXLINES];

TString alllosstables;
// main structure--------------------------------------
typedef struct{
  int i;
  int ap,zp,  at,zt,  ae,ze,  ar,zr;
  char losstab_proj[25]; 
  char losstab_ejec[25]; 
  char losstab_ejec_au[25];   //detector layers
  char losstab_ejec_si[25]; 
  char losstab_ejec_al[25]; 
  double k,dk;
  int kine;   // what kinematics  1 or  2?
  double T,Exc,ang;
  double tck; // thickness of the foil (mylar)
  double ly1,ly2;//detector layers
  double p3,p2,p1,p0;
  double E,dE;
  double sidepth;// implantation depth in Si
} TSnrfit;

TSnrfit sr[MAXLINES];



#include "pb_basic.c"



//  free_vars_step contains -1 or step
double free_vars_step[MAXVARIABLES];
int varnamefreelast=0;

int minuit_freevars, minuit_ndata;

  //  TGraphErrors -  data to display :
  const int MAXPOINTS=1000;
  int tg_nmax;
  double
         tg_p1[MAXPOINTS], 
         tg_p0[MAXPOINTS], 
         tg_x[MAXPOINTS], 
         tg_xdep[MAXPOINTS], 
         tg_y[MAXPOINTS], 
         tg_dx[MAXPOINTS], 
         tg_dy[MAXPOINTS];


const double angle_error=0.1;   // error of setting the angle ==0.1 degree












/******************************************************************
 *
 *         readout_reac_file   
 *
 *    can be extremely slow, this runs only once.....
 *    prepares all the variables and it's values
 *
 *    keeps the flie in memory
 */
int readout_reac_file(const char* filename){
  FILE * pFile;
  int i, j ,  idest;
  TString token, token2;
  char mystring[MAXLINELENGTH]; // temporary during readout

  pFile = fopen ( filename ,"r");printf("opening...%s\n","");
  if (pFile==NULL) { printf("cannot open %s,STOP\n", filename ); return 1;} 



  /*
   *    readout into memory.   clear field, spaces, add  '='
   */
  //****************   readout into memory, some some simplifications
  i=0;
  while ((i<MAXLINES)&&( feof(pFile)==0) ){
    if ( fgets (mystring , MAXLINELENGTH , pFile) ==NULL){
      // mystring remains unchanged at the end, NULL is returned
      line[i]="\0";
    }else{
      line[i]=mystring;
    }
   line[i].ReplaceAll('\n','\0');
   int lastlen=line[i].Length();
   //  one space in place of sever spaces - easier to parse later
   //  also  =  will serve as token ==> removing spaces around
   //  e.g.  if length < 4 ==> cannot be data "9 1"
   //   printf("line# %3d\n", i );
   line[i].ToLower(); // necessary to BASIC & FOR lookup_table2
   do {
     lastlen=line[i].Length();
     line[i].ReplaceAll("  "," ");
     //no spaces at the end of line (will append =)
     if ( line[i].Last(' ')==(line[i].Length()-1) ){
       line[i]=line[i](0,line[i].Length()-1) ;
     }
     line[i].ReplaceAll(" #","#");  // comment can start at any column
     line[i].ReplaceAll(" =","=");
     line[i].ReplaceAll("= ","=");
     if (line[i].Length()<4){ line[i]="#"; }
     //          printf(" replacing  spaces  %d ->  %d\n",  lastlen, line[i].Length() ) ;
     //     lastlen=line[i].Length();
   }while(  lastlen!=line[i].Length()  );
   if (line[i].Length()>3){//same i if not

   if (line[i].Index("#")==0){
     // if not predefined text => do not store the line
     if ( (line[i].Index("#define var ")!=0) &&
	  (line[i].Index("#free var ")!=0) ){
       i=i-1;
     }else{// when # ==>  mark the end with "=" to enable parsing later
       line[i].Append("=");// may be not needed?
     }
   }//starts with # 
   //   printf ("%s\n", line[i].Data()  );
   i=i+1;
   }//length>3 else just continue with the same i
   else{ line[i]="";}// last line 
  }//while i<MAXLINES
  printf("closing...%s\n","");
  fclose (pFile);
  //------------------readount   done -------------







  // ****     create  table of variables  ********
  /*    free vars  must be first  in the table.
   *      clear namespace and start with  freevars
   */
  init_namespace();
  i=0;
  printf("preparing free vars%s\n","");
  while( line[i].Length()>0 ){ 


      if  (line[i].Index("#free var ")==0 ){//free var
       TObjArray *tar= line[i].Tokenize("=");
       printf("%d\n", tar->GetEntries()  );
          token= ((TObjString*)(tar->At(0)))->GetString();
	  //     at 1  is  'step'
          token2= ((TObjString*)(tar->At(2)))->GetString();
      	  token.Remove(0,10);// remove the #text
	  if (token.Length()>11){
	    printf("maximum variables' name length is 12 /%s/\n",
		   token.Data()	); return 1;}   


     	  j=lookup_table_tk(  token.Data()  );//variable's position
       tar->Delete();
       free_vars_step[j]=token2.Atof();// fill the value
      printf("\t... free var /%s/ @%02d has step %11.5lf\n", 
	       token.Data() , j, free_vars_step[j] );
      }//free var
      i++;
  }//while




  /*
   *  now we have namespace with all the free variables allocated.
   */
  varnamefreelast=varnamelast;
  for (int i=0;i<varnamelast;i++){printf("%s=%lf\n", varname[i],variables[i]);}

  printf(" free vars= %d\n", varnamefreelast );

  minuit_freevars=varnamefreelast;

  /*
   *         reorder lines, clear comments, other values into name table
   */
  i=0; idest=0;// i is source line, j  is destination line
  while( line[i].Length()>0 ){ 
    printf ("%2d %2d %s", i,idest,line[i].Data()  ); 

    // REMOVE #comment lines
    if  (line[i].Index("#")!=0 ){
      line[idest]=line[i].Data();idest++;
    }

    //insert the variable into the table: FIXED size 12 of  "#define var "
    if  (line[i].Index("#define var ")==0 ){
       TObjArray *tar= line[i].Tokenize("=");
       printf("%d\n", tar->GetEntries()  );
       token= ((TObjString*)(tar->At(0)))->GetString();//var
       token2= ((TObjString*)(tar->At(1)))->GetString();//value
      	  token.Remove(0,12);// remove the #text, varname remains
       	  j=lookup_table_tk(  token.Data()  );//variable's position
       tar->Delete();
       variables[j]=token2.Atof();// fill the value
       //      printf("\t... var /%s/ @%02d contains %11.5lf\n", 
       //      token.Data() , j, variables[j] );
    }else{//define var 
       printf("%s\n","");
    }//define var
    i=i+1;//  next 
  }//  while  line[i]  contains  something-------------done ---
  minuit_ndata=(idest-1);

  printf(" defined vars= %d\n", varnamelast );

  /*
   *  no we have namespace with all the free variables allocated.
   */
  //  for (int i=0;i<varnamelast;i++){printf("%s=%lf\n", varname[i],variables[i]);}
  printf("-------------- readout reaction file done ----------%s\n", "");
  line[idest]=""; // last line ""
  return 0;
}//readout_reac_file


















/***********************************************************************
 *
 *   analyze the reaction description and material
 *
 *     in principle - we need 2 tables to load
 */
int parse_reaction_and_material(int iline, const char* react, const char* mat){

  TString reactS,proj,tgt1,ejec,remn,kine12;
  TString losstable;
  int kine;

  //  printf("parsing material %s and %s\n",  react, mat  );

  reactS=react;
  TObjArray *tar=reactS.Tokenize("_");
  //     
  proj= ((TObjString*)(tar->At(0)))->GetString();
  tgt1= ((TObjString*)(tar->At(1)))->GetString();
  ejec= ((TObjString*)(tar->At(2)))->GetString();
  remn= ((TObjString*)(tar->At(3)))->GetString();
  kine=1;

 if (tar->GetEntries() >4 ){
  kine12= ((TObjString*)(tar->At(4)))->GetString();
  if (kine12.CompareTo("k2")==0){ kine=2; }
 }//if kine12 defined
 // printf("%4s %4s %4s %4s %4s  \n",proj.Data(),tgt1.Data(),  ejec.Data(),  remn.Data(),  kine12.Data()   );

 //alllosstables="";

 //----------------------------------- structure
 int * azp, *azt, *aze, *azr;
 azp=(int*)getAZ( proj.Data() );
 azt=(int*)getAZ( tgt1.Data() );
 aze=(int*) getAZ( ejec.Data() );
 azr=(int*) getAZ( remn.Data() );
 printf("%3d %3d/   %3d %3d/   %3d %3d/   %3d %3d\n",	azp[0],azp[1], 	azt[0],azt[1],	aze[0],aze[1],	azr[0],azr[1] );
 sr[iline].i=iline;
 sr[iline].ap=azp[0];   sr[iline].zp=azp[1];
 sr[iline].at=azt[0];   sr[iline].zt=azt[1];
 sr[iline].ae=aze[0];   sr[iline].ze=aze[1];
 sr[iline].ar=azr[0];   sr[iline].zr=azr[1];
 
 sr[iline].kine= kine;// kinematics 1 or 2


 //---------------------------------- loss

 losstable=proj.Data();

 losstable.Append("_in_");
 losstable.Append( mat );
 losstable.Append(".los");
 sprintf( sr[iline].losstab_proj, "%s", losstable.Data()  );
 losstable.Append(" ");
 // printf("%s ---> %s\n", losstable.Data(),  alllosstables.Data()  );
 if ( alllosstables.Index(losstable.Data())<0){
   alllosstables.Append(  losstable.Data() );
 }




 losstable=ejec.Data();

 losstable.Append("_in_");
 losstable.Append( mat );
 losstable.Append(".los");
 sprintf( sr[iline].losstab_ejec, "%s", losstable.Data()  );
 losstable.Append(" ");
 // printf("%s -2-> %s\n", losstable.Data(),  alllosstables.Data()   );
 if ( alllosstables.Index(losstable.Data())<0){
   alllosstables.Append(  losstable.Data() );
 }

 /********
  *  specialy in   Au   and  Si    for  detectors  1/3
  */

 losstable=ejec.Data();

 losstable.Append("_in_");
 losstable.Append( "au" );
 losstable.Append(".los");
 sprintf( sr[iline].losstab_ejec_au, "%s", losstable.Data()  );
 losstable.Append(" ");
 // printf("%s -2-> %s\n", losstable.Data(),  alllosstables.Data()   );
 if ( alllosstables.Index(losstable.Data())<0){
   alllosstables.Append(  losstable.Data() );
 }


 /*
  *  specialy in   Au   and  Si    for  detectors  2/3
  */


 losstable=ejec.Data();

 losstable.Append("_in_");
 losstable.Append( "si" );
 losstable.Append(".los");
 sprintf( sr[iline].losstab_ejec_si, "%s", losstable.Data()  );
 losstable.Append(" ");
 // printf("%s -2-> %s\n", losstable.Data(),  alllosstables.Data()   );
 if ( alllosstables.Index(losstable.Data())<0){
   alllosstables.Append(  losstable.Data() );
 }

 /*
  *  specialy in   Au   and  Si    for  detectors  3/3
  */


 losstable=ejec.Data();

 losstable.Append("_in_");
 losstable.Append( "al" );
 losstable.Append(".los");
 sprintf( sr[iline].losstab_ejec_si, "%s", losstable.Data()  );
 losstable.Append(" ");
 // printf("%s -2-> %s\n", losstable.Data(),  alllosstables.Data()   );
 if ( alllosstables.Index(losstable.Data())<0){
   alllosstables.Append(  losstable.Data() );
 }


 //printf("losstables==%s,%s\n",losstable.Data(),losstable2.Data());
 // if (load_losses_table( losstable.Data())!=0){return 1;}
 // if (load_losses_table( losstable2.Data())!=0){return 1;}
 // load_losses_table( losstable2.Data() );
 return 0;
}//parse_reaction_and_material








/***********************************************
 *   interpolate the fixed variables in the table, leave the free
 *    only lines with data
 *  fill in the structure[i]
 *  prepare and load losstables
 *
 *     in principle - it is reasobale to interpolate
 *          column #5-#14   i.e.  i=4...13
 *
 *
 */
int interpolate_varnames_in_table(){

  int  i=0, j,  look1, look2, acceptedlines=0;
  TString token, tok, tok2;
  //     
  alllosstables="";
  look1=varnamelast;
  while( line[i].Length()>0 ){
    //    if  (line[i].Index("#")!=0 ){// not comment
      TObjArray *tar=line[i].Tokenize(" ");
      if (tar->GetEntries() != 14 ){ printf("not 14 collumns.stop%s\n","");return 1;}
      if (tar->GetEntries() == 14 ){
	acceptedlines++;
      //-------  go through all the tokens and print -----
        for (int k=0;k<tar->GetEntries();k++){
	  if ( (k!=2)&&(k!=3) ){
	  tok= ((TObjString*)(tar->At(k)))->GetString();

	  if ( tok.IsFloat() ){
	    //	    printf(" %7.2f",  tok.Atof()  );  
	    if (k== 0){ sr[i].k  = tok.Atof()    ; }
	    if (k== 1){ sr[i].dk = tok.Atof()    ; }
	    if (k== 4){ sr[i].Exc= tok.Atof()    ; }
	    if (k== 5){ sr[i].T  = tok.Atof()    ; }
	    if (k== 6){ sr[i].tck= tok.Atof()    ; }
	    if (k== 7){ sr[i].ang= tok.Atof()    ; }
	    if (k== 8){ sr[i].ly1= tok.Atof()    ; }
	    if (k== 9){ sr[i].ly2= tok.Atof()    ; }
	    if (k==10){ sr[i].p3 = tok.Atof()    ; }
	    if (k==11){ sr[i].p2 = tok.Atof()    ; }
	    if (k==12){ sr[i].p1 = tok.Atof()    ; }
	    if (k==13){ sr[i].p0 = tok.Atof()    ; }
	  }else{// IS NOT  FLOAT.........=> variable

	    // replace fixed parameters here and treat the formulea
	    char expr[120];
	    //done	    tok.ToLower();  // NECESSARY FOR lookup_table2
	    sprintf( expr, "%s" , tok.Data()  ) ;  
	    if (   (strpbrk( expr ,  "+" )!=NULL)||   
		   (strpbrk( expr ,  "-" )!=NULL)||  
		   (strpbrk( expr ,  "*" )!=NULL)||    
		   (strpbrk( expr ,  "/" )!=NULL)    	 
		   ){
	      printf("FORM=%s\n" ,  tok.Data()  );
	    }else{//lookup_table2_isform==1

	    j=lookup_table_tk( tok.Data()  ); // key point - the NAME
	    if (free_vars_step[j]<=0.0){
	      //	    printf(" %7.2f", variables[j] );
	    char ch[40]; sprintf( ch, "%lf",  variables[j] );
	    line[i].Replace( line[i].Index(varname[j]), strlen(varname[j]),ch );
	    if (k== 4){ sr[i].Exc= variables[j]    ; }
	    if (k== 5){ sr[i].T  = variables[j]    ; }
	    if (k== 6){ sr[i].tck= variables[j]    ; }
	    if (k== 7){ sr[i].ang= variables[j]    ; }
	    if (k== 8){ sr[i].ly1= variables[j]    ; }
	    if (k== 9){ sr[i].ly2= variables[j]    ; }
	    if (k==10){ sr[i].p3 = variables[j]    ; }
	    if (k==11){ sr[i].p2 = variables[j]    ; }
	    if (k==12){ sr[i].p1 = variables[j]    ; }
	    if (k==13){ sr[i].p0 = variables[j]    ; }

	    }//else{//free/fixed
	    //printf(" %7s", varname[j] ); 
	    //}//free/fixed
	    //if (k==13) {printf("%s\n","");}//new line at te last
	    }//lookup_table2_isform==1
	  }//if else lfoat
	  }// not k 2  nor k 3 (reaction and medium)
	  }// for all tar
      }// MUST be 14  entries, else skip 
         tok = ((TObjString*)(tar->At(2)))->GetString();
         tok2= ((TObjString*)(tar->At(3)))->GetString();
	 int r=parse_reaction_and_material( i,  tok.Data(),  tok2.Data()  );
	 if (r!=0){ return 1;}
	delete tar; // manual says I must delete it
	//    }//if line is LIVE == not commented
    /***********************************************************************/
    /* else{//this stage removes all commented lines that are FIXED	   */
    /*   if (line[i].Index("#define var ")==0){				   */
    /* 	 TObjArray *tar= line[i].Tokenize("=");				   */
    /* 	 token= ((TObjString*)(tar->At(0)))->GetString();//var		   */
    /* 	 token.Remove(0,12);// remove the #text, varname remains	   */
    /* 	 tar->Delete();							   */
    /*   }//#defined var						   */
    /***********************************************************************/
    //}// it was commented..... remove FIXED
    i=i+1;//  next 
  }//  while  line[i]  contains  something-------------done ---
  //  printf( "%d lines from the original table  accepted\n", acceptedlines );



  /*
   *   check if something didnt go wrong.....
   */
  look2=varnamelast;
  if (look1 != look2){
    printf("%s\n","Problem, some table items were treated as variables but they were not present in lookup table....and they were added to it. stop please.\n\n ");
    return 1;
  }// varnamelast changed

  i=0; printf("------------------------%s\n","");
  while( line[i].Length()>0){printf("%2d %s\n",i,line[i].Data() );i++;}


  // alllosstables
  printf("ALLLOSS TABLES:\n%s\n", alllosstables.Data() );

  if (load_losses_table_all( alllosstables )!=0){return 1;}  
  printf("------------------------%s\n","");

  /*
   *   last printout of  variable table
   */
  for (int i=0;i<varnamelast;i++){ 
    if (i<varnamefreelast){
    printf("%14s=%19.7lf (%lf)\n",varname[i],variables[i],free_vars_step[i] ); 
    }else{
    printf("%14s=%19.7lf \n",varname[i],variables[i] ); 
    }
  }
  return 0;
}//interpolate_varnames_in_table









/***********************************************
 *  
 *    do_kinematics
 *
 *    fill the remaining variables
 *    (printout)
 *    compute loss in 50% of thickness of the tgt layer
 *
 *
 *
 */
double  do_kinematics(int sverbo){
  int verbo=sverbo;
  double DE=0.0;
  double CHI2=0.0;

  int  i=0, k=0;
  TString token, tok, tok2;
  double TKE, TKE_ae, TKE_in, TKE_out, TKE_IN_SI;  //TKE angle error

  double SLICE=0.1;  //  0.1um was in ver.1

  int verb=0; // 0..no,    3.. max 
  int  look1, look2;
 look1=varnamelast;
  while( line[i].Length()>0 ){
    TObjArray *tar=line[i].Tokenize(" ");
    // start with 4 (Exc.,  T, ...)
    for (k=4;k<tar->GetEntries();k++){
      tok= ((TObjString*)(tar->At(k)))->GetString();
       if ( tok.IsFloat()==1 ){
       }else{//all done for float-do for string

	 double variablesj;
	 char expr[120];
	 sprintf( expr, "%s" , tok.Data()  ) ; 
	 variablesj=lookup_table2( expr  );


	 /* if (lookup_table2_isform( expr )==1){ */
	 /*      printf("FORM=%s\n" ,  tok.Data()  ); */
	 /* }else{//lookup_table2_isform==1 */
	 /*   j=lookup_table( tok.Data()  ); */
	 /*   variablesj=variables[j]; */
	 /* } */
	 //	 variables[j];
	    if (k== 4){ sr[i].Exc= variablesj  ; }
	    if (k== 5){ sr[i].T  = variablesj  ; }
	    if (k== 6){ sr[i].tck= variablesj  ; }
	    if (k== 7){ sr[i].ang= variablesj  ; }
	    if (k== 8){ sr[i].ly1= variablesj  ; }
	    if (k== 9){ sr[i].ly2= variablesj  ; }
	    if (k==10){ sr[i].p3 = variablesj  ; }
	    if (k==11){ sr[i].p2 = variablesj  ; }
	    if (k==12){ sr[i].p1 = variablesj  ; }
	    if (k==13){ sr[i].p0 = variablesj  ; }	 
       }//tok is not float
    }//for all k 
    tar->Delete();

    if (verbo!=0){
    //-----------------------------PRINTOUT BEGIN
    if (i==0)
    printf("azp   azt   aze   azr,      (kinem)    k          dk,            Exc      T      thickness      ang,  /layer1      layer2/    <p3 p2 p1 p0>%s\n",""); 
    printf("%3d %3d,%3d %3d,%3d %3d,%3d %3d,(%1d) %9.3lf %9.3lf, %9.3lf %9.3lf %9.3lf, %9.3lf /%9.3lf %9.3lf/ <  %9.3lf %9.3lf  %9.5lf  %9.5lf > \n", 
sr[i].ap, sr[i].zp, sr[i].at, sr[i].zt,
sr[i].ae, sr[i].ze, sr[i].ar, sr[i].zr,
sr[i].kine,
	   sr[i].k,sr[i].dk,   

	   sr[i].Exc, sr[i].T, sr[i].tck, sr[i].ang,
	   sr[i].ly1,  sr[i].ly2,
	   sr[i].p3, sr[i].p2, sr[i].p1, sr[i].p0 //,
	   //	   sr[i].losstab_proj,
	   //sr[i].losstab_ejec
	   );
    }// verbo ==0
    //-----------------------------PRINTOUT END


    //------------------loose in 50%-----------BEGIN
    // TKE=loose_E( sr[i].T , sr[i].losstab_proj , 0.5*sr[i].tck  , SLICE, verb );
 TKE=loose_E( sr[i].T , sr[i].losstab_proj , 0.5*sr[i].tck  , SLICE, 0 );
 // if (verbo!=0){ printf("TKE=%11.5f(mid)  ",  TKE  ); }



 //
 // ----------------   KINEMATICS --------------
 //
 TKE_ae=angular_E3( sr[i].ap,  sr[i].zp,  sr[i].at,  sr[i].zt,
		 sr[i].ae,  sr[i].ze,  sr[i].ar,  sr[i].zr,
		 TKE,   sr[i].ang-angle_error,   sr[i].kine,  sr[i].Exc,
		 verb );
 TKE   =angular_E3( sr[i].ap,  sr[i].zp,  sr[i].at,  sr[i].zt,
		 sr[i].ae,  sr[i].ze,  sr[i].ar,  sr[i].zr,
		 TKE,   sr[i].ang,   sr[i].kine,  sr[i].Exc,
		 verb );
 TKE_ae= (TKE-TKE_ae)* (TKE-TKE_ae)  ; //contains an error fronm angle ^2
 // if (verbo!=0){ printf("%11.5f(ejec)",  TKE  );}

 //
 //Ap,Zp, At,Zt, Ae,Ze, Ar,Zr,  TKE , ang , sol , Exc  ,verb );// sol 1/2, excE
 //   TKE=t3a;

    //------------------loose in 50%-----------END
 double extendl= sr[i].ang/180*3.1415926;
 extendl=1.0/cos(extendl) * sr[i].tck;
 TKE=loose_E( TKE ,     sr[i].losstab_ejec , 0.5*extendl  , SLICE, 0 );
 if (TKE<0){ verbo=3; return -1.0; }
 // if (verbo!=0){ printf("%11.5f(full)",  TKE  );}
 //              ....DETECTOR......
 
 //
// GOLDEN LAYER
//
 TKE=loose_E( TKE ,     sr[i].losstab_ejec_au ,  sr[i].ly1  , SLICE, 0 );
 if (TKE<0){ verbo=3; return -1.0; }
 // if (verbo!=0){ printf("%11.5f(au)",  TKE  );}
 // Si LAYER
 TKE_in=TKE;


 /*
  * SI   LAYER  
  */
 TKE_out=loose_E( TKE , sr[i].losstab_ejec_si ,  sr[i].ly2  , SLICE, 0 );
 TKE_IN_SI  =   TKE_in  -   TKE_out; // Energy deposited in the detector

 if (TKE_out==0.0){// find implantation depth
   TKE_out=loose_E( TKE ,sr[i].losstab_ejec_si , sr[i].ly2 , -SLICE, 0 ); 
   sr[i].sidepth=TKE_out;
 }// stopped
 else{ sr[i].sidepth=sr[i].ly2;  }
 // if (verbo!=0){ printf("%11.5f(si)  (in Si=%11.5f)\n",  TKE_out,  TKE_IN_SI  );}
 // stop here.....



 /*
  *   e ,   de from  linear dependence
  */
 sr[i].E =  TKE_IN_SI;
 //to je kravina 
 // sr[i].dE=  TKE_IN_SI *  sr[i].p1*  sr[i].dk  ;// preliminary linear dependence
 /*
  * preliminary linear dependence
  *    combined with  angular error influence
  */
 sr[i].dE= sqrt(   (TKE_ae)+  pow( sr[i].p1 *  sr[i].dk,2.0)  )  ;
 // sr[i].dE= TKE *  sr[i].dk/sr[i].k  ; // not this...


 /*
  *     need to weight....
  */
DE= sr[i].E - 
  ( sr[i].p3*sr[i].k*sr[i].k*sr[i].k 
   +sr[i].p2*sr[i].k*sr[i].k 
   +sr[i].p1*sr[i].k 
   +sr[i].p0 );

 CHI2=CHI2+ DE*DE/sr[i].dE/sr[i].dE;
 //TGraphErrors  -  fill in the global fields:
 if (i<MAXPOINTS){
   tg_x[i] =sr[i].k;
   tg_xdep[i] =sr[i].sidepth;
   tg_y[i] =sr[i].E;
   tg_dx[i]=sr[i].dk;
   tg_dy[i]=sr[i].dE;
   tg_p1[i]=sr[i].p1;
   tg_p0[i]=sr[i].p0;
   tg_nmax=i+1;
  }

    i++;
  }//while line exists-----------------------------------------WHILE

  if (verbo!=0){ 
    printf("-----------------------------------------------------%lf---------------do_kinematics end\n",CHI2);
  }  
  
  look2=varnamelast;
    if (look1 != look2){
    printf("%s\n","Problem, some table items were treated as variables but they were not present in lookup table....and they were added to it. stop please.\n\n ");
    return 0.0;
  }// varnamelast changed

  return CHI2;
}//interpolate_varnames_in_table









/**********************************************************************    
 *MUNIUT
 **********************************************************************
 *
 *
 */ 
void calc_chi_square(Int_t &npar, Double_t *gin, Double_t &f, Double_t *par, Int_t iflag){
  /*
   *  f carries out the chi2
   *  npar is number of fit parameters
   *  par[] contains the parameters
   */
  int i;
  // printf("%2d  %13.6f\n", npar, par[0] );
  for (i=0;i<npar;i++){
    variables[i]=par[i]; // change the value of the parameter in the table
  }
  f=do_kinematics(0);
  return;
}// minuit function










/***********************************************************
 *  blindly taken from 
 *    http://tesla.desy.de/~pcastro/example_progs/fit/minuit_c_style/fit_minuit.cxx 
 */ 
int lincal(){
  //  int MAXPAR=20; // for the future
  static Double_t vstart[20];
  static Double_t step[20];


// if you want to access to these parameters, use:
  Double_t amin,edm,errdef;
  Int_t nvpar,nparx,icstat;
  //  resulting   parameter and its' error
  double p,dp ;

printf( "Minuit:  %d parameters\n", varnamefreelast  );
 // PREPARE TMinuit fit
 TMinuit *gM=new TMinuit( varnamefreelast ); // maximum of n parameters
   //
  //  select verbose level:
  //    default :     (58 lines in this test)
  //    -1 : minimum  ( 4 lines in this test)
  //     0 : low      (31 lines)
  //     1 : medium   (61 lines)
  //     2 : high     (89 lines)
  //     3 : maximum (199 lines in this test)
  //
  gM->SetPrintLevel();
  // set the user function that calculates chi_square (the value to minimize)
  gM->SetFCN(calc_chi_square);

  //  this I just copy......
  Double_t arglist[10];
  Int_t ierflg = 0;
  arglist[0] = 1;
  gM->mnexcm("SET ERR", arglist ,1,ierflg);


  // Set starting values and step sizes for parameters (modif.)
  char pnam[10];// parameter name
  for (int i=0;i<varnamefreelast;i++){
    vstart[i]=variables[i];
    step[i]=free_vars_step[i];
    sprintf( pnam, "p%d",  varnamefreelast-i-1 );
    gM->mnparm(i, pnam , vstart[i], step[i], 0, 0, ierflg);
  }//for all free params



 // Now ready for minimization step (just copied...)
  arglist[0] = 500;
  arglist[1] = 1.;
  gM->mnexcm("MIGRAD", arglist ,2,ierflg);// MINUIT RUN - evidently

// Print results
   printf("smnsat----------------------------%d\n",1);
   gM->mnstat(amin,edm,errdef,nvpar,nparx,icstat);
printf("best=%f   estimvert=%f  UP(uncert)=%f   nvari=%d  npar=%d  COVMTX(0-3)=%d \n",amin,edm,errdef, nvpar,nparx,icstat);

    printf("mnPrint results----------------------------%d\n",1);
  for (int i=0;i<varnamefreelast;i++){
    //    vstart[i]=variables[i];
    //    step[i]=free_vars_step[i];
    sprintf( pnam, "p%d", varnamefreelast-i-1 );
    gM->GetParameter( i, p, dp );printf("   %s : %f  %f \n",pnam, p,dp);
    variables[i]=p;
    //    gM->mnparm(i, pnam , vstart[i], step[i], 0, 0, ierflg);
  }//for all free params
    printf("mnPrint results----------------------------%d\n",1);
    printf("Chi2 = %f   data=%d  params=%d  ChiNorm = %f  \n",  amin, minuit_ndata, minuit_freevars, sqrt( amin / (minuit_ndata-minuit_freevars)  )  );
    printf("mnPrint results----------------------------%d\n",1);


    /*
     *    GRAPH:   we must refresh variables[j] to remove the effect of MIGRAD
     *          we did in the previous loop => new do_kinematics....
     */
    do_kinematics(1);
    /*
     *    najdi pocet moznych  grafu zde. 
     */
    double allp1[50]; //max 50 grafu
    int colorgr[tg_nmax];
    int maxi=0;
    for (int i=0;i<50;i++){      allp1[i]=0.0;    }
    for (int n=0 ; n<tg_nmax ; n++){colorgr[n]=0;};


for (int n=0 ; n<tg_nmax ; n++){
    for (int i=0;i<50;i++){      
      if ( (allp1[i]==0.0) ){ colorgr[n]=i;allp1[i]= tg_p1[n];maxi=i; break;  }
      if ( (allp1[i]== tg_p1[n]) ){ colorgr[n]=i; i=50;break;   }
    }//i graphs
 }//n - lines
 printf("%d. different graphs\n", maxi+1 );

    //  g  is  a  normal  graph....
    double rozpty=0.002;   // 2 keV
    double rozptytmp=0.0;
    double max_x_chan=3500.;
/* TGraphErrors *g=new TGraphErrors(tg_nmax, tg_x, tg_y, tg_dx, tg_dy); */
/*    g->Draw("pawl");  */
/*    TLine *l=new TLine( 0.0 , p0, 5000. , p1*5000.+p0 ); */
/*    l->SetLineColor(2); */
/*    l->Draw("same"); */
/*  return 0; */

for (int n=0 ; n<tg_nmax ; n++){
    rozptytmp=tg_y[n]- tg_p1[n]*tg_x[n] -tg_p0[n];
    if (rozptytmp<0.0){ rozptytmp=-rozptytmp;}
    if (rozptytmp>rozpty) { rozpty=rozptytmp;}
    if ( tg_x[n]>max_x_chan){ max_x_chan= tg_x[n]*1.05;  }
  /*   printf("%f %f D=%f, x=%f  p1=%f, p0=%f ... %f\n",  */
/* 	   tg_y[n], tg_p1[n]*tg_x[n] +tg_p0[n],  */
/* 	   tg_y[n] - tg_p1[n]*tg_x[n] +tg_p0[n],  */
/* 	   tg_x[n],tg_p1[n], tg_p0[n], */
/* 	   rozpty */
/* ); */
    tg_y[n]=tg_y[n]- tg_p1[n]*tg_x[n] -tg_p0[n];
  }//
/*
*    prepare  canvas:
*/
  TH2F *hh=new TH2F("hh","hh",1500,0,max_x_chan,200,-rozpty*1.1, rozpty*1.1);
  hh->SetStats(kFALSE);
  hh->SetTitle(";channel or impl.depth;E_{theo}-E_{calib}");
  hh->Draw();
  /*
   *
   *
   *
   *
   */

  int graphtype;  // 1== channels,  2==depth
  graphtype=1;


  // TRY TO MAKE g1  as  channel x energy
   TGraphErrors *g1=new TGraphErrors(tg_nmax, tg_x, tg_y, tg_dx, tg_dy);
   g1->SetMarkerStyle(1);
   
  //g2   is   'remnants'  graph
  //  TGraphErrors *g2=new TGraphErrors(tg_nmax, tg_x, tg_y, tg_dx, tg_dy);
  TGraphErrors *g2=new TGraphErrors(tg_nmax, tg_xdep, tg_y, tg_dx, tg_dy);
  g2->SetMarkerStyle(1);
  //  g2->SetMarkerColor(1);


  // EITHER  OR
  if (graphtype==1){  g1->Draw("p"); }
  if (graphtype==2){  g2->Draw("p"); }
  // common line
  TLine *l2=new TLine( 0.0 , 0.0, max_x_chan , 0.0 );
  l2->SetLineColor(2);  l2->Draw("same");
  
  // NOW COLOR MARKERS
  for (int n=0 ; n<tg_nmax ; n++){
    TMarker *m;
    //  channel
    if (graphtype==1){ 
         m=new TMarker( tg_x[n], tg_y[n], 22 );
    }
    //  implantation depth
    if (graphtype==2){ 
         m=new TMarker( tg_xdep[n], tg_y[n], 22 );
    }

    m->SetMarkerColor( colorgr[n]+1 );
    m->Draw();

  }// marker 

  // }// VERBOSITY
  //  return;

 return  0;


}// lincal---  fit kalibrace ---


/*
 *
 *  losslib2.h
 *
 *   bunch of functions to 
 *   - compute reaction kinematics
 *   - evaluate losses in materials
 *        - load spline datafiles 
 *
 *
 *  o/  reaction procedure - the basics:
 *       loose_E ,  angular_E3 , loose_E
 *
 *
 */

#include <TF1.h>
#include <TROOT.h>

#include <TGraph.h>
#include <TGraphErrors.h>
#include <TSpline.h>
//#include <TSpline3.h>
//pow
#include <math.h>
// gr_engine ... ifstream
//using namespace std;
// tchainb has everything with objstring  #include <TObjString.h>
#include <TChain.h>
// to have environment
#include "TEnv.h"
// to ask for directory 
#include <sys/types.h>
#include <sys/stat.h>

/************************************
 *  check what is number and strip it
 *      U238
 */
int* getAZ( const char* elem ){
  TString el=elem;
  TString as;
  int a=0,z=0,i;
  i=3;// 999 nucleons maximum
  as=el( el.Length()-i, el.Length() );
  if (!as.IsDigit() ){ i--;}  as=el( el.Length()-i, el.Length() );
  if (!as.IsDigit() ){ i--;}  as=el( el.Length()-i, el.Length() );
  a=as.Atoi();// A
  el=el(0, el.Length()-i);

  i=1;
  if (el.CompareTo("h")==0){ z=i;} i++;
  if (el.CompareTo("he")==0){ z=i;} i++;
  if (el.CompareTo("li")==0){ z=i;} i++;
  if (el.CompareTo("be")==0){ z=i;} i++;
  if (el.CompareTo("b")==0){ z=i;} i++;
  if (el.CompareTo("c")==0){ z=i;} i++;
  if (el.CompareTo("n")==0){ z=i;} i++;
  if (el.CompareTo("o")==0){ z=i;} i++;
  if (el.CompareTo("f")==0){ z=i;} i++;
  if (el.CompareTo("ne")==0){ z=i;} i++;
  //11-20
  if (el.CompareTo("na")==0){ z=i;} i++;
  if (el.CompareTo("mg")==0){ z=i;} i++;
  if (el.CompareTo("al")==0){ z=i;} i++;
  if (el.CompareTo("si")==0){ z=i;} i++;
  if (el.CompareTo("p")==0){ z=i;} i++;
  if (el.CompareTo("s")==0){ z=i;} i++;
  if (el.CompareTo("cl")==0){ z=i;} i++;
  if (el.CompareTo("ar")==0){ z=i;} i++;
  if (el.CompareTo("k")==0){ z=i;} i++;
  if (el.CompareTo("ca")==0){ z=i;} i++;
  //21-30
  if (el.CompareTo("sc")==0){ z=i;} i++;
  if (el.CompareTo("ti")==0){ z=i;} i++;
  if (el.CompareTo("v")==0){ z=i;} i++;
  if (el.CompareTo("cr")==0){ z=i;} i++;
  if (el.CompareTo("mn")==0){ z=i;} i++;
  if (el.CompareTo("fe")==0){ z=i;} i++;
  if (el.CompareTo("co")==0){ z=i;} i++;
  if (el.CompareTo("ni")==0){ z=i;} i++;
  if (el.CompareTo("cu")==0){ z=i;} i++;
  if (el.CompareTo("zn")==0){ z=i;} i++;
  //31-
  if (el.CompareTo("ga")==0){ z=i;} i++;
  if (el.CompareTo("ge")==0){ z=i;} i++;
  if (el.CompareTo("as")==0){ z=i;} i++;
  if (el.CompareTo("se")==0){ z=i;} i++;
  if (el.CompareTo("br")==0){ z=i;} i++;
  if (el.CompareTo("kr")==0){ z=i;} i++;

  //  printf("/%d/%d/\n", a, z );//Z
  int * az=new int[2];   //  allocate like in pruchod_rea
  az[0]=a;   az[1]=z;
  return az;   // send it like (int)
}

  //#################################################
  //#      MEX
  //#################################################

double f_mex(int a, int z){
 if (a==1 && z==0){ return 8.071323;} 	 if (a==1 && z==1){ return 7.2889705;} 	
 if (a==2 && z==1){ return 13.13572158;}  if (a==3 && z==1){ return 14.949806;} 
 if (a==4 && z==1){ return 25.901518;} 	 if (a==5 && z==1){ return 32.89244;} 	
 if (a==3 && z==2){ return 14.93121475;}   if (a==4 && z==2){ return 2.42491565;}
 if (a==5 && z==2){ return 11.386233;} 	   if (a==6 && z==2){ return 17.595106;} 	
 if (a==7 && z==2){ return 26.101038;} 	 if (a==8 && z==2){ return 31.598044;} 	
 if (a==9 && z==2){ return 40.939429;} 	 if (a==10 && z==2){ return 48.809203;}   
 if (a==4 && z==3){ return 25.323185;} 	 if (a==5 && z==3){ return 11.678886;} 	
 if (a==6 && z==3){ return 14.086793;} 	 if (a==7 && z==3){ return 14.908141;} 	
 if (a==8 && z==3){ return 20.946844;} 	 if (a==9 && z==3){ return 24.954264;} 	
 if (a==10 && z==3){ return 33.050581;}  if (a==11 && z==3){ return 40.79731;} 	
 if (a==6 && z==4){ return 18.374947;} 	 if (a==7 && z==4){ return 15.770034;} 	
 if (a==8 && z==4){ return 4.941672;} 	 if (a==9 && z==4){ return 11.347648;} 	
 if (a==10 && z==4){ return 12.60667;} 	 if (a==11 && z==4){ return 20.174064;}   
 if (a==12 && z==4){ return 25.076506;}    if (a==13 && z==4){ return 33.247823;}   
 if (a==14 && z==4){ return 39.954498;}    if (a==7 && z==5){ return 27.868346;} 	
 if (a==8 && z==5){ return 22.92149;} 	 if (a==9 && z==5){ return 12.415681;} 	
 if (a==10 && z==5){ return 12.050731;}    if (a==11 && z==5){ return 8.667931;} 	
 if (a==12 && z==5){ return 13.368899;}    if (a==13 && z==5){ return 16.562166;}   
 if (a==14 && z==5){ return 23.663683;}    if (a==15 && z==5){ return 28.972278;}   
 if (a==16 && z==5){ return 37.081686;}    if (a==17 && z==5){ return 43.770816;}   
 if (a==8 && z==6){ return 35.09406;} 	 if (a==9 && z==6){ return 28.910491;} 	
 if (a==10 && z==6){ return 15.698682;}    if (a==11 && z==6){ return 10.650342;}   
 if (a==12 && z==6){ return 0.000000;}    if (a==13 && z==6){ return 3.12501129;}  
 if (a==14 && z==6){ return 3.01989305;}   if (a==15 && z==6){ return 9.873144;} 	
 if (a==16 && z==6){ return 13.694129;}    if (a==17 && z==6){ return 21.038832;}   
 if (a==18 && z==6){ return 24.926178;}    if (a==19 && z==6){ return 32.420666;}   
 if (a==20 && z==6){ return 37.55761;} 	 if (a==10 && z==7){ return 38.800148;}   
 if (a==11 && z==7){ return 24.303569;}    if (a==12 && z==7){ return 17.338082;}   
 if (a==13 && z==7){ return 5.345481;} 	 if (a==14 && z==7){ return 2.86341704;}  
 if (a==15 && z==7){ return 0.10143805;}   if (a==16 && z==7){ return 5.683658;} 	
 if (a==17 && z==7){ return 7.871368;} 	 if (a==18 && z==7){ return 13.114466;}   
 if (a==19 && z==7){ return 15.862129;}    if (a==20 && z==7){ return 21.76511;} 	
 if (a==21 && z==7){ return 25.251164;}    if (a==22 && z==7){ return 32.038675;}   
 if (a==23 && z==7){ return 36.682;} 	 if (a==12 && z==8){ return 32.047954;}   
 if (a==13 && z==8){ return 23.112428;}    if (a==14 && z==8){ return 8.007356;} 	
 if (a==15 && z==8){ return 2.855605;} 	 if (a==16 && z==8){ return -4.73700141;} 
 if (a==17 && z==8){ return -0.808813;}    if (a==18 && z==8){ return -0.781522;}   
 if (a==19 && z==8){ return 3.33487;} 	 if (a==20 && z==8){ return 3.797462;} 	
 if (a==21 && z==8){ return 8.062906;} 	 if (a==22 && z==8){ return 9.284152;} 	
 if (a==23 && z==8){ return 14.617;} 	 if (a==24 && z==8){ return 18.594;} 	
 if (a==15 && z==9){ return 16.775372;}    if (a==16 && z==9){ return 10.680254;}   
 if (a==17 && z==9){ return 1.951701;} 	 if (a==18 && z==9){ return 0.873701;} 	
 if (a==19 && z==9){ return -1.487386;}    if (a==20 && z==9){ return -0.017404;}   
 if (a==21 && z==9){ return -0.047551;}    if (a==22 && z==9){ return 2.793378;} 	
 if (a==23 && z==9){ return 3.329747;} 	 if (a==24 && z==9){ return 7.559527;} 	
 if (a==25 && z==9){ return 11.348;} 	 if (a==26 && z==9){ return 18.608;} 	
 if (a==27 && z==9){ return 24.69;} 	 if (a==16 && z==10){ return 23.996462;}  
 if (a==17 && z==10){ return 16.460901;}   if (a==18 && z==10){ return 5.317166;}   
 if (a==19 && z==10){ return 1.75144;} 	 if (a==20 && z==10){ return -7.04193131;}
 if (a==21 && z==10){ return -5.731776;}   if (a==22 && z==10){ return -8.024715;}  
 if (a==23 && z==10){ return -5.154045;}   if (a==24 && z==10){ return -5.951521;}  
 if (a==25 && z==10){ return -2.108075;}   if (a==26 && z==10){ return 0.429611;}   
 if (a==27 && z==10){ return 7.033;} 	 if (a==28 && z==10){ return 11.267;} 	
 if (a==29 && z==10){ return 18.357;} 	 if (a==30 && z==10){ return 23.052;} 	
 if (a==31 && z==10){ return 30.824;} 	 if (a==18 && z==11){ return 24.189968;}  
 if (a==19 && z==11){ return 12.926808;}   if (a==20 && z==11){ return 6.847719;}   
 if (a==21 && z==11){ return -2.184161;}   if (a==22 && z==11){ return -5.182436;}  
 if (a==23 && z==11){ return -9.52985358;} if (a==24 && z==11){ return -8.418114;}  
 if (a==25 && z==11){ return -9.357818;}   if (a==26 && z==11){ return -6.862316;}  
 if (a==27 && z==11){ return -5.517436;}   if (a==28 && z==11){ return -0.989247;}  
 if (a==29 && z==11){ return 2.665004;}    if (a==30 && z==11){ return 8.36109;} 	
 if (a==31 && z==11){ return 12.552;} 	 if (a==32 && z==11){ return 18.834;} 	
 if (a==33 && z==11){ return 23.632;} 	 if (a==19 && z==12){ return 33.040092;}  
 if (a==20 && z==12){ return 17.570348;}   if (a==21 && z==12){ return 10.910506;}  
 if (a==22 && z==12){ return -0.396963;}   if (a==23 && z==12){ return -5.473766;}  
 if (a==24 && z==12){ return -13.933567;}  if (a==25 && z==12){ return -13.192826;} 
 if (a==26 && z==12){ return -16.214582;}  if (a==27 && z==12){ return -14.586651;} 
 if (a==28 && z==12){ return -15.018641;}  if (a==29 && z==12){ return -10.619032;} 
 if (a==30 && z==12){ return -8.910672;}   if (a==31 && z==12){ return -3.21738;}   
 if (a==32 && z==12){ return -0.954781;}   if (a==33 && z==12){ return 4.89407;} 	
 if (a==34 && z==12){ return 8.587;} 	 if (a==35 && z==12){ return 15.638;} 	
 if (a==36 && z==12){ return 20.38;} 	 if (a==23 && z==13){ return 6.76957;} 	
 if (a==24 && z==13){ return -0.056946;}   if (a==25 && z==13){ return -8.916172;}  
 if (a==26 && z==13){ return -12.210309;}  if (a==27 && z==13){ return -17.196658;} 
 if (a==28 && z==13){ return -16.850441;}  if (a==29 && z==13){ return -18.215322;} 
 if (a==30 && z==13){ return -15.872419;}  if (a==31 && z==13){ return -14.953628;} 
 if (a==32 && z==13){ return -11.061966;}  if (a==33 && z==13){ return -8.529377;}  
 if (a==34 && z==13){ return -3.04;} 	 if (a==35 && z==13){ return -0.21;} 	
 if (a==36 && z==13){ return 5.917;} 	 if (a==37 && z==13){ return 9.83;} 	
 if (a==38 && z==13){ return 16.193;} 	 if (a==39 && z==13){ return 20.356;} 	
 if (a==24 && z==14){ return 10.754673;}   if (a==25 && z==14){ return 3.824318;}   
 if (a==26 && z==14){ return -7.144632;}   if (a==27 && z==14){ return -12.384301;} 
 if (a==28 && z==14){ return -21.4927968;} if (a==29 && z==14){ return -21.895046;} 
 if (a==30 && z==14){ return -24.432928;}  if (a==31 && z==14){ return -22.949006;} 
 if (a==32 && z==14){ return -24.080907;}  if (a==33 && z==14){ return -20.492662;} 
 if (a==34 && z==14){ return -19.95677;}   if (a==35 && z==14){ return -14.360307;} 
 if (a==36 && z==14){ return -12.421;} 	 if (a==37 && z==14){ return -6.607;} 	
 if (a==38 && z==14){ return -4.152;} 	 if (a==39 && z==14){ return 2.291;} 	
 if (a==40 && z==14){ return 5.47;} 	 if (a==41 && z==14){ return 12.174;} 	
 if (a==42 && z==14){ return 15.159;} 	 if (a==27 && z==15){ return -0.71703;}   
 if (a==28 && z==15){ return -7.158753;}   if (a==29 && z==15){ return -16.952626;} 
 if (a==30 && z==15){ return -20.200575;}  if (a==31 && z==15){ return -24.440885;} 
 if (a==32 && z==15){ return -24.305218;}  if (a==33 && z==15){ return -26.337486;} 
 if (a==34 && z==15){ return -24.557669;}  if (a==35 && z==15){ return -24.85774;}  
 if (a==36 && z==15){ return -20.250977;}  if (a==37 && z==15){ return -18.994145;} 
 if (a==38 && z==15){ return -14.75782;}   if (a==39 && z==15){ return -12.873735;} 
 if (a==40 && z==15){ return -8.064;} 	
 if (a==41 && z==15){ return -5.017;} 	 if (a==42 && z==15){ return 0.995;} 	
 if (a==43 && z==15){ return 4.813;} 	 if (a==44 && z==15){ return 9.378;} 	
 if (a==28 && z==16){ return 4.073203;}    if (a==29 && z==16){ return -3.159582;}  
 if (a==30 && z==16){ return -14.062532;}  if (a==31 && z==16){ return -19.044648;} 
 if (a==32 && z==16){ return -26.015697;}  if (a==33 && z==16){ return -26.585994;} 
 if (a==34 && z==16){ return -29.931788;}  if (a==35 && z==16){ return -28.846356;} 
 if (a==36 && z==16){ return -30.664075;}  if (a==37 && z==16){ return -26.89636;}  
 if (a==38 && z==16){ return -26.861197;}  if (a==39 && z==16){ return -23.162245;} 
 if (a==40 && z==16){ return -22.91;} 	 if (a==41 && z==16){ return -19.019105;} 
 if (a==42 && z==16){ return -17.677503;}  if (a==43 && z==16){ return -12.049;} 	
 if (a==44 && z==16){ return -9.102;} 	 if (a==45 && z==16){ return -3.889;} 	
 if (a==31 && z==17){ return -7.067165;}   if (a==32 && z==17){ return -13.329771;} 
 if (a==33 && z==17){ return -21.003432;}  if (a==34 && z==17){ return -24.439776;} 
 if (a==35 && z==17){ return -29.01354;}   if (a==36 && z==17){ return -29.521857;} 
 if (a==37 && z==17){ return -31.761532;}  if (a==38 && z==17){ return -29.798097;} 
 if (a==39 && z==17){ return -29.800203;}  if (a==40 && z==17){ return -27.55781;}  
 if (a==41 && z==17){ return -27.307189;}  if (a==42 && z==17){ return -24.91299;}  
 if (a==43 && z==17){ return -24.14;} 	 if (a==44 && z==17){ return -20.231052;} 
 if (a==45 && z==17){ return -18.361;} 	 if (a==46 && z==17){ return -13.847;} 	
 if (a==47 && z==17){ return -8.919;} 	 if (a==32 && z==18){ return -2.200204;}  
 if (a==33 && z==18){ return -9.384141;} if (a==34 && z==18){ return -18.377217;} 
 if (a==35 && z==18){ return -23.047411;}if (a==36 && z==18){ return -30.23154;}  
 if (a==37 && z==18){ return -30.947659;}if (a==38 && z==18){ return -34.714551;} 
 if (a==39 && z==18){ return -33.242011;}if (a==40 && z==18){ return -35.039896;}
 if (a==41 && z==18){ return -33.067467;}if (a==42 && z==18){ return -34.422675;} 
 if (a==43 && z==18){ return -32.009808;}if (a==44 && z==18){ return -32.673053;} 
 if (a==45 && z==18){ return -29.770589;}if (a==46 && z==18){ return -29.720127;} 
 if (a==47 && z==18){ return -25.907836;}if (a==35 && z==19){ return -11.1689;}   
 if (a==36 && z==19){ return -17.426171;}if (a==37 && z==19){ return -24.800199;} 
 if (a==38 && z==19){ return -28.800691;}if (a==39 && z==19){ return -33.807011;} 
 if (a==40 && z==19){ return -33.535205;}if (a==41 && z==19){ return -35.559074;} 
 if (a==42 && z==19){ return -35.021556;}if (a==43 && z==19){ return -36.593239;} 
 if (a==44 && z==19){ return -35.809606;}if (a==45 && z==19){ return -36.608186;} 
 if (a==46 && z==19){ return -35.418323;}if (a==47 && z==19){ return -35.696272;} 
 if (a==48 && z==19){ return -32.123935;}if (a==49 && z==19){ return -30.319265;} 
 if (a==50 && z==19){ return -25.352141;}if (a==36 && z==20){ return -6.439359;}  
 if (a==37 && z==20){ return -13.16176;} if (a==38 && z==20){ return -22.05922;}  
 if (a==39 && z==20){ return -27.2744;}  if (a==40 && z==20){ return -34.846275;} 
 if (a==41 && z==20){ return -35.137759;}if (a==42 && z==20){ return -38.547072;} 
 if (a==43 && z==20){ return -38.408639;}if (a==44 && z==20){ return -41.468479;} 
 if (a==45 && z==20){ return -40.81195;} if (a==46 && z==20){ return -43.135077;} 
 if (a==47 && z==20){ return -42.340123;}if (a==48 && z==20){ return -44.214129;} 
 if (a==49 && z==20){ return -41.289265;}if (a==50 && z==20){ return -39.570832;} 
 if (a==51 && z==20){ return -35.863251;}if (a==52 && z==20){ return -32.509141;} 
 if (a==39 && z==21){ return -14.168021;}if (a==40 && z==21){ return -20.523228;} 
 if (a==41 && z==21){ return -28.642392;}if (a==42 && z==21){ return -32.121239;} 
 if (a==43 && z==21){ return -36.187929;}if (a==44 && z==21){ return -37.816093;} 
 if (a==45 && z==21){ return -41.067792;}if (a==46 && z==21){ return -41.757115;} 
 if (a==47 && z==21){ return -44.332121;}if (a==48 && z==21){ return -44.496101;} 
 if (a==49 && z==21){ return -46.552368;}if (a==50 && z==21){ return -44.536885;} 
 if (a==51 && z==21){ return -43.218184;}if (a==52 && z==21){ return -40.356541;} 
 if (a==54 && z==21){ return -34.218841;}if (a==55 && z==21){ return -29.580571;} 
 if (a==40 && z==22){ return -8.850275;} if (a==42 && z==22){ return -25.121552;} 
 if (a==43 && z==22){ return -29.321103;}if (a==44 && z==22){ return -37.548459;} 
 if (a==45 && z==22){ return -39.005737;}if (a==46 && z==22){ return -44.123422;} 
 if (a==47 && z==22){ return -44.932394;}if (a==48 && z==22){ return -48.487727;} 
 if (a==49 && z==22){ return -48.558799;}if (a==50 && z==22){ return -51.426672;} 
 if (a==51 && z==22){ return -49.727849;}if (a==52 && z==22){ return -49.464837;} 
 if (a==53 && z==22){ return -46.828839;}if (a==54 && z==22){ return -45.594395;} 
 if (a==55 && z==22){ return -41.670332;}if (a==56 && z==22){ return -38.936785;} 
 if (a==57 && z==22){ return -33.543903;}if (a==44 && z==23){ return -24.11638;}  
 if (a==45 && z==23){ return -31.879629;}if (a==46 && z==23){ return -37.073013;} 
 if (a==47 && z==23){ return -42.002051;}if (a==48 && z==23){ return -44.475385;} 
 if (a==49 && z==23){ return -47.956943;}if (a==50 && z==23){ return -49.221554;} 
 if (a==51 && z==23){ return -52.201383;}if (a==52 && z==23){ return -51.44131;}  
 if (a==53 && z==23){ return -51.848839;}if (a==54 && z==23){ return -49.890954;} 
 if (a==55 && z==23){ return -49.151491;}if (a==56 && z==23){ return -46.080109;} 
 if (a==57 && z==23){ return -44.188742;}if (a==58 && z==23){ return -40.208743;} 
 if (a==59 && z==23){ return -37.066562;}if (a==60 && z==23){ return -32.577268;} 
 if (a==45 && z==24){ return -18.965218;}if (a==46 && z==24){ return -29.473742;} 
 if (a==47 && z==24){ return -34.558385;}if (a==48 && z==24){ return -42.81918;}  
 if (a==49 && z==24){ return -45.330484;}if (a==50 && z==24){ return -50.259499;} 
 if (a==51 && z==24){ return -51.448807;}if (a==52 && z==24){ return -55.416933;} 
 if (a==53 && z==24){ return -55.284741;}if (a==54 && z==24){ return -56.932545;} 
 if (a==55 && z==24){ return -55.107491;}if (a==56 && z==24){ return -55.281245;} 
 if (a==57 && z==24){ return -52.52414;} if (a==58 && z==24){ return -51.834726;} 
 if (a==59 && z==24){ return -47.89149;} if (a==60 && z==24){ return -46.503876;} 
 if (a==61 && z==24){ return -42.180653;}if (a==62 && z==24){ return -40.414553;} 
 return 0;
}


double AMU(int a, int z){
  double mex,amu;
    mex=f_mex(a,z);
    amu= ( mex + a * 931.494)/931.494;
    return amu;
}










  //#################################################
  //#      angular_E3    kinematika reakce.....
  //#################################################



//
//  sl ... solution (if there are 2
//
double angular_E3(int a, int z, int a2, int z2,  int a3, int z3, int a4, int z4, 
		  double t, double theta , int sl=1 , double exctgt=0.0,int  verbose=0){

 double m1,m2,m3,m4,es,p1,costh3;
 //printf("a=%d  z=%d\n",a,z);
     m1=AMU(a,z);
     m2=AMU(a2,z2);
     m3=AMU(a3,z3);
     m4=AMU(a4,z4);
    m1=m1*931.494;    m2=m2*931.494;
    m3=m3*931.494;    m4=m4*931.494 + exctgt;

     es=t + m1  + m2;
     //     p1=sqrt(     (t + m1)**2  - m1**2  );
     p1=sqrt(     pow( (t + m1),2.0)  - pow(m1,2.0)  );
     costh3=cos( theta * 3.1415926/180.0);

//#nerelativ
    double Q=m1+m2-m3-m4;
//#    my $SQne=sqrt( $m1*$m3*$t*$costh3**2 +($m1+$m2)*($m4*$Q+($m4-$m1)*$t)  );
//#    my $t3na=(sqrt($m1*$m3*t)*$costh3 + $SQne )**2 /($m1+$m2)**2;
//#    my $t3nb=(sqrt($m1*$m3*t)*$costh3 - $SQne )**2 /($m1+$m2)**2;
//#    print "Q=$Q ;  T3nerel = $t3na  $t3nb  \n";

//    printf( "               Q=%f , p=%f;  \n",Q,p1);

//#relativ
//    double a3b= es**2 - p1**2 + (m3**2 - m4**2);
    double a3b= pow(es,2.0) - pow(p1,2.0) + ( pow(m3,2.0) -  pow(m4,2.0) );
    //    double SQ=sqrt( a3b**2 - 4*m3**2 * (es**2-p1**2*costh3**2) );
    double SQ=sqrt( pow(a3b,2.0) - 4*pow(m3,2.0) * (  pow(es,2.0)-pow(p1,2.0)*pow(costh3,2.0) ) );

    //    double t3a=(a3b*es + p1*costh3* SQ)/2/(es**2 - p1**2*costh3**2) - m3;
    double t3a=(a3b*es + p1*costh3* SQ)/2/( pow(es,2.0) - pow(p1,2.0)*pow(costh3,2.0) ) - m3;
    //    double t3b=(a3b*es - p1*costh3* SQ)/2/(es**2 - p1**2*costh3**2) - m3;
    double t3b=(a3b*es - p1*costh3* SQ)/2/( pow(es,2.0) - pow(p1,2.0)*pow(costh3,2.0) ) - m3;
    double tresult=0.0;
    //# prepare 2-solutions --- decision......  
    double E1, V, ttr, ttrc, p3c, E3c, rho3;
    E1=t+m1;/// # full energy
    V=p1/( E1 + m2 );// # CMS velocity  pc/E->v/c?
    ttr=- (m1+m2)/m2 * Q; // treshold in lab
       if (Q>0){ttr=0;}
       ttrc=-Q;  // treshold in cms
       if (Q>0){ttrc=0;}
       //     p3c=m2*sqrt( (t-ttr)*(t-ttr+2/m2*m3*m4)/( 2*m2*t + (m1+m2)**2)  );
       p3c=m2*sqrt( (t-ttr)*(t-ttr+2/m2*m3*m4)/( 2*m2*t + pow((m1+m2),2.0) )  );
       //     E3c=sqrt( p3c**2 + m3**2 );
       E3c=sqrt( pow(p3c,2.0) + pow(m3,2.0)  );
     rho3=V/p3c * E3c;
     if (verbose>0){
    printf ("       (%d %d) + (%d %d)  ->  (%d %d)  +  (%d %d)  \n",a, z,a2 ,z2,a3, z3,a4, z4 );
    printf ("        p1   =%15.5f (projectile impuls)\n",p1 );
    printf ("        E1   =%15.5f (projectile - total energy)\n",E1 );
    printf ("        V    =%15.5f (velocity of CMS ... v/c)\n",V );
    printf ("        ttr  =%15.5f (Treshold in Lab)\n",ttr );
    printf ("        ttrc =%15.5f (Treshold in CMS == -Q)\n",ttrc );
    printf ("        Q    =%15.5f (if Q>0 = exoterm)\n",Q );
    printf ("        ExcTg=%15.5f (excitation of target)\n", exctgt );
    printf ("        p3c  =%15.5f\n",p3c );
    printf ("        rho3 =%15.5f (if <=1.0 : 1 solution else 2 solutions for T3)\n",rho3 );
     }

    if (rho3<=1){ t3b=0;}
    if (sl==1){   tresult=t3a;}
    if (sl==2){   tresult=t3b;}
     if (verbose>0){
    printf ("        TKE1 =%15.5f (before reaction)\n",t );
    printf ("        angl =%15.5f (               )\n",theta );
    printf ("        TKE3 =%15.5f (after reaction)\n", t3a );
    if (rho3>1){ 
    printf ("        TKE3b=%15.5f (second solution after reaction)\n", t3b );
    printf ("        TKE3 =%15.5f (output of the program)\n", tresult );
    }//if rho3
     }// verb

     if ( (a+a2!=a3+a4) || (z+z2)!=(z3+z4) ){
       printf("########### ERROR IN angular_E3  input%d\n",1);
       printf("                                      %d\n",1);
       printf("########### ERROR IN angular_E3  input%d\n",1);
       tresult=0.0;
     }
//#    my $t4a=($a4*$es + $p1*$costh4* $SQ)/2/($es**2 - $p1**2*$costh4**2) - $m3;
//#    my $t4b=($a4*$es + $p1*$costh4* $SQ)/2/($es**2 - $p1**2*$costh4**2) - $m3;
//#    print "  $a3b = A3 ;  $t3a = T3A ;   $t3b = T3B \n";

//    printf( "  T3 from  T=%f MeV at angle %f   = %f MeV   (%f)\n",t,theta, t3a,t3b);

//#    print "    kinetic E T4=$t4a ($t4b) \n";
    //    if (rho3<=1){ t3b=0;}
    return tresult;
} // angular_E3





//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//=============================================================
//            ENGINE READ 4 COLUMNS   give -1 if  to be  0
//=============================================================
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
//&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&&
/*  version 2010 04 07 - can read quite a mess...
 *
 */
TGraphErrors* qgr_engine (const char* name, int rx, int ry, int rdx, int rdy)
{
 float x[32000],y[32000],dy[32000],dx[32000],  bu;
 int i,j;
 FILE * pFile;
 int MAXLINES=32000;
 // char mystring[1500];// one line
 TString oneline, title=" ", token;

  printf("going to open filename=%s\n", name );

  pFile=fopen( name ,"r" ); 
  if (pFile==NULL) {
    printf("cannot open %s,STOPping\n", name ); 
    return 0;
  } // error
  //  printf("file opened\n%s","");
  //.......................readout HERE.......i
  i=0;
  while ((i<MAXLINES)&&( feof(pFile)==0) ){
    // printf("  reading line %d...\n", i );
    if ( oneline.Gets (pFile, kTRUE) ){//chop true ... continue if not eof
      //printf("     string==%s\n",  oneline.Data() ); 

    //purify .................. start
    int lastlen;
    do {
      lastlen=oneline.Length();
      if (oneline.Index(" ")==0){oneline=oneline(1,oneline.Length()-1);}
       oneline.ReplaceAll("  "," ");
    }while( lastlen!=oneline.Length());
    //purify .................. stop
    //printf("S=%s\n", oneline.Data() );
   //........ parse oneline
   if (oneline.Index("#")==0){ // starts with # - COMMENT HERE
     title=oneline( 1,oneline.Length()-1  );
     //     title=oneline( oneline.Length(),oneline.Length()-1  );
  }else{// DATA HERE
     if (oneline.Length()>1){
       if (oneline.Index("#")>0){oneline.Remove(oneline.Index("#") );}// #
      x[i]=0; y[i] =0;dx[i]=0;dy[i]=0; j=0;  //  go through the columns 
      /*
       * r?? contains a column to use for x,dx,y,dy:
       */
      TObjArray *tar= oneline.Tokenize(" ");
      while( (j<=rx && rx>=0) || (j<=ry && ry>=0) || (j<=rdy && rdy>=0)|| (j<=rdx && rdx>=0)){  
         token= ((TObjString*)(tar->At(j)))->GetString();
	 bu= token.Atof(); 
         if (rx==j)  { x[i] =bu;}
         if (ry==j)  { y[i] =bu;}
         if (rdx==j) { dx[i] =bu;}
         if (rdy==j) { dy[i] =bu;}
         j++;
      }//small while 
      tar->Delete();
      i++;// skip to a next datum if point
     }// lenght>1
   }//DATA HERE end


    }//if oneline.Gets false
   
   //........ parse oneline
  }//while
  //.......................readout HERE end....i
  //.......................close, create the TGraphErrors
  fclose( pFile);

  printf("%d elements read. title= /%s/", i-1, title.Data() );
 TGraphErrors *g=new TGraphErrors(i,x,y,dx,dy);
 g->SetMarkerStyle(22);
 g->SetTitle(title.Data()  );
 // g->Print();// DONT PRINT TABLES
 return g;
 //-----------------------------------------------------
}




//=============================================================
//            two points in file
//=============================================================
TGraphErrors*  qgr2 (const char* name)
{
  TGraphErrors* res;
  printf("going to create graph 2 col. %s\n",name);
   res=qgr_engine(name,0,1,-1,-1);
   if (res==NULL){ return 0;}
   TGraphErrors  *gr=(TGraphErrors*)res;
   //   printf("gr inside gr2=%d\n", (int)gr);
   //   gr->Draw("pawl");
   return gr;
}




/* **************************************************
 *    spline   to     function  
 *
 *     we want a spline  (ok - from TGraph)
 *     we want a function that can be fit......
 *          .....  qgr_mksplfun
 */
double splinefun(double *x, double *p) {
  TSpline* sp3;
// trick to transfer the address of the spline here
  sp3=(TSpline3*)(int)p[0];  //recover address of  TSpline3*
  return  sp3->Eval(x[0]); 
}


/*****************************************************
 *    spline   to   function
 *
 *     .... look for the function by    name                          funname
 */
TSpline3* qgr_mksplfun(TGraph *cg, float xmin, float xmax, const char *funname="fif_splinefun") {
 int n;
 // float x[1000],y[1000];
 n=cg->GetN();
 if (n<1){printf("%s######## GRAPH INVALID #####################\n","");return 0;}
 TSpline3 *sp=new TSpline3("splname", cg);

 //    cg->Draw("plaw"); sp->SetLineColor(2); sp->Draw("same");

 // to initialize graph function - send (overriden) address as a par
 printf( "%s\n",  "" );
 // printf( "%s\n",  "######## to create function #####################" );
 TF1 *fif_sp=new TF1(funname,splinefun,xmin,xmax,1);
 // printf( "%s\n",  "######## to set parameter #####################" );
 fif_sp->SetParameter(0, (int64_t)sp ); // This is an address of TSpline3 (made int)!
    //  fif->Eval(84.)
 printf( "%s%s%s\n",  "function ",funname," NOW defined in_the_stack and TSpline3 *sp is returned." );
 // printf( "%s\n",  " you can: FillRandom histo, etc" );
 // cout<<"  TH1F *h=new TH1F(\"h\",\"h\",1000,50,275)"<<endl;
 // cout<<"------  histogram range MUST BE INSIDE function range ---- \n";
 // cout<<"  h->FillRandom(\""<<funname<<"\",10000)"<<endl;
 // cout<<".......TSpline3 *sp=(TSpline3*)qgr_mksplfun(g, 50,250) ...  returned now\n";
 // cout<<".......sp->SetLineColor(4); sp->Draw(\"same\");\n";
 return sp;
}













  //#################################################
  //#     losses   -  loads THE TABLES
//     the data are in     ./srim_losses/
  //#################################################



int load_losses_table(const char *fname, int soverwrite=0, const char *charpat=""){
  //==================================================
  // TO LOAD THE dE/dx functions: 
  int overwrite=soverwrite;
  TF1 *f=(TF1*)gROOT->GetListOfFunctions()->FindObject(fname);
  if (f!=NULL){
    printf(" FUNCTION    %15s    ALREADY EXISTS  ...  ",fname);
    if ((f->GetXmin()<0.1)&&(f->GetXmax()>80.)){}else{
      overwrite=1;
    }
    if (overwrite==1){printf("OVERWRITING - but ....\n");
      f->Delete();
    }else{printf(" no action....   %s\n", ""); return 0;}
  }

  /*
Here I will experiment with systemwide  path
gEnv->GetValue("Unix.*.Root.DynamicPath","")

   */
  TString dynapa=gEnv->GetValue("Unix.*.Root.DynamicPath","");
  char* pPath;  pPath = getenv ("HOME");
  dynapa.ReplaceAll( "~", pPath );
  TString tok;
  struct stat info;

  TObjArray *tar=dynapa.Tokenize(":");
  for (int k=0;k<tar->GetEntries();k++){
    tok= ((TObjString*)(tar->At(k)))->GetString();
    tok.Append("/srim_losses/");
    if( stat( tok.Data(), &info ) != 0 ){
      //       printf( "cannot access %s\n", tok.Data() );
    tok.Clear();
    }else{
       if( info.st_mode & S_IFDIR )  // S_ISDIR() doesn't exist on my windows 
	 {
	   //   printf( "%s is a directory\n", tok.Data() );
    break;
	 }else{
	 //      printf( "%s is no directory\n", tok.Data() );
    tok.Clear();
       }
    }
  }//all k

  TString pat;
  if ( strlen(charpat)>0){
    pat=charpat;   //#########   SUBdirectory
    printf("%s is ACCEPTED dir. for losses -overr\n",charpat);
  }else{
    pat=tok;
    if ( pat.Length()==0 ){
      printf("No data path with srim_losses was found%s\n","");
      exit(0);
    }
    printf("%s is ACCEPTED dir. for losses -dynam\n",tok.Data());
  }
  //  pat="srim_losses/";   //#########   SUBdirectory
  pat.Append( fname );
  TGraph *ccc=(TGraph*) qgr2( pat.Data() );
  if (ccc==NULL){ return 1;}
  //  ccc->Draw("pawl");

     TString ss;
     ss.Append(fname);
     ss.Append("  table of losses.");



     TSpline3 *sp=(TSpline3*)qgr_mksplfun(ccc,0.00001, 180,"fif_spl_func2");  // from 0.0001 MeV to 30MeV
     //     sp->SetLineColor(3); sp->Draw("same");
     sp->SetNameTitle(fname,ss.Data() );
  TF1* fif_splinefun;
  fif_splinefun=(TF1*)gROOT->GetListOfFunctions()->FindObject("fif_spl_func2");

  fif_splinefun->SetNameTitle( fname, ss.Data() ); // make the function unique
  fif_splinefun->SetLineColor(4); 
  fif_splinefun->SetNpx(5000);
  fif_splinefun->Draw();
  //  fif_splinefun->Draw("same");
  //  now - reachable   by   gROOT->GetListOfFunctions()->ls()
  // although SetNpx may be small to display the function nicely, Eval() works
  printf("######  splines loaded   %15s     #########\n",fname);

  return 0;
}


int load_losses_table_all(TString allt){
  TString token, tok, tok2;

 TObjArray *tar=allt.Tokenize(" ");
 for (int k=0;k<tar->GetEntries();k++){
   tok= ((TObjString*)(tar->At(k)))->GetString();
   if ( load_losses_table( tok.Data() )!=0){
     return 1;
   }

 }//all k
 tar->Delete();
  return 0;
}







  //#################################################
  //#
  //#     loose_E   -   evaluate the energy loss in material.
  //#
  //#  tables are in   MeV/mm == keV/um
  //#################################################

// E in MeV,  width, step in micrometers,  losses in kev/um
double loose_E( double Eini, char *dedx_func, double width, 
		double step=0.05, int verbose=0){

  /*
   *   trick mode - implantation depth   step < 0
   */
  double x=0.0,dx=step, E=Eini, dE,  emin, emax;

  double stopping;
  int flag_last=0;
  if (dx<0){ dx=-dx;}
  if (verbose>1){ printf("loose_E:   E init == %fMeV, mat:/%s/ ,thick:%fum verbosity %d\n",
			 Eini, dedx_func ,width , verbose);}

  if (E<=0){
    if (verbose>0){ printf(" E init == 0,  no action taken in layer [%s]\n",dedx_func);}
return 0.0;}

  if (verbose>2){printf("preparing to get function %s\n","");}
  TF1 *f=(TF1*)gROOT->GetListOfFunctions()->FindObject(dedx_func);

  if (verbose>2){printf(" spline function found at addr: %d\n", (int64_t)f ); }

  if (f==NULL){return 0;}
   
   emin=f->GetXmin();   
   emax=f->GetXmax(); // watch the limits!

   if (verbose>2){printf("having the function with  (min max) = (%f %f) MeV\n", emin,emax );}

   while (x<width && flag_last==0){
     x=x+dx;   //x=x+step; 
     //     if (x>width){ dx=width-x+step; x=width; flag_last=1;}
     if (x>width){ dx=width-x+dx; x=width; flag_last=1;}// do not use 'step'


     stopping=0.0;
     if (E>=emin && E<emax){
       stopping=f->Eval(E);
     }else{
       stopping=f->Eval(emin); 
     }// set stopping power
     /*
      * PROBLEM - compilation worked with isnan - interpreter not
      */
     //     if (isnan(stopping)){ 
     if ( stopping==0.0 ){ 
       printf("error in loosE func access%s\n","");return -1.0;
     }

     // stopping units = keV/micron  =>>> MeV/micron
     dE=stopping*dx*0.001;

     if (E-dE<0){
       if (verbose>01){
	 printf("   Stopped between %f and %f microns in layer [%s] dEdx=%f\n",
                     x,x+step, dedx_func, stopping); }
	 flag_last=1;
	 dE=E;E=0; 
       }  else{
        E=E-dE;  
     }// E-dE<0
     
     if (verbose>2){printf(" x=%.3f dx=%.3f  dE=%.6f eV  E=%.6f \n",x,dx, dE*1e+6, E);}
   }//  WHILE
   if (verbose>0){printf(" E init=%10.6f, E after layer [%s] %10.3fum  = %.6f MeV, loss = %.9f MeV\n", 
			 Eini,  dedx_func, width,  E, Eini-E);}

   if (step<0.0){ return x;}else{    return E; }// two modes of operation
}



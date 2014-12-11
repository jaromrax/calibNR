/*
 *   nrfit.C    
 *
 *   nuclear reaction  fit
 *    - takes into account kinematics, losses
 *
 *    - can fit experiental channels to energy
 *      and search for minimum CHi2 by variation of different
 *      components (beam energy, target thickness...)
 *
 */

#include "losslib2.h"
#include "manip_table.h"


void nrfit(const char* filename){
  int i;
  double d;

  // PARSE THE table first   
  i=readout_reac_file(filename); 
  if (i!=0){ printf("Error in analyze_react1 ... STOP%s\n","");return;}
  printf("%s\n","INTERPOLATED VALUES");

  i=interpolate_varnames_in_table();
  if (i!=0){ printf("%s\n","Problem during 1st parsing the table.STOP");return;}

  printf("table ... prepared\nstructure ... prepared\nloss tables ... loaded%s\n","" );


  //  preprocess_table();
  d=do_kinematics(1);
  //  printf("total ch2=%lf\n", d );
 

  lincal();

  //  d=do_kinematics(0);
  //  printf("total ch2=%lf\n", d );




}//nrfit=========================================== MAIN END =====

/*

wc=0;root -n -b -q  compile.C 2>compile.log;  cat compile.log; wc=`cat compile.log | grep error | wc -l` ; if [ $wc -eq 0 ]; then  mplayer -really-quiet /usr/share/sounds/gnome/default/alerts/bark.ogg 2>/dev/null; fi;  mplayer -really-quiet /usr/share/sounds/gnome/default/alerts/bark.ogg 2>/dev/null; echo $wc errors


 */
void compile(){
  printf("\n\n\n\n\n%s","");
    printf("==================================================%s\n","");
    printf("\n\n\n\n\n%s","");

  //  .L kibbler_gui.C++  ;
  gROOT->ProcessLine(".L nrfit.C++g");

  printf("\n\nroot -n -b -q  compile.C  %s\n\n\n","");

}

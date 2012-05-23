// Adapted from main.cc

#include "GMH_App.hh"

int main (int argc, char** argv)
{
     if (!g_thread_supported()) g_thread_init (NULL);

     GMH_App* The_App=new GMH_App(argc, argv);
     The_App->run();



     return 0;
}





/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/main.cc,v 2.0 2000/07/22 17:21:14 ckuklewicz Exp $
$Log: main.cc,v $
Revision 2.0  2000/07/22 17:21:14  ckuklewicz
Synchonizing release numbers

*/

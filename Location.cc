#include "Location.hh"

Location::Location(const Location& other) : 
    back(NULL), front(NULL),
    hasMine(other.hasMine),selected(other.selected),
    measured(other.measured),adjacentMines(other.adjacentMines),
    user(other.user), help(other.help) 
{
    // DEBUG cout << "C"; cout.flush(); 
}

Location& Location::operator=(const Location& other) {
    if( this != &other ) {
        // back=NULL;
        // front=NULL;
        hasMine=other.hasMine;
        selected=other.selected;
        measured=other.measured;
        adjacentMines=other.adjacentMines;
        user=other.user; 
        help=other.help;
        // DEBUG cout << "A"; cout.flush();
    };
    return *this;
}

Location::Location() : back(NULL),front(NULL),hasMine(false),
                       selected(false),measured(false),adjacentMines(0),
                       user(NOTHING), help(NONE) 
{ 
    // DEBUG cout << "N"; cout.flush(); 
}

Location::~Location() {
    // DEBUG cout << "D"; cout.flush();
    if (back) {
        delete back;
        back=NULL;
    };
    if (front) {
        front->destroy();
        front=NULL;
    };
}

void Location::reset() {
    // ignore the back tiles
    // just delete the front pixmap
    if (NULL!=front) {
        front->destroy();
        front=NULL;
    };
    hasMine=false;
    selected=false;
    measured=false;
    adjacentMines=0;
    user=NOTHING;
    help=NONE;
}

/*
  $Header: /home/ckuklewicz/cvsroot/gminehunter/Location.cc,v 2.2 2000/07/24 03:57:04 ckuklewicz Exp $
  $Log: Location.cc,v $
  Revision 2.2  2000/07/24 03:57:04  ckuklewicz
  Compile cleanly under -Wall

*/

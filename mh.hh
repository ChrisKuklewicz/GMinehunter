#ifndef INCL_MH_HH
#define INCL_MH_HH

#include "GMH.hh"

#include <stl.h>

/// Global typedefs
//@{ 
/// Type of object used to refer to locations in objects outside Game
typedef guint32 index_type;
/// Comparator used internally in ScanSet
typedef less<index_type> lt_index_type;
/// Type of set in ScanSet
typedef set<index_type,lt_index_type> set_index_type;
//@}


#include "ScanSet.hh"

using namespace std;






/** This is for ordering the sets waiting on the Moves queue.
 *  It will put mines at front of list.  */
struct ScanSet_MoveOrder : binary_function <const ScanSet&,const ScanSet&,bool>
{
    inline bool operator ()(const ScanSet& left,const ScanSet& right) const
	{
            //	    return (left.max_mines()>right.max_mines());
            if (left.max_mines()!=right.max_mines())
            {
                return (left.max_mines()>right.max_mines());
            }
            else
            {
                return (left.get_set()<right.get_set());
            };
	};
};

/// Definition of Comparator function object for (AddType). "IsCertain" ScanSets come first.
struct lt_AddSet : binary_function <const ScanSet&,const ScanSet&,bool>
{
    inline bool operator()(const ScanSet& left,const ScanSet& right) const
	{
	    if (left.is_certain()==right.is_certain())
		return left.get_set()<right.get_set();
	    else
		return left.is_certain();
	};
};

/// Definition of Comparator function object for (KnownType). Ordered by locations.
struct lt_KnownSet : binary_function <const ScanSet&,const ScanSet&,bool>
{
    inline bool operator()(const ScanSet& left,const ScanSet& right) const
	{
	    return left.get_set()<right.get_set();
	};
};


class Game;
typedef Game GameBoardType;
typedef set<ScanSet,lt_KnownSet> KnownType;
// typedef deque<ScanSet> AddType;
typedef set<ScanSet,lt_AddSet> AddType;
//typedef multiset<ScanSet,ScanSet_MoveOrder> MovesType;
typedef set<ScanSet,ScanSet_MoveOrder> MovesType;

/// A sorted set of pair<probability of mine,location index>
typedef pair< double, guint32 > pair_prob_loc_t;
typedef multimap< double, guint32, less<double> > MapProbLoc_t;

#endif

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/mh.hh,v 2.5 2000/08/07 03:22:33 ckuklewicz Exp $
$Log: mh.hh,v $
Revision 2.5  2000/08/07 03:22:33  ckuklewicz
Wow...tons and tons of bugs fixed.
Added the sendPing, received_ping functions.
The COMPUTER_GAME seems to actually work.

Revision 2.4  2000/08/02 13:58:32  ckuklewicz
Add WrapPointer.  Merge GreatGuess into Minehunter

Revision 2.3  2000/07/31 04:22:15  ckuklewicz
Everything compiles and links.  Happy Day
Single user still works.
Need to implement guess_it() from java
The 2^29 limit in cln may need examining

Revision 2.2  2000/07/30 05:54:08  ckuklewicz
CLeaninG UP minHunteR cOde

Revision 2.1  2000/07/22 17:35:34  ckuklewicz
Added

*/

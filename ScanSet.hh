#ifndef INCL_SCANSET_H
#define INCL_SCANSET_H

#include "mh.hh"

/**
 * Scanset is the basic unit of the algorithm, consisting of a
 * set of locations and a min and max number of mines.
 *
 * Note: Location and ScanSet are independent classes
 *
 * ScanSet contains a set of location indices and a
 * minimum and maximum # of mines known to be in those locations
 * Constructors, and single line member functions are inline
 * There are two friend binary operators, * and -
 * which handle intersection and set difference
 * Also MakeNewRefined is used like and operator
 */
class ScanSet
{
public:
    /// Creates an empty set with no locations or mines
    ScanSet() : setIndex(),maxMines(0),minMines(0) {};
    /// Working copy constructor
    ScanSet(const ScanSet& _rScanSet) :
        setIndex(_rScanSet.get_set()),
        maxMines(_rScanSet.max_mines()),
        minMines(_rScanSet.min_mines())
	{};
    /// Destructor needs to do nothing special
    virtual ~ScanSet() {};
    /// Working assignment operator
    ScanSet& operator=(const ScanSet& _rScanSet) {
	_assert(_rScanSet.is_ok());
	setIndex=_rScanSet.get_set();
	maxMines=_rScanSet.max_mines();
	minMines=_rScanSet.min_mines();
	return *this;
    };

    //@{
    /// Factory: special operator* calculates the set intersection
    friend ScanSet operator*(const ScanSet& left,const ScanSet& right);
    /// Factory: special operator- calculates the set difference
    friend ScanSet operator-(const ScanSet& left,const ScanSet& right);
    /// Factory: used if Compared, only Compares setIndex, not mines
    friend bool operator==(const ScanSet& left,const ScanSet& right);
    //@}

    /// Writes text summary of Scanset, useful output function for debugging
    friend ostream& operator<< (ostream& out, const ScanSet& right);

    /// Need non-const version of get_set 
    /// for intersection & difference operations
    set_index_type& grab_set(void) { return setIndex; };
public:
    /// Access to min and max mines
    //@{
    guint32& max_mines() { return maxMines; };
    guint32& min_mines() { return minMines; };
    const guint32 max_mines() const { return maxMines; };
    const guint32 min_mines() const { return minMines; };
    //@}


    /// Functions to access the set of locations, some return success/fail bool
    //@{
    const set_index_type& get_set(void) const { return setIndex;	};
    bool add_index(const index_type _index) { return (setIndex.insert(_index)).second;};
    bool has_index(const index_type _index) const { return (0!=setIndex.count(_index)); };
    bool remove_index(const index_type _index, bool _isMine);
    size_t size() const { return setIndex.size(); };
    //@}


    /// Informational methods
    //@{
    /// Sanity check to see if this is consistant, aborts program if not
    bool is_ok() const;
    /// Check if the ScanSet really knows anything
    bool is_trivial() const 
    { 
        return ((max_mines()==size())&&(min_mines()==0));	
    };
    /// Check to see if the ScanSet knows everything
    bool is_certain() const 
    { 
        return ((max_mines()==0)||(min_mines()==size()));	
    };
    /// Check if this can impove the bounds in the passed Scanset.
    bool can_refine(const ScanSet& _rScanSet) const 
    {
	return ((max_mines()<_rScanSet.max_mines())||
		(min_mines()>_rScanSet.min_mines()));
    };
    //@}


    /// Factory: generates the smarter set
    ScanSet make_new_refined(const ScanSet& _rScanSet) const;

private:
    set_index_type setIndex;
    guint32 maxMines;
    guint32 minMines;
};

////////////////////////////////////////////////////////////////

#endif

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/ScanSet.hh,v 2.5 2000/08/02 13:58:32 ckuklewicz Exp $
$Log: ScanSet.hh,v $
Revision 2.5  2000/08/02 13:58:32  ckuklewicz
Add WrapPointer.  Merge GreatGuess into Minehunter

Revision 2.4  2000/07/31 04:22:15  ckuklewicz
Everything compiles and links.  Happy Day
Single user still works.
Need to implement guess_it() from java
The 2^29 limit in cln may need examining

Revision 2.3  2000/07/30 05:54:08  ckuklewicz
CLeaninG UP minHunteR cOde

Revision 2.2  2000/07/24 03:57:04  ckuklewicz
Compile cleanly under -Wall

Revision 2.1  2000/07/22 17:35:34  ckuklewicz
Added

*/

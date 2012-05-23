#include "mh.hh"

/// Helper functions [inline]...libraries usually don't have 3 way max/min
//@{

inline int min(const int& x,const guint32& y)
{
    int a=y;
    if (x<a)
	return x;
    else
	return y;
};

//@}

// --------------------------------------------------------------------

ScanSet operator*(const ScanSet& left,const ScanSet& right)
{
    ScanSet C;
    _assert(left.is_ok());
    _assert(right.is_ok());
    _assert(0!=left.size());
    _assert(0!=right.size());

    set_intersection(left.get_set().begin(), left.get_set().end(), right.get_set().begin(),
		     right.get_set().end(),inserter(C.setIndex, C.setIndex.begin()),lt_index_type());  // XXX : This is killing the program (13 13 25) every time because right is empty ? 

    // basic requirements
    // required for C.min_mines to make sense
    // careful about signed vs unsigned
    int a,b,c;
    a=left.max_mines();
    a-=right.min_mines();
    a+=(right.size()-C.size());
    if (a<0)
    {
        cerr << left;
        cerr << right;
        cerr << C;
    }
    _assert(a>=0);

    b=right.max_mines();
    b-=left.min_mines();
    b+=(left.size()-C.size());
    if (b<0)
    {
        cerr << left;
        cerr << right;
        cerr << C;
    }
    _assert(b>=0);

    C.max_mines()=min(min(left.max_mines(),right.max_mines()),C.size());

    a=left.min_mines();
    a-=(left.size()-C.size());
    b=right.min_mines();
    b-=(right.size()-C.size());
    c=max(max(0,a),b);
    C.min_mines()=c;

    _assert(C.min_mines()<=left.max_mines());
    _assert(C.min_mines()<=right.max_mines());
    // Could realize we could do a refine op at this point...
    // if C.Min>left.Min then refine left
    // if C.Min>left.Min then refine right
    _assert(C.is_ok());
    return C;
}

ScanSet operator-(const ScanSet& left,const ScanSet& right)
{
    ScanSet D;
    int Cmin(0),Cmax(0);

    _assert(left.is_ok());
    _assert(right.is_ok());
    set_difference(left.get_set().begin(), left.get_set().end(), right.get_set().begin(),
		   right.get_set().end(),inserter(D.setIndex, D.setIndex.begin()),lt_index_type());

    // D.set=A.set-B.set
    // (A*B).Size = A.Size-D.Size
    // D.Max = A.Max - (A*B).Min
    // D.Min = A.Min - (A*B).Max
    // 	D.max_mines()=left.max_mines()-max(0,left.min_mines()-D.size(),
    // 			right.min_mines()-(right.size()-(left.size()-D.size())));
    // 	D.min_mines()=left.min_mines()-min(left.max_mines(),right.max_mines(),
    // 			left.size()-D.size());
    int a,b,c;

    a=left.min_mines()-D.size();
    b=right.min_mines()-(right.size()-(left.size()-D.size()));
    c=max(max(0,a),b);
    Cmin=c;

    a=left.size()-D.size();
    c=min(a,min(left.max_mines(),right.max_mines()));
    Cmax=c;

    a=left.max_mines()-Cmin;
    b=D.size();
    c=min(max(0,a),b);
    D.max_mines()=c;

    a=left.min_mines()-Cmax;
    c=max(0,a);
    D.min_mines()=c;
    _assert(D.is_ok());
    return D;
}

inline bool operator==(const ScanSet& left,const ScanSet& right)
{
    return (left.get_set()==right.get_set());
};


ostream& operator<< (ostream& out, const ScanSet& right)
{
    // A1 and A2 are used to traverse right
    set_index_type::const_iterator A1,A2;
    out << "Scan Set. Size: " << right.size() << " Min: " << right.min_mines();
    out << " Max: " << right.max_mines() << endl << "Locations:";
    A1=right.get_set().begin();
    A2=right.get_set().end();
    for(;A1!=A2;++A1)
	{
	    out << " " << (*A1);
	};
    out << endl;
    return out;
};

// --------------------------------------------------------------------

bool ScanSet::remove_index(const index_type _index, bool _isMine)
{
    _assert(is_ok());
    if (setIndex.erase(_index))
    {
        if (_isMine)
        {
            if (0<minMines)
            {
                --minMines;
            }
            if (0<maxMines)
            {
                --maxMines;
            }
            else
            {
                _never_get_here;
            }
        }
        else
        {
            if (maxMines==1+size())
            {
                --maxMines;
            }
        }
        _assert(is_ok());
        return true;
    }
    else
    {
	return false;
    }
}

ScanSet ScanSet::make_new_refined(const ScanSet& _rScanSet) const
{
    ScanSet S(_rScanSet);
    _assert(is_ok());
    _assert(_rScanSet.is_ok());
    S.max_mines()=min(_rScanSet.max_mines(),max_mines());
    S.min_mines()=max(_rScanSet.min_mines(),min_mines());
    _assert(S.is_ok());
    return S;
};

bool ScanSet::is_ok() const
{
    size_t s=size();
    if ((s<maxMines) || (minMines<0) ||
        (minMines>maxMines))
    {
        return false;
    }
    else
    {
        return true;
    }
};

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/ScanSet.cc,v 2.6 2000/08/07 03:22:33 ckuklewicz Exp $
$Log: ScanSet.cc,v $
Revision 2.6  2000/08/07 03:22:33  ckuklewicz
Wow...tons and tons of bugs fixed.
Added the sendPing, received_ping functions.
The COMPUTER_GAME seems to actually work.

Revision 2.5  2000/08/02 13:58:32  ckuklewicz
Add WrapPointer.  Merge GreatGuess into Minehunter

Revision 2.4  2000/07/31 04:22:15  ckuklewicz
Everything compiles and links.  Happy Day
Single user still works.
Need to implement guess_it() from java
The 2^29 limit in cln may need examining

Revision 2.3  2000/07/30 23:23:06  ckuklewicz
Mostly cleaned up!

Revision 2.2  2000/07/30 05:54:08  ckuklewicz
CLeaninG UP minHunteR cOde

Revision 2.1  2000/07/22 17:35:34  ckuklewicz
Added

*/

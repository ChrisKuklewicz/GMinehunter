#include "Minehunter.hh"
#include "WrapSlot.hh"
#include <strstream>

class Gtk_Timer
{
    GTimer* timer;
public:
    Gtk_Timer() { timer=g_timer_new(); };
    ~Gtk_Timer() {  g_timer_destroy(timer); timer=NULL; };
    inline void start() { g_timer_start(timer); };
    inline void stop() { g_timer_stop(timer); };
    inline gdouble secs() { return g_timer_elapsed(timer,NULL); };
    inline gulong usecs() {
        gulong us; g_timer_elapsed(timer,&us); return us; };
    inline gdouble elapsed(gulong& us) { 
        return g_timer_elapsed(timer,&us); };
    inline void reset() { g_timer_reset(timer); };
};

void Minehunter::queue_set(const ScanSet& S)
{
    _assert(S.is_ok());
    if (S.is_trivial())
        return;
    if (S.is_certain())
    {
        bool mineflag=(0!=S.max_mines() ? true : false);
        set_index_type::const_iterator A(S.get_set().begin());
        const set_index_type::const_iterator& end(S.get_set().end());
        // Now add each element of S as a seperate ScanSet
        // into the Moves list. Start with iterators on S
        for (;A!=end;++A)
        {
            if ( !game_board_.is_marked(*A) )
            {
                ScanSet I;
                // First put the info into I
                I.add_index(*A);
                if (mineflag)
                {
                    I.max_mines()=1;
                    I.min_mines()=1;
                }
                // Then copy I into Moves
                _assert(I.is_ok());
                _assert(0<I.size());
                _assert(I.is_certain());
                moves_.insert(I);	// ignore return value
            }
        }
        return;
    }
    KnownType::iterator it;
    it=known_.find(S);
    if (known_.end()!=it) {
        if (S.can_refine(*it)) {
            ScanSet I(S.make_new_refined(*it));
            known_.erase(it);
            queue_set(I);  // recursive call (only once)
        }
    }
    else
    {
        pair<AddType::iterator, bool> p;
        p=add_.insert(S);
        if (!p.second)
	{
            AddType::iterator T;
	    T=p.first;
	    if (S.can_refine(*T))
            {
                ScanSet I(S.make_new_refined(*T));
                add_.erase(T);	// dump old version
                p=add_.insert(I);	// replace with new version
                _assert(p.second);
            }
	}
    }
    return;
}

void Minehunter::setup(guint32 x_max,guint32 y_max,guint32 mines)
{
    _assert(0<x_max);
    _assert(0<y_max);
    _assert(mines<x_max*y_max);
    switch(state_)
    {
        case NEEDS_SETUP:
            game_board_.reset(x_max,y_max,mines);
            state_=NEEDS_SCANSET;
            break;
        default:
            break; // DO NOTHING
    }
}

/// Accepts and trims the newset
void Minehunter::add_scanset(ScanSet * newset)
{
    _show_args1(*newset);
    _nullChk(newset);
    switch(state_)
    {
        case NEEDS_SCANSET: 
            state_=PROCESSING;  
            // fall through, no break
        case PROCESSING:
            {
                ScanSet S=game_board_.trim(*newset);
                game_board_.mark_measured(S);
                queue_set(S);
                delete newset;
                newset=NULL;
            }
            break;
        default:
            break; // DO NOTHING
    }
}

/// If new information, then queue_set it.
void Minehunter::mark_empty(guint32 loc)
{
    _show_args1(loc);
    _assert(loc<game_board_.locations());
    switch(state_)
    {
        case NEEDS_SCANSET:  
            state_=PROCESSING;
            // fall through
        case PROCESSING:
            if (!game_board_.is_marked(loc))
            {
                ScanSet S;
                S.add_index(loc);
                queue_set(S);  // will route to moves_
            }
            break;
        default:
            break; // DO NOTHING
    }
}

/// If new information, then queue_set it.
void Minehunter::mark_mine(guint32 loc)
{
    _show_args1(loc);
    _assert(loc<game_board_.locations());
    switch(state_)
    {
        case NEEDS_SCANSET:  
            state_=PROCESSING;
            // fall through
        case PROCESSING:
            if (!game_board_.is_marked(loc)) 
            {
                ScanSet S;
                S.add_index(loc);
                ++S.min_mines();
                ++S.max_mines();
                queue_set(S);  // will route to moves_
            }
            break;
        default:
            break; // DO NOTHING
    }
}

void Minehunter::game_over(GameEnd ending)
{
    _show_args1((int)ending);
    switch(state_)
    {
        case NEEDS_SCANSET:        // Fall through
        case PROCESSING:           // Fall through
        case NEEDS_GAMEOVER:
            state_=NEEDS_SETUP;
            known_.clear();
            add_.clear();
            moves_.clear();
            break;
        default:
            break; // DO NOTHING
    }
}

void Minehunter::process()
{
    _show_args3(moves_.size(),add_.size(),known_.size());
    _assert(PROCESSING==state_);
    // Choose action
    if (0<moves_.size())
    {
        move_it();
        // Did we win?
        if (game_board_.complete())
        {
            game_over(WON_GAME);
        }
    }
    else if (0<add_.size())
    {
        add_it();
    }
    else
    {
        // Essentially the thread has nothing else to process
        // and may need to guess
        state_=NEEDS_SCANSET;
        //if (0==pings_pending_)
        //{
        ++pings_pending_;
        _show_args1(pings_pending_);
        sendPing.emit(true);
        //}
        // When the ping reply arrives at received_ping, the system
        // will decide if it still needs to call guess_it.
        // It is quite possible that new info arrives in the
    }
}

void* Minehunter::main(void* signal_buffer)
{
    _show_args1(signal_buffer);
    _nullChk(signal_buffer);
    // Experiment:
    AbstractSignalBuffer *absb_ = static_cast<AbstractSignalBuffer*>(signal_buffer);
    // Cannot use dynamic cast on void* so use intermediate absb_:
    sb_ = dynamic_cast<SafeSignalBuffer*>(absb_);
    sb_->reference(); // since it is a Sigc::Object
    // Hopefully if void* is to random memory, it won't accidentally
    // look like a SafeSignalBuffer.
    while ((state_!=CANCELED)&&(!is_canceled))
    {
        switch(state_)
        {
            case NEEDS_SETUP:
                _messsage("NEEDS_SETUP");
                sb_->wait_for_change(0,0);
                break;
            case NEEDS_SCANSET:
                // We are here if we were just setup
                // or if we just sent a "might guess" ping
                _messsage("NEEDS_SCANSET");
                sb_->wait_for_change(0,0);
                break;
            case NEEDS_GAMEOVER:
                _messsage("NEEDS_GAMEOVER");
                sb_->wait_for_change(0,0);
                break;
            case PROCESSING:
                _messsage("PROCESSING");
                process();
                break;
            case CANCELED:   // Can happen when another thread calls destroy()
                _messsage("CANCELED");
                break;
            default:
                _never_get_here;
        }
        if ((state_!=CANCELED)&&(!is_canceled))
        {
            sb_->callEvents.emit();
        }
    }
    _messsage("Minehunter main loop has been CANCELED, exiting thread");
    sb_->unreference(); // since it is a Sigc::Object
    delete this;
    return NULL;
}

////////////////////////////////////////////////////////////
// add_it is a very busy function
// Logic:
// 		if not is_ok or is_trivial or is_certain, abort.
// 		if same set as Known, try to refine, return
// 		loop over Known sets it intersects
// 			if added set is subset of Known
// 				if intersect is refined, put in Add
// 				if nontrivial difference, put in Add
// 			if Known set is subset of added
// 				if intersect is refined, put in Add
// 				if nontrivial difference, put in Add
// 			else
// 				put in Add
// 				put non trivial differences in Add
// 		after loop, put added into Known
void Minehunter::add_it()
{
    // S will be item to add, I is auxillary set
    ScanSet S=ScanSet(),I=ScanSet();
    // It and It_next used to traverse Known
    KnownType::iterator it,it_next;
    // Used in loop over known
    bool flag_next=false;

    // pop off the set at the front of the add queue
    _assert(0<add_.size());
    S=*(add_.begin());
    add_.erase(add_.begin());
    _assert(S.is_ok());                 // paranoia
    _assert(!S.is_trivial());           // paranoia
    _assert(!S.is_certain());           // paranoia

    // DEBUG cerr << "add_it popped " << S << endl;

    // We need to check for uniqueness
    it=known_.find(S);
    if (known_.end()!=it)
    {
        if (S.can_refine(*it))
        {
            I=S.make_new_refined(*it);
            known_.erase(it);
            queue_set(I);
        }
        else
        {
            ; // Do nothing
        }
        return;  // Stop processing
    }

    // Need to Compare S with all elements in Known
    // And look for intersections
    for (it=known_.begin();it!=known_.end();it=it_next) 
    {
        _assert(0<(*it).size());  // Guard against old bug
	// Get the set intersection
	// Assume that S and (*it) must not have the same set of locations
	I=S*(*it);
	if (!I.is_trivial())
        {
            // Found a new set
            // Check against S
            if(S.size()==I.size())
            {
                // S is a subset of (*It)
                if (I.can_refine(S))
                {
                    // new info in intersection I=S*(*it)
                    queue_set(I);
                    // TODO : We could stop here and return....?
                }
                // get set difference
                I=(*it)-S;
                if (!I.is_trivial())
                {
                    // Found a new set: the difference (*it)-S
                    queue_set(I);
                }
            }
            else if (it->size()==I.size())   // Check against (*It)
            {
                // (*It) is a subset of S
                if (I.can_refine(*it))
                {
                    // new info in intersection I=S*(*it)
                    it_next=it;
                    ++it_next;
                    flag_next=true;  // We have handled setting it_next
                    known_.erase(it);  // blow this away explicitly
                    queue_set(I);
                }
                // get set difference, without using it
                I=S-I;  // I hope this works :)
                if (!I.is_trivial())
                {
                    // Found a new set: the difference S-(S*(*it))
                    queue_set(I);
                }
            }
            else
            {
                // Not a subset relationship
                // Found a new Set: the intersection (*it)*S
                queue_set(I);
                // Get first difference
                I=S-(*it);
                if (!I.is_trivial())
                {
                    // Found a new Set: the difference S-(*it)
                    queue_set(I);
                }
                // Get second difference
                I=(*it)-S;
                if (!I.is_trivial())
                {
                    // Found a new Set: the difference (*it)-S
                    queue_set(I);
                }
            }
        }
	if (!flag_next) {
	    it_next=it;  // Safe to reference *It still
	    ++it_next;  // get the next one...
	} else {
            flag_next=false;
        }
    }

    // DEBUG cerr << "add_it adding the set to known" << endl;
    known_.insert(S);
    return;
}

void Minehunter::move_it()
{
    _show_args0();
    // S will be the location to move to
    ScanSet S=ScanSet(),I=ScanSet();
    // loc is location in S to move to
    index_type loc;
    // is_mine is whether S has a mine
    bool is_mine;
    // index points to location in a ScanSet
    set_index_type::const_iterator index;
    // a and a_next used to traverse Add, points to scanset
    AddType::iterator a,a_next;
    // k and k_next used to traverse Known, points to scanset
    KnownType::iterator k,k_next;
    // m and m_next used to traverse Moves, points to scanset
    MovesType::iterator m,m_next;

    // pop S from front of moves_
    S=*(moves_.begin());
    moves_.erase(moves_.begin());
    _assert(S.is_ok());                // paranoia
    _assert(1==S.size());              // paranoia
    _assert(S.is_certain());           // paranoia

    // DEBUG cerr << "move_it popped " << S << endl;

    // Pull info from S
    loc=*(S.get_set().begin());
    is_mine=(S.max_mines()!=0);

    // Loop through Add
    // This loop adapted to editing a few elements in a set
    for (a=add_.begin();a!=add_.end();a=a_next)
    {
        a_next=a;
        ++a_next;
        // does a have loc in it?
        index=a->get_set().find(loc);
        if (index!=a->get_set().end())
        {
            // remove S from a
            I=*a;
            add_.erase(a);  // remove before surgery on *a
            I.remove_index(loc,is_mine);
            queue_set(I);
            // FIXME: Added paranoia, start loop over?
            a_next=add_.begin();
        }
    }


    // Loop through Known
    for (k=known_.begin();k!=known_.end();k=k_next)
    {
        k_next=k;
        ++k_next;
        _assert(0<k->size());  // Guard against old bug
        // does k have loc in it?
        index=(*k).get_set().find(loc);
        if (index!=k->get_set().end())
        {
            // Need to put (*k)-S into Add
            // get a copy of (*k)
            I=(*k);
            // erase k from Known, invalidating k
            known_.erase(k);
            // Remove the location and its mine
            I.remove_index(loc,is_mine);
            // put the replacement ScanSet into Add/Known
            queue_set(I);
            // FIXME: Added paranoia, reset loop to beginning of Known
            k_next=known_.begin();
        }
    }

    // Loop through Moves
    for (m=moves_.begin();m!=moves_.end();m=m_next)
    {
        m_next=m;
        ++m_next;
        index=m->get_set().find(loc);
        if (index!=m->get_set().end())
        {
            moves_.erase(m);  // remove and forget
            // FIXME: Added paranoia, start loop over?
            m_next=moves_.begin();
        }
    }

    // Send correct signal
    bool new_info;
    if (!is_mine) 
    {
        new_info=game_board_.mark_empty(loc);
        _assert(new_info);  // paranoia
        markEmpty.emit(loc);
    }
    else
    {
        new_info=game_board_.mark_mine(loc);
        _assert(new_info);  // paranoia
        markMine.emit(loc);
    }
}

void Minehunter::receive_ping(bool new_ping)
{
    _show_args2(new_ping,pings_pending_);
    if (new_ping)
    {
        sendPing.emit(false);
    }
    else
    {
        --pings_pending_;
        // See if there are any other pings we are waiting on...
        if (0==pings_pending_)
            switch(state_)
            {
                case NEEDS_SCANSET:  // Fall through
                case PROCESSING:
                    if ( (moves_.size()==0) && (add_.size()==0) )
                    {
                        guess_it();
                    }
                    break;
                default:
                    // ignore ping return in this case
                    break;
            }
    }
}

void Minehunter::fake_guess(string& out_report)
{
    _show_args0();
    ostrstream report;
    pair_prob_loc_t p_l;             
    report << "Having to fake the guess because there are ";
    report << game_board_.measured() << endl;
    report << "measured locations which is larger than 32." << endl;
    if (0<game_board_.unmeasured())
    {
        p_l.first=0.4;
        p_l.second=game_board_.locations();
        prob_loc_->insert(p_l);
        report << "Guessing unmeasured with fake 0.4 prob." << endl;
    } 
    else 
    {
        p_l.first=0.4;
        p_l.second=*(known_.begin()->get_set().begin());
        prob_loc_->insert(p_l);
        report << "Guessing the first location I found (" << p_l.second;
        report << ") with 0.4 prob." << endl;
    }
    report << ends;
    out_report=report.str();
    return;
}

void Minehunter::guess_it()
{
    _show_args0();
    // DEBUG cerr << known_.size() << " " << sb_->size();
    string init_report,time_report, prob_report;

    // This only exits if the game has been setup but there
    // is no data yet.
    if (game_board_.unmeasured()==game_board_.locations())
        return;
    clean_up();
    //if (32<game_board_.measured())
    //{
        // fake_guess(prob_report);
        // publishText.emit(prob_report);
        // _show_args1(prob_report); 
    //}
    //else
    {
        // Fill prob_loc_
        if (initialize(init_report))
        {
            publishText.emit(init_report);
            _show_args1(init_report);
        
            if (!call_op(time_report))
            {
                publishText.emit("op was aborted");
                delete prob_loc_;
                prob_loc_=NULL;
                return;
            }
            publishText(time_report);
            _show_args1(time_report);

            calc_prob(prob_report);  // work with BIG numbers
            publishText.emit(prob_report);
            _show_args1(prob_report); 
        }
        else
        {
            // There were no measured locations.
            publishText.emit(init_report);
            _show_args1(init_report);
        }
    }

    // Check the guess signal for a receiver
    if (!guess.empty())
    {
        guess.emit(prob_loc_); // Pass on data
        prob_loc_=NULL;  // Forget data
    }
    else
    {
        delete prob_loc_;  // Destroy data
        prob_loc_=NULL;  // Clear pointer
    }
}

bool Minehunter::call_op(string &out_report)
{
    // Do timing analysis of guess op
    Gtk_Timer timer;
    ostrstream report;

    timer.start();
    op(); // work very hard
    timer.stop();
    if (!sb_->empty())         // Check for incomoing messages
    {
        return false; // abort
    }
    report << "Duration of op for " << (m_max_+1)
           << " measured locations was " << timer.secs() << " seconds." << endl;
    report << ends;
    out_report=report.str();
    return true;
}

bool Minehunter::initialize(string& out_report)
{
    _show_args0();
    KnownType::const_iterator kit_(known_.begin()),k_end(known_.end());
    guint32 k,loc,m;
    set_index_type::const_iterator sit, send;
    ostrstream report;
    map<guint32,guint32>::const_iterator it_loc;
    bool break_out=false;

    m_=0;
    min_mm_ = maxloc;
    max_mm_ = 0;
    report << "Initializing guessing algorithm, look for 'certain' ScanSets." << endl;
    for(;kit_!=k_end;)
    {
        if (kit_->min_mines()==kit_->max_mines())
        {
            guint32 just_added = 0;
            // DEBUG cerr << "found certain in known: " << *kit_;
            k=k_max_;
            ++k_max_;
            // formally request the additional room in the vectors
            k_tot_.resize(k_max_);
            k_mines_.resize(k_max_);
            // record # of mines in the known_ scanset
            k_tot_[k]=kit_->min_mines();
            // Loop over locations in the known_ scanset
            send=kit_->get_set().end();
            for(sit=kit_->get_set().begin(); sit!=send;)
            {
                loc=(*sit);
                it_loc=loc_m_.find(loc);
                if (loc_m_.end()!=it_loc)
                {
                    m=it_loc->second;
                    _assert(loc==m_loc_[m]);   // paranoia
                    m_known_[m].push_front(k);
                    ++sit;  // increment loop
                }
                else 
                {
                    m=m_max_;
                    if (m<32)  // Test for overflow
                    {
                        ++m_max_;
                        ++just_added;  // In case we later exceed 32 locations
                        _assert(0==m_loc_[m]);
                        // assign index m to location loc
                        m_loc_[m]=loc;
                        loc_m_[loc]=m;
                        _assert(m==loc_m_[m_loc_[m]]);
                        m_known_[m].push_front(k);
                        ++sit;  // increment loop
                    }
                    else
                    {
                        // Undo the already executed part of sit/send loop
                        send=sit;
                        for(sit=kit_->get_set().begin(); sit!=send;++sit)
                        {
                            loc=(*sit);
                            m=loc_m_[loc];
                            m_known_[m].pop_front();
                        }
                        --k_max_;
                        m_max_ -= just_added;
                        kit_=k_end;
                        break_out=true;
                    }
                }
            }
        }
        if (!break_out)
            ++kit_;  // increment loop
    }
    if (0==k_max_)
    {
        // All measured locations must have been mines! Oh dear...
        guint32 num_locs=game_board_.unmeasured();
        guint32 remaining=game_board_.mines()-game_board_.mined();
        pair_prob_loc_t p_l;
        p_l.first=remaining;
        p_l.first/=num_locs;
        p_l.second=game_board_.locations();
        prob_loc_->insert( p_l );
        report << "Only unmeasured locations, prob is" << p_l.first << endl;
        report << ends;
        out_report = report.str();
        return false;
    }
    if ( break_out )
    {
        report << "Only the first " << m_max_ 
               << " measured locations out of " << game_board_.measured()
               << " were used." << endl;
    }
    --m_max_; // So that m_mined_[m_max_] is a valid entry
    total_mines_=game_board_.mines()-game_board_.mined();
    num_unmeasured_=game_board_.unmeasured();
    report << "Found " << k_max_ << " 'certain' ScanSets" << endl;
    /* Disable verbose reporting
    for (k=0; k < k_max_; ++k)
    {
        report << "known #" << k << " has " << k_tot_[k] << " mines." << endl;
    }
    for (m=0; m<=m_max_; ++m) 
    {
        report << "location " << m_loc_[m] << " in known #'s";
        kend=m_known_[m].end();
        for (k1=m_known_[m].begin(); k1 != kend; ++k1)
        {
            report << " " << *k1;
        }
        report << endl;
    }
    */
    report << "Total mines left to find: " << total_mines_ << endl;
    report << "Measured locations: " << (m_max_+1) << endl;
    report << "Unmeasured locations: " << num_unmeasured_ << endl;
    report << ends;
    out_report = report.str();
    return true;
}

/// Build for pure speed. No local variables.
/// Can recusively call op(m+1) zero *or* one *or* two times.
/// Start with op(0) and it ends with op(m_max_) at the deepest.
void Minehunter::op() // operates on index this->m
{
    //    _show_args1(m_);
    result_=0;  // zero will mean we found a working layout

    // Try to set m_mined_[m1]=1 and set result_
    kend=m_known_[m_].end();
    for (k1=m_known_[m_].begin(); k1 != kend; ++k1)
    {
        k_=(*k1);
        if (k_mines_[k_] < k_tot_[k_])
        {
            ++k_mines_[k_];
            // This check tries to avoid later loop over all k_
            if (k_mines_[k_] < k_tot_[k_])
            {
                // MOST COMMON PATH to set result_=-1
                result_=-1; // too low..not enough mines yet
                // but keep laying down mines
            }
        }
        else
        {
            for(k2=m_known_[m_].begin(); k2 != k1; ++k2)
            {
                --k_mines_[*k2];
            }
            result_=+1; // too high..failed to set m_mined_[m]=1
            break;  // exits the for (k1) loop
        }
    }
    if (+1!=result_)
    {
        m_mined_[m_]=1; // we succeeded to set m_mined_[m]=1
        ++measured_mines_; // total of m_mined_[]
        if (0==result_)
        {
            // Check thoroughly
            for(k_=0; k_<k_max_; ++k_)
            {
                if (k_mines_[k_] < k_tot_[k_])
                {
                    result_=-1;  // too low..not enough mines yet
                    break;       // exits the for(k_<k_max_) loop
                }
            }
        }
    }

    if (-1==result_)
    {
        // MOST COMMON PATH
        if ((measured_mines_<total_mines_) && (m_<m_max_))
        {
            // MOST COMMON PATH
            ++m_;
            op();  // **RECURSE** with m_mined_[m]=1
            --m_;
            // WARNING: the result_ variable is now undefined
            // WARNING: the kend variable is now undefined
        }

        // set m_mined_[m]=0
        kend=m_known_[m_].end();
        for (k1=m_known_[m_].begin(); k1 != kend; ++k1)
        {
            --k_mines_[*k1];
        }
        m_mined_[m_]=0;
        --measured_mines_;
    }
    else if (0==result_)
    { 
        // check for incoming messages or "death"
        // if they exist, abort guessing operation
        if (!sb_->empty() || is_canceled)
        {
            m_max_=0; // no more recursion will be done, exit all
        }
        else
        {
            // record the working layout
            ++mm_ways_[measured_mines_];
            if (measured_mines_ > max_mm_)
            {
                max_mm_=measured_mines_;
            }
            if (measured_mines_ < min_mm_)
            {
                min_mm_=measured_mines_;
            }
            for (m_temp=0; m_temp<=m_; ++m_temp) // loop can stop at m_temp==m_
            {
                mm_m_ways_[measured_mines_][m_temp] += m_mined_[m_temp];
            }

            // set m_mined_[m]=0
            kend=m_known_[m_].end();
            for (k1=m_known_[m_].begin(); k1 != kend; ++k1)
            {
                --k_mines_[*k1];
            }
            m_mined_[m_]=0;
            --measured_mines_;
        }
    }

    // Always **RECURSE** with m_mined_[m]=0
    if (m_<m_max_)
    {
        ++m_;
        op();
        --m_;
    }
}

/// This erases all but prob_loc_
void Minehunter::clean_up()
{
    _show_args0();
    guint32 m,mm;  // unused: loc,k

    measured_mines_=0;
    total_mines_=0;
    num_unmeasured_=0;
    m_max_=0;
    for (m=0; m<maxloc; ++m)
    {
        m_known_[m].clear();
        m_mined_[m]=0;
        m_loc_[m]=0;
        
        for (mm=0; mm<maxloc; ++mm)
        {
            mm_ways_[m]=0;
            mm_m_ways_[mm][m]=0;
        }
    }
    loc_m_.clear();
//      for (loc=0; loc<1024; ++loc) // XXX
//      {
//          loc_m_[loc]=0;
//      }
    k_max_=0;
    k_mines_.clear();
    k_tot_.clear();
//      for (k=0; k<1024; ++k)
//      {
//          k_mines_[k]=0;
//          k_tot_[k]=0;
//      }
    if (NULL==prob_loc_)
    {
        prob_loc_=new MapProbLoc_t();
    }
    else
    {
        delete prob_loc_;
        prob_loc_=NULL;
        prob_loc_=new MapProbLoc_t();
    }
}

// This function is in a separate file since it needs
// the CLN, a Class Library for Numbers by Bruno Haible
// Found via the www.fsf.org list of GNU GPL software
// Does infinite precision arithmetic
// From cln.h:
#define  WANT_OBFUSCATING_OPERATORS
// Basic types and definitions.
#include "cl_types.h"
#include "cl_object.h"
// Abstract number classes.
#include "cl_number.h"
#include "cl_number_io.h"
#include "cl_complex_class.h"
#include "cl_real_class.h"
#include "cl_rational_class.h"
// Integers.
#include "cl_integer_class.h"
#include "cl_integer.h"
#include "cl_integer_io.h"
// Rational numbers.
#include "cl_rational.h"
#include "cl_rational_io.h"
// Also need an array container:
#include <cl_SV_integer.h>


///////////////////////////////////////////////////////////////
/// Algorithm simply copied from my GreatGuess.java class
/// Returns a string reporting details
void Minehunter::calc_prob(string& out_report)
{
    _show_args0();
    unsigned long mm,m;  // measured mines mm loop and location index m loop
    cl_I temp, temp2;
    cl_SV_I unmeasured_comb(max_mm_-min_mm_+1); // min_mm_ to max_mm_ inclusive
    cl_SV_I m_total_ways(m_max_+1);  // 0...(m_max_) inclusive, numerator
    cl_I big_total_comb(0);          // denominator
    cl_I total_unmeasured(0);        // unmeasured numerator
    cl_I big_total_unmeasured(0);    // unmeasured denominator
    cl_RA ratio;                     // Exact fraction
    pair_prob_loc_t p_l;             // double prob. and guint32 location
    ostrstream report;     // Make reporting string with sstream...fun!

    // Fill m_total_ways
    if (0==num_unmeasured_)
    {
        for (mm=min_mm_; mm<=max_mm_; ++mm)
        {
            big_total_comb +=  mm_ways_[mm];
            for (m=0; m<=m_max_; ++m)
            {
                m_total_ways[m] += mm_m_ways_[mm][m];
            }
        }
    }
    else 
    {
        // Also calculations for unmeasured locations
        for (mm=min_mm_; mm<=max_mm_; ++mm)
        {
            unmeasured_comb[mm-min_mm_] = 
                binomial( num_unmeasured_ , total_mines_ - mm );
            temp = mm_ways_[mm] * unmeasured_comb[mm-min_mm_];
            big_total_comb += temp;
            for (m=0; m<=m_max_; ++m)
            {
                m_total_ways[m] += mm_m_ways_[mm][m] * unmeasured_comb[mm-min_mm_];
            }
            total_unmeasured += temp * ( total_mines_ - mm );
        }
    }

    guint mm_sum(0);
    report << "Dump of mm_ways_ and mm_m_ways:" << endl;
    for (mm=min_mm_; mm<=max_mm_; ++mm) 
    {
        report << mm << " : " << mm_ways_[mm] << " :";
        mm_sum+=mm_ways_[mm];
        for (m=0; m<=m_max_; ++m)
        {
            report << " " << mm_m_ways_[mm][m];
        }
        report << endl;
    }
    report << "A total of " << mm_sum << 
        " ways of arranging mines in  measured locations." << endl;

    report << "Guessing algorithm report: big numbers" << endl;
    report << "Total # of possible mine configurations:" << endl;
    report << big_total_comb << endl;
    report << "Same total as a double: " << cl_double_approx(big_total_comb) << endl;
    report << "index : ( x , y ) : probability of mine" << endl;

    _nullChk(prob_loc_);
    for (m=0; m<=m_max_; ++m)
    {
        ratio = m_total_ways[m] / big_total_comb;
        p_l.first = cl_double_approx( ratio );
        p_l.second = m_loc_[m];
        prob_loc_->insert( p_l );
        report << p_l.second << " : ( " << (p_l.second % game_board_.x_max())
               << " , " << (p_l.second/game_board_.x_max()) << " ) : "
               <<  setprecision(7) << p_l.first << endl;
    }
    // Also calculations for unmeasured locations
    if (0<num_unmeasured_)
    {
        big_total_unmeasured = big_total_comb * num_unmeasured_;
        ratio = total_unmeasured / big_total_unmeasured;
        p_l.first = cl_double_approx( ratio );
        // GOTCHA: Outside usual range means prob. for unmeasured
        p_l.second = game_board_.locations();
        prob_loc_->insert( p_l );
        report << "Probability of mine in unmeasured location: " 
               <<  setprecision(7) << p_l.first << endl;
    }
    report << ends;
    out_report=report.str();
}

#undef  WANT_OBFUSCATING_OPERATORS

/*
$Header: /home/ckuklewicz/cvsroot/gminehunter/Minehunter.cc,v 2.11 2000/08/20 00:23:42 ckuklewicz Exp $
$Log: Minehunter.cc,v $
Revision 2.11  2000/08/20 00:23:42  ckuklewicz
Numerous bugs,races,lockups, gui logic all fixed
It works great.  I love -O3

Revision 2.10  2000/08/11 04:56:03  ckuklewicz
Worked out a few user interface policy bugs.
Made G_SignalBuffer_EventSource more like a swiss army knife.
(Can look like an event source, a timeout, or an idler)
Need to make timeout_msecs==-1 special.

The seed=0 randomly selected seed is reported in the text box.

Revision 2.9  2000/08/10 04:32:51  ckuklewicz
Reworked the locking in SafeSignalBuffer, removing busy_calling_
(but leaving it defined for now)

Added G_SignalBuffer_EventSource, works great

This will be the tarball version

Revision 2.8  2000/08/07 03:22:33  ckuklewicz
Wow...tons and tons of bugs fixed.
Added the sendPing, received_ping functions.
The COMPUTER_GAME seems to actually work.

Revision 2.7  2000/08/02 22:00:30  ckuklewicz
Try and add code to hook up App/Win to Minehunter
Who knows if it will compile

Revision 2.6  2000/08/02 20:40:05  ckuklewicz
Changed Buffers to allow emitOneEvent.
Moved GreatGuess code fully into Minehunter.cc
and removed files (to old/)
Now compiles with optMakefile, links with libcln.a

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


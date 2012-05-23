#include "Game.hh"

void Game::reset(guint32 x_max, guint32 y_max, guint32 mines) 
{
    empty_.grab_set().clear();
    mined_.grab_set().clear();
    measured_.grab_set().clear();
    mined_.max_mines()=0;
    mined_.min_mines()=0;
    x_max_=x_max;
    y_max_=y_max;
    mines_=mines;
    remaining_=mines;
    locations_=x_max_*y_max_;
}

bool Game::is_marked(index_type loc)
{
    return (empty_.get_set().end()!=empty_.get_set().find(loc)) ||
        (mined_.get_set().end()!=mined_.get_set().find(loc));
}

bool Game::is_measured(index_type loc)
{
    return (measured_.get_set().end()!=measured_.get_set().find(loc));
}

bool Game::mark_empty(index_type loc)   
{
    measured_.remove_index(loc,false);
    return empty_.add_index(loc); 
}

bool Game::mark_mine(index_type loc)
{ 
    measured_.remove_index(loc,false);
    if (mined_.add_index(loc)) {
        ++mined_.max_mines();
        ++mined_.min_mines();
        --remaining_;
        return true;
    } else {
        return false;
    }
}

bool Game::mark_measured(const ScanSet& S)
{
    set_index_type::const_iterator index(S.get_set().begin());
    for(;index!=S.get_set().end();++index)
    {
        if (!is_marked(*index))
        {
            measured_.add_index(*index);
        }
    }
    return true;
}

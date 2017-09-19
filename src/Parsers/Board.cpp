// Owl - www.owlclient.com
// Copyright (c) 2012-2017, Adalid Claure <aclaure@gmail.com>

#include "Board.h"

namespace owl
{

StringMap &BoardObject::getOptions()
{
    return _options;
}

const StringMap &BoardObject::getOptions() const
{
    return _options;
}

}

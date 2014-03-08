/*
 * This file is a part of the open source stm32plus library.
 * Copyright (c) 2011,2012,2013 Andy Brown <www.andybrown.me.uk>
 * Please see website for licensing terms.
 */

#pragma once


class TimeDisplay {

  protected:
    uint32_t _time;

    void drawTime(LcdPanel& gl,const Point& p);

  public:
    TimeDisplay();
};

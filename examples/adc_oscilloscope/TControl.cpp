/*
 * This file is a part of the open source stm32plus library.
 * Copyright (c) 2011,2012,2013 Andy Brown <www.andybrown.me.uk>
 * Please see website for licensing terms.
 */

#include "stdafx.h"


/*
 * Constructor
 */

void TControl::draw(Display& display) {

  LcdPanel& gl(display.getGl());

  // erase the background

  Rectangle rc(CONTROL_X,
               LcdPanel::SHORT_SIDE-ControlsContainerControl::CONTROLS_HEIGHT+1,
               CONTROL_WIDTH,
               ControlsContainerControl::CONTROLS_HEIGHT-2);

  clearRectangle(gl,rc);

  // draw the time

  drawTime(gl,Point(rc.X+2,rc.Y+2));
}

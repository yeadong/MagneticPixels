//===-- GameTest.h -------------------------------*- C++ -*-===//
//
//  Created:     2013/09/20
//  Author:      Mihailenco E. at Emboss Games, 2013
//  Description: Same as main, but plays editor's layer
//
//===---------------------------------------------------------===//

#pragma once
#ifndef GAMETEST_H_
#define GAMETEST_H_

#include "EMBase.h"
#include "MPix.h"
#include "GameMain.h"

namespace MPix {

   // Forward dependencies
   class TouchLayer;
   class PixelsLayer;
   class Command;

   // Game scene for tests, differs by:
   // buttons
   // finished behavior
   // button handler
   class GameTest: public GameMain
   {
   public:

      EM_GAME_STATE(GameTest);

      bool init() override;

   protected:

      // Command to display results on game finish
      ErrorCode FinishedGame();

      // Different handler
      void BtnHnadler(Object* sender);

      // Different buttons
      void CreateButtons() override;

   };

}


#endif // GAMETEST_H_





//===-- GameMain.h -------------------------------*- C++ -*-===//
//
//  Created:     2013/09/02
//  Author:      Mihailenco E. at Emboss Games, 2013
//  Description: Main game state is active when playing
//
//===---------------------------------------------------------===//

#pragma once
#ifndef GAMEMAIN_H_
#define GAMEMAIN_H_

#include "EMBase.h"
#include "MPix.h"
#include "GameState.h"

namespace MPix {

   // Forward dependencies
   class TouchLayer;
   class PixelsLayer;
   class Command;

   // Game scene
   class GameMain: public GameState
   {
   public:

      EM_GAME_STATE(GameMain);
      GameMain();
      ~GameMain();
      bool init() override;

      void onEnter() override;
      void onExit() override;

      ErrorCode Tick( float t ) override;

   protected:

      // Layers, constructing the content
      PixelsLayer* pixels;
      TouchLayer* touch;
      Layer* bg;
      Node* notification;
      Node* notification_content;
       
      // Content holds pixels and touch layers, making easy to scroll/zoom
      Node* content;

   protected:

      // Method is called when game state initializes
      virtual void CreateButtons();

      // Method is called when player finished game
      virtual ErrorCode FinishedGame();

   protected:
      //Notification

      struct Notification {
         string message;
         bool boundled;
         Point pos;
      };

      list<Notification> notifications;

      // Called on notification request
      ErrorCode ShowNotification(const Notification& n);

      // Touch in notification
      void onNotificationClick();
      EventListener* notification_toucher;
      bool notification_active;

   protected:

      // Saved on init screen metrics
      Size m_full_size, m_half_size, m_visible_size;
      float m_content_scale;
      Point m_center, m_origin;

   protected:

      void BtnHnadler(Object* sender);

      int logic_tick = 0;
      int views_tick = 0;

   private: // Commands

      // Handles CmdGameFinished
      ErrorCode onCmdGameFinished();

      // Handles CmdShowNotification
      ErrorCode onCmdShowNotification(const string& message, bool boundled, Point pos);

   };

}

#endif // GAMEMAIN_H_

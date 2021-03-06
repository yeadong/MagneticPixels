//===-- TouchLayer.h ------------------------------------*- C++ -*-===//
//
//  Created:     2013/09/01
//  Author:      Mihailenco E. at TheEndlessCat Games, 2013
//  Description: TouchLayer works with touches, displays grid
//
//===---------------------------------------------------------===//

#pragma once
#ifndef TOUCHLAYER_H__
#define TOUCHLAYER_H__

#include "ECBase.h"
#include "MPix.h"

namespace MPix {

   // Forward dependencies
   //

   // TouchLayer class
   class TouchLayer : public Layer
   {
   public:

      bool init() override;
      CREATE_FUNC(TouchLayer);

   public:

      enum FieldState {
         WAITING_TOUCH = 0,
         ONE_TOUCH_RECORDING,
         ZOOMING_AND_PANING,
         IGNORING,
      };

      TouchLayer();
      ~TouchLayer();

      void onEnter() override;

      // -------- Delegates -------------------------------------------------

      bool     onTouchBegan( Touch *touch, Event *event);
      void onTouchCancelled( Touch *touch, Event *event);
      void     onTouchEnded( Touch *touch, Event *event);
      void     onTouchMoved( Touch *touch, Event *event);

      void onBackClicked();

      // ---------- listners ----------------------------------------------
       ErrorCode onBGFG();

   private:

      FieldState st;

      EventListener* toucher;

      // -------- Gesture with one touch ------------------------------------------

      // Here is gesture stored (SCREEN coordinates)
      vector<Vector2> sequence;

      void AnalyseSequence();

      Vector2 ps, pe;
      int n_acute_angles;

      double timestamp;

      int idling_counter;

      // --------- Zooming and panning ----------------------------------- // TODO
      // p1, p2, ps1, ps2 ... etc

   private: // Recognized gestures

      // Called when tap received
      void GestureTapPoint(Vector2 pos);
      void GestureSwipe(Direction dir);
      void GestureLongSwipe(Direction dir);
      void GestureShake();
      void GestureRotateCW();
      void GestureRotateCCW();


   private: // Touch on/off commands use semaphore technique

      // Helpers, always do what said
      ErrorCode TouchEnable();
      ErrorCode TouchDisable();

      // Clear touch queue and set to waiting
      void ResetState();

   };

}

#endif // TOUCHLAYER_H__



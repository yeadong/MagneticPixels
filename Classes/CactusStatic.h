//===-- NeedlePixel.h -------------------------------*- C++ -*-===//
//
//  Created:     2013/09/09
//  Author:      Mihailenco E. at TheEndlessCat Games, 2013
//  Description:
//
//===---------------------------------------------------------===//

#pragma once
#ifndef STATIC_CACTUS_H_
#define STATIC_CACTUS_H_

#include "ECBase.h"
#include "ECNode.h"

#include "IKilling.h"
#include "IAlive.h"
#include "IMoveBlocker.h"

namespace MPix {

   // Forward dependencies
   class Context;

   // MagneticPixel
   class CactusStatic :
      public IKilling,         // Can kill
      public IAlive,           // Can be killed(by pitfalls for example)
      public IMoveBlocker      // Blocks cactus
   {
   public:

      // Register this pixel type to pixel factory
      ECNODE_CHILD(CactusStatic);

   public:

      CactusStatic();

      // ----- Implements interfaces  -----------------------------------------

      // Pixel
      PixelType GetType() const override { return Pixel::PixelType::CACTUS_PIX; }

      virtual void InitSnapshots( const Context& context ) override;
      virtual void PushSnapshot( const Context& context ) override;
      virtual void ClearSnapshots( const Context& context ) override;
      virtual void PopSnapshots( const Context& context, int n ) override;

   };

}


#endif // STATIC_CACTUS_H_
//===-- WallPixelView.h -------------------------------*- C++ -*-===//
//
//  Created:     2013/09/06
//  Author:      Mihailenco E. at Emboss Games, 2013
//  Description: Wall pixel view
//
//===---------------------------------------------------------===//

#pragma once
#ifndef WALLVIEW_H_
#define WALLVIEW_H_

#include "EMBase.h"
#include "PixelView.h"

namespace MPix {

   // Forward dependencies
   class WallPixel;


   // WallPixelView

   class WallPixelView : public PixelView
   {
   public:

      EM_NODE_CHILD(WallPixelView);
      WallPixelView();
      ~WallPixelView();

      void Build( shared_ptr<Pixel> model ) override;

      bool Update( CmdUIUpdatePixelView::Reason reason ) override;
      void BindContents( Node* target, int recommendedOrder = 0 ) override;

      // View interface
      void setVisible( bool visibility ) override;
      void setPosition( Point pos ) override;

   protected:

   private:
      shared_ptr<WallPixel> pixel;
      vector<Node*>  blocks_urdl;

   };

}


#endif // WALLVIEW_H_
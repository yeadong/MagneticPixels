#include "ColorBox.h"
#include "MPix.h"

using namespace MPix;

void ColorBox::draw(Renderer *renderer, const Matrix &transform, bool transformUpdated)
{
    _customCommand.init(_globalZOrder);
    _customCommand.func = CC_CALLBACK_0(ColorBox::onDraw, this, transform, transformUpdated);
    renderer->addCommand(&_customCommand);
}

ColorBox* ColorBox::create()
{
   ColorBox *pRet = new ColorBox();
   pRet->autorelease();
   pRet->col = Color4F(1.0f, 0.0f, 1.0f, 1.0f);
   pRet->borders_col = Color4F(0.0f, 1.0f, 1.0f, 1.0f);
   pRet->borders = 0;
   pRet->width = 5.0f;
   return pRet;
}

void ColorBox::SetColor( Color4F col )
{
   this->col = col;
}

void ColorBox::SetBorders( unsigned borders, Color4F col )
{
   this->borders = borders;
   borders_col = col;
}

void ColorBox::onDraw(const Matrix &transform, bool )
{

   Director* director = Director::getInstance();
   CCASSERT(nullptr != director, "Director is null when seting matrix stack");
   director->pushMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);
   director->loadMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW, transform);

   CHECK_GL_ERROR_DEBUG();

   // Draw
   auto col = this->col;
   col.a *= (this->_displayedOpacity / 255.0f);

   auto ap = getAnchorPoint();
   Vector2 p00 = ap * MPIX_CELL_SIZE * -1;
   Vector2 p11 = p00 + MPIX_CELL_SIZE_P;
   Vector2 p10 = p00; p10.x += MPIX_CELL_SIZE;
   Vector2 p01 = p00; p01.y += MPIX_CELL_SIZE;

   DrawPrimitives::drawSolidRect(p00, p11, col);
   auto p = Vector2(width,width);
   if (borders) {
      if ( borders & BORDER_LEFT )
         DrawPrimitives::drawSolidRect(p00-p, p01+p, borders_col);
      if ( borders & BORDER_RIGHT )
          DrawPrimitives::drawSolidRect(p10-p, p11+p, borders_col);
      if ( borders & BORDER_UP )
          DrawPrimitives::drawSolidRect(p01-p, p11+p, borders_col);
      if ( borders & BORDER_DOWN )
          DrawPrimitives::drawSolidRect(p00-p, p10+p, borders_col);
   }

   CHECK_GL_ERROR_DEBUG();

   //end draw
   director->popMatrix(MATRIX_STACK_TYPE::MATRIX_STACK_MODELVIEW);

}

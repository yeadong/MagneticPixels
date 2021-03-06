#include "SimpleInputDialog.h"
#include "ContentManager.h"

using namespace MPix;

//====---------------------------------------------======//

SimpleInputDialog* MPix::SimpleInputDialog::create( const char* req /*= "Input, please"*/, const char* def /*= ""*/ )
{
   auto fab = new SimpleInputDialog();
   if (fab->initWithRequast(req, def)) {
      fab->autorelease();
      return fab;
   }
   delete fab;
   return nullptr;
}

bool MPix::SimpleInputDialog::initWithRequast( const char* req /*= "Input, please"*/, const char* def /*= ""*/ )
{

   auto sx = Director::getInstance()->getWinSize();
   Vector2 center = Vector2(sx.width/2, sx.height/2);

   auto bg = LayerColor::create(Color4B(0,0,0,255));
   addChild(bg);

   auto pLabel = Label::createWithTTF(req, ContentManager::getInstance().GetEditorFont(), 36);
   pLabel->setPosition(center + Vector2(0,center.y*0.5f));
   addChild(pLabel);

   field = TextFieldTTF::textFieldWithPlaceHolder(
      def,    // Text
      ContentManager::getInstance().GetEditorFont(), // Font
      36       // Size
   );
   field->setPosition(center + Vector2(0,center.y*0.25f));
   addChild(field);

   MenuItemFont::setFontSize(36);
   auto m = Menu::create();
   auto yes = MenuItemFont::create("OK", [&](Ref *sender) {
      OnOK();
   });
   yes->setTag(101);
   auto no = MenuItemFont::create("CANCEL", [&](Ref *sender) {
      OnCancel();
   });
   no->setTag(102);
   m->addChild(yes);
   m->addChild(no);
   m->alignItemsHorizontallyWithPadding(10.0f);
   addChild(m);


   field->attachWithIME();

   return true;

}

void MPix::SimpleInputDialog::OnOK()
{
   string inp = field->getString();
   if (fun)
      fun(inp);
   field->detachWithIME();
   Terminate();
}

void MPix::SimpleInputDialog::OnCancel()
{
   field->detachWithIME();
   Terminate();
}

void MPix::SimpleInputDialog::Terminate()
{
   this->removeFromParent();
}

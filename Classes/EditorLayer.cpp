#include "EditorLayer.h"
#include "GameStateManager.h"
#include "ContentManager.h"
#include "LevelManager.h"
#include "Cmd.h"
#include "ColorBox.h"
#include "RoundMark.h"
#include "EditorToolbox.h"

#include "ViewManager.h"
#include "PixelView.h"
#include "GoalView.h"
#include "Pixel.h"
#include "Goal.h"
#include "Level.h"
#include "LevelGenerator.h"
#include "EditorData.h"



using namespace MPix;

// constants
const float TAP_THRESHOLD = 10.0f;


MPix::EditorLayer::EditorLayer()
{
    CmdDidEnterBG::listners["EditorLayer"] = std::bind( &EditorLayer::onBGFG, this );
    CmdWillEnterFG::listners["EditorLayer"] = std::bind( &EditorLayer::onBGFG, this );
    ContentManager::getInstance().SetupPixelNodes(this);

    auto lvl = LevelManager::getInstance().EditorGetCurrentLevel();
    if (lvl) {
       editor_data = LevelGenerator::getInstance()->SaveToEditorData(lvl);
       assert(editor_data);
    } else 
       editor_data = new EditorData();
}

MPix::EditorLayer::~EditorLayer()
{
    delete editor_data;
    views.clear();
    walls.clear();
    tasks.clear();
    CmdDidEnterBG::listners.erase("EditorLayer");
    CmdWillEnterFG::listners.erase("EditorLayer");
    ContentManager::getInstance().UnsetupPixelNodes(this);
}

bool EditorLayer::init()
{
   cursor = Coordinates(0, 0);
   st = TouchState::WAITING_TOUCH; 

   auto box = ColorBox::create();
   box->SetColor(Color4F::WHITE);
   box->SetBorders(15, Color4F::RED);
   box->setPosition(LogicToScreen(cursor));
   box->runAction(
      RepeatForever::create(
         Sequence::create(
            FadeIn::create(0.4f), FadeOut::create(0.4f), nullptr
         )
      )
   );
   addChild(box, -10);
   this->cursor_node = box;

   auto dn = DrawNode::create();
   // Draw a grid and gray pixel
   auto cl = Color4F(0.5f, 0.5f, 0.5f, 1.0f);
   const int size = 90;
   for (int i=0; i < size*2+1; ++i){
      dn->drawSegment( 
         LogicToScreen(Coordinates(-size, i-size)),
         LogicToScreen(Coordinates(size, i-size)),
         0.5f,
         cl);
      dn->drawSegment( 
         LogicToScreen(Coordinates(i-size, -size)),
         LogicToScreen(Coordinates(i-size, size)),
         0.5f,
         cl);
   }
   addChild(dn);

   for (int i=0; i<4; ++i) {
      auto m = RoundMark::create("X");
      m->setScale(3.0f );
      addChild(m, 100500);
      vp_marks[i] = m;
   }

   if (editor_data) {

      for (auto px : editor_data->GetAllPixels() ) {
         cursor = px->GetPos();
         InsertPixelView(px);
      }

      for (auto px : editor_data->GetAllWalls() ) {
         cursor = px->GetPos();
         InsertWallView(px);
      }

      for (auto px : editor_data->GetAllGoals() ) {
         cursor = px.first;
         InsertGoalView(px.second);
      }
   }

   cursor = Coordinates(0, 0);
   

   return true;
}

ErrorCode EditorLayer::onBGFG() {
    st = TouchState::WAITING_TOUCH;
    return ErrorCode::RET_OK;
}

bool EditorLayer::onTouchBegan( Touch *touch, Event *event )
{
   switch (st)
   {
   case TouchState::WAITING_TOUCH:
      pos = this->getPosition();
      first_touch = touch;
      st = TouchState::ONE_TOUCH;
      dragging = false;
      return true;
   case TouchState::ONE_TOUCH:
      second_touch = touch;
      pos = this->getPosition();
      scale = this->getScale();
      st = TouchState::ZOOMING;
      return true;
   case TouchState::ZOOMING:
      return false;
   default:
      return false;
   }

   return false;

}

void EditorLayer::onTouchMoved( Touch *touch, Event *event )
{
   switch (st)
   {
   case TouchState::WAITING_TOUCH:
      break;
   case TouchState::ONE_TOUCH: {
      if (!dragging) {
         if ( (touch->getStartLocationInView() - touch->getLocationInView()).getLength() > TAP_THRESHOLD ) {
            dragging = true;
         }
         else return;
      }
      auto was = getParent()->convertToNodeSpace(touch->getStartLocation());
      auto dir = getParent()->convertToNodeSpace(touch->getLocation()) - was;
      this->setPosition(pos + dir);
      UpdateViewport();
      break;
   }
   case TouchState::ZOOMING:
      // TODO: make zoom, now DISKOTEKA!
      //this->setVisible(!this->isVisible());
      break;
   case TouchState::IGNORING:
      break;
   default:
      break;
   }

}

void EditorLayer::onTouchCancelled( Touch *touch, Event *event )
{
   // React same as End
   onTouchEnded(touch, event);
}

void EditorLayer::onTouchEnded( Touch *touch, Event *event )
{
   //EM_LOG_WARNING("Ended");
   auto pScrEnd = touch->getLocationInView();
   auto pScrStart = touch->getStartLocationInView();

   switch (st)
   {
   case TouchState::WAITING_TOUCH:
      break;
   case TouchState::ONE_TOUCH:
      if ( (pScrEnd-pScrStart).getLength() < TAP_THRESHOLD )
         GestureTapPoint( ScreenToLogic(this->convertTouchToNodeSpace(touch)) );
      first_touch = nullptr;
      st = TouchState::WAITING_TOUCH;
      break;
   case TouchState::ZOOMING:
      // here are two touches,  but user released only one
      st = TouchState::IGNORING;
      break;
   case TouchState::IGNORING:
      // Well noooow we can switch to waiting
      st = TouchState::WAITING_TOUCH;
      break;
   default:
      break;
   }


}


void MPix::EditorLayer::GestureTapPoint( Coordinates p )
{
   EM_LOG_DEBUG(" P: " + p);
   if (p != cursor) {
      MoveCursor(p);
      return;
   }

   GameStateManager::getInstance().CurrentState()->Execute(new CmdEditorAction(CmdEditorAction::EditorAction::ED_SHOW_TOOLS));
}

void MPix::EditorLayer::MoveCursor( Coordinates pos )
{
   cursor = pos;
   auto act = MoveTo::create(0.2f, LogicToScreen(cursor));
   act->setTag(42);
   cursor_node->stopActionByTag(42);
   cursor_node->runAction(act);
}

void MPix::EditorLayer::InsertPixelView( shared_ptr<Pixel> px )
{
   auto view = PixelView::create(px);
   view->Build(px);
   view->BindContents(this, 1);
   //view->setPosition( LogicToScreen(cursor));
   view->Update(CmdUIUpdatePixelView::Reason::CREATED);

   auto it = views.find(cursor);
   if (it == views.end()) {
      vector<shared_ptr<PixelView>> v;
      v.reserve(5);
      v.push_back(view);
      views.emplace(cursor, v);
   } else {
      // Hide underlay
      it->second.back()->setVisible(false);
      it->second.push_back(view);

      // Show digit with count
      PutMark(cursor, editor_data->NumberOfPixelsAt(cursor));
   }

}

void MPix::EditorLayer::InsertWallView( shared_ptr<Pixel> px )
{
   auto view = PixelView::create(px);
   view->Build(px);
   view->BindContents(this, 1);
   //view->setPosition( LogicToScreen(cursor));
   view->Update(CmdUIUpdatePixelView::Reason::CREATED);
   walls[cursor] = view;
}

void MPix::EditorLayer::InsertGoalView( PixelColor color )
{
   // Bad thing
   // TODO: Implement class TaskView!!!
   // for now - create goal view with single task and put to screen
   auto g = make_shared<Goal>();
   g->AddTask(cursor, color);
   auto gv = make_shared<GoalView>();
   gv->Build(g);
   gv->BindContents(this, 0);
   gv->Update(CmdUIUpdateGoalView::Reason::CREATED);
   tasks.emplace(cursor, gv);
}

//////////////////////////////////////////////////////////////////////////

void MPix::EditorLayer::InsertWall( shared_ptr<Pixel> px )
{
   editor_data->InsertWallAt(cursor, px);
   InsertWallView(px);
}

void MPix::EditorLayer::InsertPixel( shared_ptr<Pixel> px )
{
   editor_data->InsertPixelAt(cursor, px);
   InsertPixelView(px);
}

void MPix::EditorLayer::InsertGoal( int n, PixelColor color )
{
   if (editor_data->InsertTask(n, cursor, color)) {
      InsertGoalView(color);
   } else {
      // Override
      EraseGoal();
      InsertGoal(n, color);
   }
}

void MPix::EditorLayer::ErasePixels()
{
   if (editor_data->RemoveAllPixelsAt(cursor)) {
      views.erase(cursor);
      PutMark(cursor, 0);
   }
}

void MPix::EditorLayer::EraseWall()
{
   if (editor_data->RemoveWallAt(cursor)) {
      walls.erase(cursor);
   }

}

void MPix::EditorLayer::EraseTopPixel()
{
   if ( editor_data->RemoveTopPixelAt(cursor) ) {
      auto it = views.find(cursor);
      assert(it != views.end());
      auto & lst = it->second;
      lst.pop_back();
      if (lst.empty()) { // IF removed last one
         views.erase(it);
      } else {
         lst.back()->setVisible(true);
      }
      PutMark(cursor, editor_data->NumberOfPixelsAt(cursor));
   }
}

void MPix::EditorLayer::EraseGoal()
{
   if ( editor_data->RemoveTask(cursor) ) {
      tasks.erase(cursor);
   }
}

void MPix::EditorLayer::EraseEverything()
{
   EraseGoal();
   ErasePixels();
   EraseWall();
}

void MPix::EditorLayer::PutMark( Coordinates pos, int value )
{
   auto it = marks.find(pos);
   if (value >= 2) { // Creation
      if (it != marks.end()) {
         dynamic_cast<LabelTTF*>(it->second)->setString(ToString(value).c_str());
      } else {
         auto m = RoundMark::create(ToString(value).c_str());
         //m->setOpacity(200);
         m->setPosition(LogicToScreen(pos) + MPIX_CELL_SIZE_P * 0.7f );
         marks.emplace(pos, m);
         this->addChild(m, 20);
      }
   } else { // Removing
      if (it != marks.end()) {
         this->removeChild(it->second);
         marks.erase(it);
      }
   }
}

shared_ptr<Level> MPix::EditorLayer::CreateLevel()
{
   assert(editor_data);
   if (editor_data->CheckConsistency()) {
      auto lvl = LevelManager::getInstance().EditorGetCurrentLevel();
      return LevelGenerator::getInstance()->CreateFromEditorData(editor_data, lvl);
   } else {
      return nullptr;
   }
}

void MPix::EditorLayer::UpdateViewport()
{
   auto diff = getPosition() - initial_pos;
   auto p = ScreenToLogic(diff);
   Rectangle r;
   r.BL = Coordinates(-4-p.x,-6-p.y);
   r.TR = Coordinates(3-p.x,5-p.y);
   this->editor_data->SetViewport(r);
   vp_marks[0]->setPosition(LogicToScreen(r.BL.x,r.BL.y) + MPIX_CELL_SIZE_HALF_P);
   vp_marks[1]->setPosition(LogicToScreen(r.TR.x,r.BL.y) + MPIX_CELL_SIZE_HALF_P);
   vp_marks[2]->setPosition(LogicToScreen(r.BL.x,r.TR.y) + MPIX_CELL_SIZE_HALF_P);
   vp_marks[3]->setPosition(LogicToScreen(r.TR.x,r.TR.y) + MPIX_CELL_SIZE_HALF_P);

}

void MPix::EditorLayer::FixPosition()
{
   auto vm = editor_data->GetViewport();
   Coordinates p = vm.BL + Coordinates(4,6); // pos defined with BL only
   auto sc = this->getScale();
   auto pos = this->getPosition() - LogicToScreen(p);
   initial_pos = this->getPosition();
   this->setPosition(pos);
   UpdateViewport();
}


void MPix::EditorLayer::RenameLevel( string newname )
{
   assert(editor_data);
   editor_data->SetName(newname);
}

bool MPix::EditorLayer::TogglePan()
{
   auto n = ! editor_data->GetAutoPan();
   editor_data->SetAutoPan(n);
   return n;
}

const string& MPix::EditorLayer::GetLevelName()
{
   assert(editor_data);
   return editor_data->GetName();
}





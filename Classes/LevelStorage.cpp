#include "LevelStorage.h"
#include "World.h"
#include "Level.h"
#include "LevelXML.h"
#include "PlatformManager.h"


using namespace MPix;
using namespace tinyxml2;

//====---------------------------------------------======//

void MPix::LevelStorage::SetMap( const string & levelMap )
{
   assert(levelMap != "");
   this->levelMap = levelMap;
}

MPix::LevelStorage::LevelStorage()
{
   levelMap = "levels.xml";
   exportDir = PlatformManager::getInstance().GetSavingPath() + "/";
}

MPix::LevelStorage::~LevelStorage()
{
}

EndlessCatLib::ErrorCode MPix::LevelStorage::GetLevels( list<shared_ptr<World>> &worlds, unordered_map<unsigned int, shared_ptr<Level>> &levels )
{

  // ssize_t size = 0;
   //char* pLevelData = nullptr;

   // Logic of loading:
   // Release version:
   //    reads /res/levelMap file to pLevelData
   // Debug version:
   //    if /writable/levelMap exists
   //       loads it to pLevelData
   //    if /writable/levelMap not exists
   //       reads /res/levelMap to pLevelData
   //       copies to /writable/levelMap for next time

   Data data;
   tinyxml2::XMLDocument doc;

#ifdef MPIX_DEVELOPERS_BUILD

   // Try to find writible levelMap
   auto writeLevelMap = FileUtils::getInstance()->getWritablePath() + levelMap;

   if ( FileUtils::getInstance()->isFileExist(writeLevelMap) ) {

      data = CCFileUtils::getInstance()->getDataFromFile(writeLevelMap);

      if (data.isNull()) {
         ECLOG_ERROR("FATAL : Writable " + levelMap + " exists at\n" + writeLevelMap + "\nbut is empty or inaccessible" );
         return ErrorCode::RET_FAIL;
      }
   }
   else  // carefully, code continues outside of #endif intentionally, logic described above
#endif
   {
      // Load file levelMap from /res
      string resLevelsFile = CCFileUtils::getInstance()->fullPathForFilename(levelMap);

      data = CCFileUtils::getInstance()->getDataFromFile(resLevelsFile);

      if (data.isNull()) {
         ECLOG_ERROR("FATAL : Can't load " + levelMap);
         return ErrorCode::RET_FAIL;
      }
   }

   doc.Parse((char*)data.getBytes(), data.getSize()); // TODO: remove reallocation

   if (doc.Error()) {
      ECLOG_ERROR("FATAL : Can't parse "+levelMap);
      ECLOG_ERROR("TIXML reports : \n " + doc.GetErrorStr1() + " \n"  + doc.GetErrorStr2() );
      return ErrorCode::RET_FAIL;
   }


#ifdef MPIX_DEVELOPERS_BUILD
   doc.SaveFile(writeLevelMap.c_str(), false);
#endif



   //Check if root is present
   auto root = doc.FirstChildElement("mpix");
   if ( !root )
   {
      ECLOG_ERROR("Can't find root element with name `mpix` in file " + levelMap);
      return ErrorCode::RET_FAIL;
   }

   // Default editor world id=0
   worlds.push_back(make_shared<World>(0, "Editor"));

   // Parse worlds section
   auto wsection = root->FirstChildElement("worlds"); //Check
   if ( !wsection )
   {
      ECLOG_ERROR("Can't find element with name `worlds` in file " + levelMap);
      return ErrorCode::RET_FAIL;
   }

   int world_counter = 0;
   auto world = wsection->FirstChildElement("world");
   while (world)  {

      auto name = world->Attribute("name");
      if (name == nullptr ) {
         ECLOG_WARNING("World without `name` attribute, skipping");
         world = world->NextSiblingElement("world");
         continue;
      }

      world_counter++;
      int id = world_counter;
      worlds.push_back(make_shared<World>(id, name));

      world = world->NextSiblingElement("world");
   }

   if (world_counter == 0 ) {
      ECLOG_WARNING("No worlds in file, only editor(=0) world will be available " + levelMap);
   }

   unordered_map<int, shared_ptr<World>> worlds_map;
   for (auto w : worlds) {
      worlds_map.emplace(w->GetID(), w);
   }

   // Parse levels section
   auto lsection = root->FirstChildElement("levels");
   if ( !lsection )
   {
      ECLOG_ERROR("Can't find element with name `levels` in file " + levelMap);
      return ErrorCode::RET_FAIL;
   }

   int lvl_counter = 0;
   int err_counter = 0;
   auto lvl = lsection->FirstChildElement("lvl");
   while (lvl) {

      auto l = LevelXML::Generate(lvl);

      // Not loaded?
      if (l == nullptr) {
         ECLOG_WARNING("Found invalid level, skipping");
         err_counter++;
         lvl = lvl->NextSiblingElement("lvl");
         continue;
      }

      // has no world?
      auto wit = worlds_map.find(l->GetWorld());
      if (wit == worlds_map.end()) {
         ECLOG_WARNING("Found worldless level, skipping");
         err_counter++;
         lvl = lvl->NextSiblingElement("lvl");
         continue;
      }

      // Dublicate!?
      auto id = l->GetID();
      auto lid = levels.find(id);
      if (lid != levels.end()) {
         ECLOG_WARNING("Found dublicate id level, skipping");
         err_counter++;
         lvl = lvl->NextSiblingElement("lvl");
         continue;
      }

      lvl_counter++;
      wit->second->AddLevel(id);
      levels.emplace(id, l);

      lvl = lvl->NextSiblingElement("lvl");
   }


   ECLOG_INFO("LevelStorage : Loaded " + world_counter + " worlds, "+lvl_counter + " levels ");
   if (err_counter) {
      ECLOG_WARNING("LevelStorage : Got " + err_counter + " errors in levels ");
   }

   return ErrorCode::RET_OK;
}


void MPix::LevelStorage::SaveLevel( shared_ptr<Level> level )
{
#ifdef MPIX_DEVELOPERS_BUILD

   char* pLevelData = nullptr;

   auto wpath = FileUtils::getInstance()->getWritablePath();
   string wname = wpath + levelMap;

   // Reading data to pLevelData

   // Open file for read
   auto wfile = fopen(wname.c_str(), "rb");
   if ( wfile == nullptr ) {
      ECLOG_ERROR(" Saving level failed, " + levelMap + " not found, or can't be read");
      return;
   }

   // Get size
   fseek(wfile,0,SEEK_END);
   unsigned long size = ftell(wfile);
   fseek(wfile,0,SEEK_SET);
   if (size == 0) {
      ECLOG_ERROR(" Saving level failed, " + levelMap + " size is zero, saving needs a boilerplate file");
      fclose(wfile);
      return;
   }

   // Reading
   pLevelData = new char[size];
   auto read = fread(pLevelData, sizeof(char), size, wfile);
   if (size != read) {
      ECLOG_ERROR(" Saving level failed, " + levelMap + " read failed, saving needs a boilerplate file");
      delete[] pLevelData;
      fclose(wfile);
      return;
   }

   // Close
   fclose(wfile);

   // Generating new level
   XMLPrinter dst;
   LevelXML::Store(level, &dst);
   auto ldata = dst.CStr();
   auto dstsz = dst.CStrSize();
   if (ldata == nullptr || dstsz <= 1) {
      ECLOG_ERROR(" Saving level failed, " + levelMap + " xml printer returned Zero string");
      delete[] pLevelData;
      return;

   }
   auto ldata_len = dstsz - 1; // Excluding NULL

   // Searching where to inject
   auto pos = strstr(pLevelData, "</levels>");
   if (pos == nullptr) {
      ECLOG_ERROR(" Saving level failed, " + levelMap + " no tag </levels> found, saving needs a boilerplate");
      delete[] pLevelData;
      return;
   }

   // Prefix size and suffix size
   // <levels>a</levels>
   //          |
   // pppppppppsssssssss
   //   9        9
   auto prefix_size = pos - pLevelData;
   auto suffix_size = size - prefix_size;

   auto newsz = size + ldata_len;
   auto pNewLevelData = new char[newsz];

   // Copy
   strncpy(pNewLevelData, pLevelData, prefix_size);
   strncpy(pNewLevelData+prefix_size, ldata, ldata_len);
   strncpy(pNewLevelData+prefix_size+ldata_len, pos, suffix_size);

   delete[] pLevelData;

   tinyxml2::XMLDocument doc;
   doc.Parse(pNewLevelData, newsz);
   delete[] pNewLevelData;

   // Check
   if (doc.Error()) {
      ECLOG_ERROR("FATAL : Can't parse "+levelMap);
      ECLOG_ERROR("TIXML reports : \n " + doc.GetErrorStr1() + " \n"  + doc.GetErrorStr2() );
      return;
   }

   // Writing back
   auto ret = doc.SaveFile(wname.c_str());
   if ( ret != tinyxml2::XML_NO_ERROR ) {
      ECLOG_ERROR(" Saving level failed, " + levelMap + " can't be written");
      return;
   }


   ECLOG_INFO(" Saved level " + level->GetName() + "(id=" + level->GetID() + ") to " + levelMap);

#else
   ECLOG_WARNING("Level deletion is avaliable only for dev. build")
#endif

}

void MPix::LevelStorage::DeleteLevel( unsigned levelID )
{
#ifdef MPIX_DEVELOPERS_BUILD

   auto wpath = FileUtils::getInstance()->getWritablePath();
   tinyxml2::XMLDocument doc;
   string wname = wpath + levelMap;
   doc.LoadFile(wname.c_str());

   //Check if root is present
   auto root = doc.FirstChildElement("mpix");
   if ( !root )
   {
      ECLOG_ERROR("Delete level failed : can't find root element with name `mpix` in file " + levelMap);
      return;
   }

   // Parse worlds section
   auto lsection = root->FirstChildElement("levels");
   if ( !lsection )
   {
      ECLOG_ERROR("Can't find element with name `levels` in file " + levelMap);
      return;
   }

   // Searching for level to delete by id
   bool deleted = false;
   auto lvl = lsection->FirstChildElement("lvl");
   while (lvl)
   {
      // Read ID
      unsigned int id = 0;
      if ((lvl->QueryUnsignedAttribute("id", &id) != XML_NO_ERROR)){
         ECLOG_WARNING("Some level has no `id` attribute in " + levelMap + ", level deletion routine skips it");
         lvl = lvl->NextSiblingElement("lvl");
         continue;
      }

      // Check match
      if (id == levelID) {
         lvl->DeleteChildren();
         lsection->DeleteChild(lvl);
         deleted = true;
         break;
      }

      lvl = lvl->NextSiblingElement("lvl");
   }

   if (deleted) {
      doc.SaveFile(wname.c_str());
      ECLOG_INFO(" Level id=" + levelID + " deleted from " + levelMap);
   }


#else
   ECLOG_WARNING("Level deletion is avaliable only for dev. build")
#endif

}

EndlessCatLib::ErrorCode MPix::LevelStorage::ExportMap()
{

   auto wpath = FileUtils::getInstance()->getWritablePath();
   string wname = wpath + levelMap;

   auto tname = exportDir + levelMap;
   tinyxml2::XMLDocument doc;
   doc.LoadFile(wname.c_str());
   if (doc.Error()) {
      ECLOG_ERROR("FATAL: Current levelmap is unparseble");
      PlatformManager::getInstance().ShowMessage("LevelStorage", "Level map is broken!");
      return ErrorCode::RET_FAIL;
   }
   doc.SaveFile(tname.c_str());
   if (doc.Error()) {
      ECLOG_ERROR("FATAL: Can't save levelmap to"+tname);
      string msg = "Can't save to\n " + tname + "\nPossibly write error";
      PlatformManager::getInstance().ShowMessage("LevelStorage", msg.c_str());
      return ErrorCode::RET_FAIL;
   }
   tname = "Levels were saved to\n " + tname + "\nNow send this to Randy, quickly!";
   PlatformManager::getInstance().ShowMessage("LevelStorage", tname.c_str());

   return ErrorCode::RET_OK;

}

const char* MPix::LevelStorage::GetExportDir()
{
   return exportDir.c_str();
}

#include <algorithm>
#include <cstdint>
#include <iostream>
#include <map>
#include <set>
using uint32=uint32_t;using int32=int32_t;
int checks=0;
#define CHECK(x) do {++checks;if(!(x)){std::cerr<<"failed line "<<__LINE__<<": "<<#x<<"\n";return 1;}}while(false)
constexpr int CLASS_HUNTER=3,CURRENT_AUTOREPEAT_SPELL=0;
constexpr int SPELL_CAST_OK=0, POWER_MANA=0,POWER_RAGE=1,POWER_ENERGY=3;using Powers=int;
struct SpellInfo{uint32 Id;int PowerType=POWER_MANA,cost=5,form=0;SpellInfo*next=nullptr;
 SpellInfo const*GetNextRankSpell()const{return next;}int CalcPowerCost(void*,int)const{return cost;}int GetSchoolMask()const{return 0;}int CheckShapeshift(int f)const{return f==form?0:1;}};
struct Unit{bool aura=false;int GetPositionX(){return 1;}int GetPositionY(){return 2;}int GetPositionZ(){return 3;}bool GetAuraOfRankedSpell(uint32,uint32=0){return aura;}};
struct GCD{bool blocked=false;bool HasGlobalCooldown(SpellInfo const*){return blocked;}};
struct PetAI{int attacks=0;void AttackStart(Unit*){++attacks;}};struct Pet{PetAI ai;bool alive=true;Unit*victim=nullptr;bool IsAlive(){return alive;}Unit*GetVictim(){return victim;}PetAI*AI(){return &ai;}};
struct Player:Unit{std::set<uint32> learned,active;GCD gcd;bool cooldown=false;int form=0,power=10,result=0,calls=0;uint32 last=0;int playerClass=3;bool autorepeat=false;Pet*pet=nullptr;
 bool HasSpell(uint32 id){return learned.count(id);}bool HasActiveSpell(uint32 id){return learned.count(id)&&active.count(id);}bool HasSpellCooldown(uint32){return cooldown;}
 GCD&GetGlobalCooldownMgr(){return gcd;}int GetShapeshiftForm(){return form;}int GetPower(Powers){return power;}
 int getClass(){return playerClass;}bool GetCurrentSpell(int){return autorepeat;}Pet*GetPet(){return pet;}
 int CastSpell(Unit*,uint32 id,bool triggered){last=id;++calls;if(triggered)return 1;return result;}
 int CastSpell(int,int,int,uint32 id,bool triggered){return CastSpell(nullptr,id,triggered);}uint32 GetGUID(){return 1;}};
struct SpellMgr{std::map<uint32,SpellInfo> spells;SpellInfo const*GetSpellInfo(uint32 id){auto it=spells.find(id);return it==spells.end()?nullptr:&it->second;}}mgr;auto sSpellMgr=&mgr;
struct Ctx{Player*me;Unit*target;};
@BODY@
int main(){mgr.spells.emplace(768,SpellInfo{768});mgr.spells.emplace(18562,SpellInfo{18562});mgr.spells.emplace(1,SpellInfo{1});mgr.spells.emplace(2,SpellInfo{2});mgr.spells[1].next=&mgr.spells[2];Player p;Unit target;
 CHECK(!TryCast(&p,&target,768));CHECK(p.calls==0);p.learned.insert(768);CHECK(!TryCast(&p,&target,768));CHECK(!TryCast(&p,&target,18562));
 p.active.insert(768);CHECK(TryCast(&p,&target,768));CHECK(p.last==768);CHECK(!TryCast(&p,nullptr,768));CHECK(!TryCast(&p,&target,0));CHECK(!TryCast(&p,&target,999));
 p.learned.insert(1);p.learned.insert(2);p.active.insert(1);CHECK(MaxRank(&p,1)==1);CHECK(TryCastRank(&p,&target,1));CHECK(p.last==1);CHECK(TryCastAt(&p,&target,1));CHECK(p.last==1);
 p.active.insert(2);CHECK(MaxRank(&p,1)==2);CHECK(KeepAuraOn(&p,&target,1));CHECK(p.last==2);target.aura=true;CHECK(!KeepAuraOn(&p,&target,1));p.aura=true;CHECK(!KeepSelfBuff(&p,1));p.aura=false;CHECK(KeepSelfBuff(&p,1));
 p.cooldown=true;CHECK(!TryCast(&p,&target,768));p.cooldown=false;p.gcd.blocked=true;CHECK(!TryCast(&p,&target,768));p.gcd.blocked=false;p.form=1;CHECK(!TryCast(&p,&target,768));p.form=0;p.power=4;CHECK(!TryCast(&p,&target,768));p.power=10;p.result=1;CHECK(!TryCast(&p,&target,768));p.result=0;
 mgr.spells[768].PowerType=5;p.power=0;CHECK(TryCast(&p,&target,768)); // Runes/procs retain native validation.
 p.active.clear();CHECK(MaxRank(&p,1)==0);CHECK(!TryCastRank(&p,&target,1));CHECK(!TryCastAt(&p,&target,1));
 Player hunter;Pet pet;hunter.pet=&pet;Ctx ctx{&hunter,&target};EngageRanged(ctx);CHECK(hunter.calls==0);CHECK(pet.ai.attacks==1);
 hunter.learned.insert(75);EngageRanged(ctx);CHECK(hunter.calls==0);hunter.active.insert(75);EngageRanged(ctx);CHECK(hunter.calls==1&&hunter.last==75);
 hunter.learned.erase(75);EngageRanged(ctx);CHECK(hunter.calls==1);hunter.learned.insert(75);hunter.active.erase(75);EngageRanged(ctx);CHECK(hunter.calls==1);hunter.active.insert(75);
 hunter.autorepeat=true;EngageRanged(ctx);CHECK(hunter.calls==1);hunter.autorepeat=false;hunter.playerClass=8;EngageRanged(ctx);CHECK(hunter.calls==1);hunter.playerClass=3;
 ctx.target=nullptr;int petCalls=pet.ai.attacks;EngageRanged(ctx);CHECK(hunter.calls==1&&pet.ai.attacks==petCalls);ctx.target=&target;
 hunter.gcd.blocked=true;hunter.cooldown=true;hunter.form=1;hunter.power=0;hunter.result=1;EngageRanged(ctx);CHECK(hunter.calls==2&&hunter.last==75); // Preserve native autorepeat setup/checks, outside shared GCD helper.
 std::cout<<checks<<" Rotation checks\n";
}

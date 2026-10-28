#include "TacticalPlanningGuide.h"
#include "CampaignTypes.hpp"
#include "HexGrid.hpp"
namespace planningguide
{
    FString Place(const FPreparationMap& Map,int Tile)
    {
        const auto* Cell=Map.Cells.FindByPredicate([Tile](const FPlanningCell& C) { return C.Tile==Tile; });
        if(!Cell) return TEXT("目的地を選ぶ");
        FString Name;
        if(const auto* Site=Map.Sites.FindByPredicate([Tile](const FPlanningSite& S) { return S.Tile==Tile; })) Name=Site->Name;
        else
        {
            using frontline::Terrain;
            Name=Cell->Terrain==uint8(Terrain::Forest) ? TEXT("森林"):Cell->Terrain==uint8(Terrain::Mountain) ? TEXT("山地"):
                 Cell->Terrain==uint8(Terrain::River) ? TEXT("川"):Cell->Terrain==uint8(Terrain::Bridge) ? TEXT("橋"):TEXT("平地");
        }
        return Name+FString::Printf(TEXT(" (%d,%d)"),Tile%frontline::Width,Tile/frontline::Width);
    }
    FString Order(const FPreparationMap& Map,int Officer)
    {
        const auto* Duty=Map.Duties.FindByPredicate([Officer](const FPlanningDuty& D) { return D.Officer==Officer; });
        if(!Duty || Duty->Kind==0) return TEXT("未指定 ／ 戦場で自由に指揮");
        return Place(Map,Duty->Tile)+(Duty->Kind==1 ? TEXT(" → 固守"):TEXT(" → 伏兵準備"));
    }
    TArray<int> Route(const FPreparationMap& Map,int Officer,int Tile)
    {
        const auto* Start=Map.Starts.FindByPredicate([Officer](const FPlanningStart& S) { return S.Officer==Officer; });
        if(!Start || Officer<0 || Officer>=3) return {};
        TSet<int> Ground;
        for(const auto& C:Map.Cells)
            if(C.Terrain!=uint8(frontline::Terrain::River) && C.Terrain!=uint8(frontline::Terrain::Sea) && !Map.Blocked.Contains(C.Tile)) Ground.Add(C.Tile);
        if(!Ground.Contains(Tile)) return {};
        TArray<int> Search{Start->Tile};TMap<int,int> Previous;Previous.Add(Start->Tile,-1);
        for(int Index=0;Index<Search.Num() && !Previous.Contains(Tile);++Index)
            for(const auto& N:hexgrid::Neighbors(Search[Index]%frontline::Width,Search[Index]/frontline::Width))
            {
                const int Next=N.first+N.second*frontline::Width;
                if(Ground.Contains(Next) && !Previous.Contains(Next)) { Previous.Add(Next,Search[Index]);Search.Add(Next); }
            }
        if(!Previous.Contains(Tile)) return {};
        TArray<int> Path;for(int Current=Tile;Current!=-1;Current=Previous[Current]) Path.Insert(Current,0);
        return Path;
    }
}

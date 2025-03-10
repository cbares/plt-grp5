#include <client/Command.h>  // Included from library shared_static
#include "Command.h"
#include <stdlib.h>     /* srand, rand */
#include <time.h>       /* time */
#include <iostream>

using namespace std;

using namespace state;

namespace client {

    client::Command::Command(){
        srand(time(NULL));
        mStatsNameMap[MAX_LIFE_POINTS] = "MAX_LIFE_POINTS";
        mStatsNameMap[LIFE_POINTS] = "LIFE_POINTS";
        mStatsNameMap[ATTACK] = "ATTACK";
        mStatsNameMap[POWER] = "POWER";
        mStatsNameMap[DEFENSE] = "DEFENSE";
        mStatsNameMap[LUCK] = "LUCK";

        mActionMap[ATTACK_1] = "ATTACK_1";
        mActionMap[ATTACK_2] = "ATTACK_2";
        mActionMap[SPELL_1] = "SPELL_1";
        mActionMap[SPELL_2] = "SPELL_2";

        mUniqueTeamAttackBuff = 1;
    }

    float client::Command::ComputeWeakAndStrength(state::Major rAttackerMajor, state::Major rVictimMajor){
        const float STRENGTH     = 2;
        const float  NEUTRAL     = 1;
        const float WEAK     = 0.5;
        switch(rAttackerMajor){
            case ELEC:
                switch (rVictimMajor)
                {
                case ELEC:
                    return NEUTRAL;

                case INFO:
                    return NEUTRAL;

                case AUTO:
                    return WEAK;

                case SIGNAL:
                    return STRENGTH;

                case SCIENCES_HUMAINES:
                    return NEUTRAL;

                }
                break;
            
            case INFO:
                switch (rVictimMajor)
                {
                case ELEC:
                    return NEUTRAL;

                case INFO:
                    return NEUTRAL;

                case AUTO:
                    return STRENGTH;

                case SIGNAL:
                    return WEAK;

                case SCIENCES_HUMAINES:
                    return NEUTRAL;
                }
                break;

            case AUTO:
                switch (rVictimMajor)
                {
                case ELEC:
                    return STRENGTH;

                case INFO:
                    return WEAK;

                case AUTO:
                    return NEUTRAL;

                case SIGNAL:
                    return NEUTRAL;

                case SCIENCES_HUMAINES:
                    return NEUTRAL;

                }
                break;

            case SIGNAL:
                switch (rVictimMajor)
                {
                case ELEC:
                    return WEAK;

                case INFO:
                    return STRENGTH;

                case AUTO:
                    return NEUTRAL;

                case SIGNAL:
                    return NEUTRAL;

                case SCIENCES_HUMAINES:
                    return NEUTRAL;
                }
                break;

            case SCIENCES_HUMAINES:
                switch (rVictimMajor)
                {
                case SCIENCES_HUMAINES:
                    return STRENGTH;
                

                default: // All other major
                    return NEUTRAL;
                }
                break;
        }
        
    }

    int client::Command::ComputeAction(state::Character &rAttacker, state::Character &rVictim, CommandID rActionMade){
        float lCoeffMajor = 0;
        int lCriticalHit = 0;
        int lAttackDamage = 0;
        int lVictimDefense = 0;
        int lAttackStat = 0;
        int lVictimLifePoints = 0;
        int lVictimStat = 0;
        int lAttackerStat = 0;
        float lPVLost = 0;
        const float lMagicCoefficiant = 0.44;
        int lBuffValue = 0;
        string lAttackerName;
        string lVictimName;
        StatsName lBuffName;
        BuffType lBeneficial;
        Major lAttacker_Major;
        Major lVictim_Major;
        Action* lActionGot = nullptr;

        /*Récupération du type d'action*/
        switch (rActionMade)
        {
        case ATTACK_1:
            lActionGot = rAttacker.GetAction(state::ATTACK_1);
            break;
        
        case ATTACK_2:
            lActionGot = rAttacker.GetAction(state::ATTACK_2);
            break;

        case SPELL_1:
            lActionGot = rAttacker.GetAction(state::SPELL_1);
            break;

        case SPELL_2:
            lActionGot = rAttacker.GetAction(state::SPELL_2);
            break;

        }


        /*Calcul du nombre de PV perdu*/
        lAttackerName = rAttacker.GetName();
        lVictimName = rVictim.GetName();
        lAttacker_Major = rAttacker.GetMajor();
        lVictim_Major = rVictim.GetMajor();

        lCoeffMajor = ComputeWeakAndStrength(lAttacker_Major, lVictim_Major); // Calcul avantage/désavantage majeur
        lCriticalHit = ComputeCriticalHit(rAttacker.GetCharacterStats(StatsName::LUCK)); // Calcul coup critique
        lAttackDamage = lActionGot->GetDamage(); // Dégâts de l'attaque
        lVictimDefense = rVictim.GetCharacterStats(StatsName::DEFENSE); // Défense de la victime
        lAttackStat = rAttacker.GetCharacterStats(StatsName::ATTACK); // Attaque de l'attaquant
        lVictimLifePoints = rVictim.GetCharacterStats(LIFE_POINTS);
        
        lPVLost = (-1) * ((lCoeffMajor * lCriticalHit * lAttackDamage * lAttackStat*mUniqueTeamAttackBuff) / lVictimDefense) * lMagicCoefficiant;
        
        /*Application de la perte de PV*/
        rVictim.SetCharacterStats(LIFE_POINTS, lVictimLifePoints + lPVLost);
        
        if(mDebugInfo){
            cout  << mActionMap[rActionMade] << ": " << lAttackerName << " inflicted " << lPVLost << " PV lost on " << lVictimName << endl;
            cout << lVictimName << " life points is now " << rVictim.GetCharacterStats(LIFE_POINTS) << endl;
        }
        
        mUniqueTeamAttackBuff = 1; // Reset du team buff
        /*Calcul de l'effet du buff*/
        
        lBuffValue = lActionGot->GetStatBuffValue();
        lBeneficial = lActionGot->GetBuffBeneficial();
        lBuffName = lActionGot->GetStatBuffName();
        
        if(mActiveLPBuffTurnTime > 0){//Check if there is an active life point team buff
            lAttackerStat = rAttacker.GetCharacterStats(lBuffName);
            rAttacker.SetCharacterStats(LIFE_POINTS, lAttackerStat + mUniqueTeamLPBuff);

            if(mDebugInfo){
                cout << mActionMap[rActionMade] << ": " << lAttackerName << " got +" << mUniqueTeamLPBuff << " " << mStatsNameMap[LIFE_POINTS] << " team beneficial" << endl;
                cout << lAttackerName << " "<<  mStatsNameMap[LIFE_POINTS] << " is now " << rAttacker.GetCharacterStats(LIFE_POINTS) << endl;
                mActiveLPBuffTurnTime--;
            }
        
        }



        


        if(lBuffValue > 0){ // Si l'action possède un buff


            switch(lBeneficial){
                case BENEFICIAL_ATTACKER: // Buff bénéfique c'est l'attaquant qui reçoit le buff
                    lAttackerStat = rAttacker.GetCharacterStats(lBuffName);
                    rAttacker.SetCharacterStats(lBuffName, lAttackerStat + lBuffValue);

                    if(mDebugInfo){
                        cout << mActionMap[rActionMade] << ": " << rAttacker.GetName() << " got +" << lBuffValue << " " << mStatsNameMap[lBuffName] << " beneficial" << endl;
                        cout << rAttacker.GetName() << " "<<  mStatsNameMap[lBuffName] << " is now " << rAttacker.GetCharacterStats(lBuffName) << endl;
                    }
                    if(lBuffName == LIFE_POINTS){
                        return lBuffValue; //If the action is a PV gain, we return it 
                    }
                    
                    
                    break;


                case NEGATIVE_VICTIM: // Buff non bénéfique pour la victime (adversaire)
                    lBuffValue = lBuffValue * (-1);
                    lVictimStat = rVictim.GetCharacterStats(lBuffName);
                    rVictim.SetCharacterStats(lBuffName, lVictimStat + lBuffValue);

                    if(mDebugInfo){
                        cout << mActionMap[rActionMade] << ": " << rAttacker.GetName() << " inflicted " << lBuffValue << " " <<mStatsNameMap[lBuffName] << " disadvantageous on " << rVictim.GetName() << endl;
                        cout << rVictim.GetName() << " "<<  mStatsNameMap[lBuffName] << " is now " << rVictim.GetCharacterStats(lBuffName) << endl;
                    }
                    break;

                case NEGATIVE_ATTACKER: // Buff non bénéfique pour l'attaquant
                    lBuffValue = lBuffValue * (-1);
                    lAttackerStat = rAttacker.GetCharacterStats(lBuffName);
                    rAttacker.SetCharacterStats(lBuffName, lAttackerStat + lBuffValue);

                    if(mDebugInfo){
                        cout << mActionMap[rActionMade] << ": " << rAttacker.GetName() << " inflicted " << lBuffValue << " " <<mStatsNameMap[lBuffName] << " disadvantageous on " << rAttacker.GetName() << endl;
                        cout << rAttacker.GetName() << " "<<  mStatsNameMap[lBuffName] << " is now " << rAttacker.GetCharacterStats(lBuffName) << endl;
                    }
                    break;

                case BENEFICIAL_TEAM: // Buff ayant un effet au tour n+1
                    switch (lBuffName)
                    {
                    case ATTACK:
                        mUniqueTeamAttackBuff = (float)lBuffValue/10; // En %
                        if(mDebugInfo){
                            cout << rAttacker.GetName() << " give a team buff attack of " << lBuffValue << "% for next turn" << endl;
                        }
                        break;

                    case LIFE_POINTS:
                        mUniqueTeamLPBuff = lBuffValue;
                        if(mDebugInfo){
                            cout << rAttacker.GetName() << " give a team buff Life points of " << lBuffValue << " life points for next 4 turns" << endl;
                        }
                        mActiveLPBuffTurnTime = 4;
                        break;

                    }



            }

        }
        
        
        return lPVLost;

    }

    float client::Command::ComputeCriticalHit(int rCharacterLuck){
        int lRandomResult;
        int lCriticalHit;
        
        lRandomResult = rand() % 100 + 1;

        if(lRandomResult <= rCharacterLuck){
            lCriticalHit = 2;

            if(mDebugInfo){
                cout << "Critical hit !" << endl;
            }
        }
        else{
            lCriticalHit = 1;
            if(mDebugInfo){
                cout << "No critical hit" << endl;
            }
        }

        return lCriticalHit;

    }

    void client::Command::SetDebugInfo(bool rValue){
        mDebugInfo = rValue;
    }

}
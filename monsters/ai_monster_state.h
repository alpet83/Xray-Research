 
 //  Module      : ai_monster_state.h
 //  Created     : 06.07.2003
 //  Modified    : 01.10.2003
 //  Author      : Serge Zhem
 //  Description : interface for monster states
 
 #pragma once 
 
 #include "ai_monster_defs.h"
 
 // IState class
 
 class IState {
 
     enum {
         STATE_NOT_ACTIVE,
         STATE_INIT,
         STATE_RUN,
         STATE_DONE
     } m_tState;                                         
 
 protected:
     enum {
         PRIORITY_NONE    = 0,
         PRIORITY_LOWEST  = 5,
         PRIORITY_LOW     = 10,
         PRIORITY_NORMAL  = 15,
         PRIORITY_HIGH    = 20,
         PRIORITY_HIGHEST = 25,
     };
     
     
     TTime           m_dwCurrentTime;                    
     TTime           m_dwStateStartedTime;               
     TTime           m_dwNextThink;                      
     TTime           m_dwTimeLocked;                     
     
     TTime           m_dwInertia;                        
     u8              m_Priority;                         
 
 public:
                         IState          ();
         virtual         ~IState         () {}
 
                 void    Execute         (TTime cur_time);                       
 
     IC  virtual bool    CheckCompletion     () {return false;}                      
     IC  virtual bool    CheckStartCondition () {return true;}                       
     IC  virtual bool    IsCompleted         () {return false;}
     IC  virtual void    CriticalInterrupt   () {}
         virtual void    UpdateExternal      () {}
 
         virtual void    Reset           ();                                     
 
     IC          void    Activate        () {m_tState = STATE_INIT;}             
     IC          bool    Active          () {return (STATE_NOT_ACTIVE != m_tState);}
 
         virtual void    Init            ();                                 
         virtual void    Run             () = 0;                             
         virtual void    Done            ();                                 
 
     IC          void    SetNextThink    (TTime next_think) {m_dwNextThink = next_think + m_dwCurrentTime;}
 
 
     /* определение приоритета состояния (реализация инерции) */ 
                 void    SetPriority     (u8 new_priority) {m_Priority = new_priority;}
 
     IC          void    SetInertia      (TTime inertia) {m_dwInertia = inertia + m_dwCurrentTime;}
     IC          bool    IsInertia       () {return ((PRIORITY_NONE != m_Priority) && (m_dwInertia > m_dwCurrentTime));}
     
     IC          u8      GetPriority     () {return m_Priority;}
 
     IC  virtual bool    GetStateAggressiveness  (){return false;}
 };
 
 
 
 
 





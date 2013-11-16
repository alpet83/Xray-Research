 #ifndef SoundH
 #define SoundH
 #pragma once
 
 #ifdef XRSOUND_EXPORTS
     #define XRSOUND_API __declspec(dllexport)
 #else
     #define XRSOUND_API __declspec(dllimport)
 #endif
 
 #ifdef __BORLANDC__
     #define XRSOUND_EDITOR_API XRSOUND_API
 
     // editor only refs
     class XRSOUND_EDITOR_API SoundEnvironment_LIB;
 #else
     #define XRSOUND_EDITOR_API
 #endif
 
 #define SNDENV_FILENAME             "sEnvironment.xr"
 #define OGG_COMMENT_VERSION         0x0002 
 
 // refs
 class   CObject;
 class   XRSOUND_API                 CSound_params;
 class   XRSOUND_API                 CSound_source;
 class   XRSOUND_API                 CSound_interface;
 class   XRSOUND_API                 CSound_stream_interface;
 class   XRSOUND_API                 CSound_environment;
 
 //
 XRSOUND_API extern u32              psSoundFreq             ;
 XRSOUND_API extern u32              psSoundModel            ;
 XRSOUND_API extern float            psSoundVEffects         ;
 XRSOUND_API extern float            psSoundVMusic           ;
 XRSOUND_API extern float            psSoundRolloff          ;
 XRSOUND_API extern float            psSoundOcclusionScale   ;
 XRSOUND_API extern Flags32          psSoundFlags            ;
 XRSOUND_API extern int              psSoundTargets          ;
 XRSOUND_API extern int              psSoundCacheSizeMB      ;
 
 // Flags
 enum {
     ssHardware          = (1ul<<1ul),   
     ssEAX               = (1ul<<2ul),   
     ss_forcedword       = u32(-1)
 };
 enum {
     sf_22K,
     sf_44K,
     sf_forcedword = u32(-1)
 };
 enum {
     sq_DEFAULT,
     sq_NOVIRT,
     sq_LIGHT,
     sq_HIGH,
     sq_forcedword = u32(-1)
 };
 enum {
     st_Undefined        = 0,
     st_SourceType       = u32(-1),
     st_forcedword       = u32(-1),
 };
 enum {
     sm_Looped           = (1ul<<0ul),   
     sm_2D               = (1ul<<1ul),   
     sm_forcedword       = u32(-1),
 };
 struct  ref_sound
 {
     CSound_source*                  handle;         
     CSound_interface*               feedback;       
     int                             g_type;         
     CObject*                        g_object;       
 
 
     ref_sound()                         { handle = 0; feedback=0; g_type=st_SourceType; g_object=0; }
     ~ref_sound()                        { destroy(); }
 
 
     IC void                 create                  ( BOOL _3D, LPCSTR name,    int     type=st_SourceType);
 
 
     IC void                 clone                   ( const ref_sound& from,    int     type=st_SourceType);
 
 
     IC void                 destroy                 ( );
 
 
 
     IC void                 play                    ( CObject* O ,                                          u32 flags=0 , float delay=0.f );
     IC void                 play_unlimited          ( CObject* O ,                                          u32 flags=0 , float delay=0.f );
     IC void                 play_at_pos             ( CObject* O ,  const Fvector &pos ,    u32 flags=0 , float delay=0.f );
     IC void                 play_at_pos_unlimited   ( CObject* O ,  const Fvector &pos ,    u32 flags=0 , float delay=0.f );
 
 
     IC void                 stop                    ( );
     IC void                 set_position            ( const Fvector &pos);
     IC void                 set_frequency           ( float freq);
     IC void                 set_range               ( float min, float max );
     IC void                 set_volume              ( float vol );
     IC void                 set_priority            ( float vol );
 
     IC const CSound_params* get_params              ( );
     IC void                 set_params              ( CSound_params* p );
 };
 
 class XRSOUND_API   CSound_source
 {
 public:
     virtual u32             length_ms               ( )     = 0;
     virtual u32             game_type               ( )     = 0;
     virtual LPCSTR          file_name               ( )     = 0;
     virtual float           start_volume            ( )     = 0;
 };
 
 class XRSOUND_API   CSound_environment
 {
 public:
 };
 
 class XRSOUND_API   CSound_params
 {
 public:
     Fvector         position;
     float           volume;
     float           freq;
     float           min_distance;
     float           max_distance;
 };
 
 class XRSOUND_API   CSound_interface
 {
 public:
     virtual void                    switch_to_2D            ()                                                          = 0;
     virtual void                    switch_to_3D            ()                                                          = 0;
     virtual void                    set_position            (const Fvector &pos)                                        = 0;
     virtual void                    set_frequency           (float freq)                                                = 0;
     virtual void                    set_range               (float min, float max)                                      = 0;
     virtual void                    set_volume              (float vol)                                                 = 0;
     virtual void                    set_priority            (float vol)                                                 = 0;
     virtual void                    stop                    ( )                                                         = 0;
     virtual const CSound_params*    get_params              ( )                                                         = 0;
 };
 
 class XRSOUND_API   CSound_stream_interface
 {
 public:
 };
 
 class XRSOUND_API   CSound_stats
 {
 public:
     u32             _rendered;
     u32             _simulated;
     u32             _cache_hits;
     u32             _cache_misses;
 };
 
 typedef     void __stdcall sound_event                      (ref_sound* S, float range);
 
 class XRSOUND_API   CSound_manager_interface
 {
     virtual void                    _initialize             ( u64 window )                                                                          = 0;
     virtual void                    _clear                  ( )                                                                                     = 0;
 public:
 
     static void                     _create                 ( u64 window );
     static void                     _destroy                ( );
 
     virtual void                    _restart                ( )                                                                                     = 0;
 
 
     virtual void                    create                  ( ref_sound& S, BOOL _3D,   LPCSTR fName,   int     type=st_SourceType)                 = 0;
     virtual void                    destroy                 ( ref_sound& S)                                                                         = 0;
     virtual void                    play                    ( ref_sound& S, CObject* O,                             u32 flags=0, float delay=0.f)   = 0;
     virtual void                    play_unlimited          ( ref_sound& S, CObject* O,                             u32 flags=0, float delay=0.f)   = 0;
     virtual void                    play_at_pos             ( ref_sound& S, CObject* O,     const Fvector &pos,     u32 flags=0, float delay=0.f)   = 0;
     virtual void                    play_at_pos_unlimited   ( ref_sound& S, CObject* O,     const Fvector &pos,     u32 flags=0, float delay=0.f)   = 0;
 
     virtual void                    set_volume              ( float f=1.f )                                                                         = 0;
     virtual void                    set_geometry_env        ( IReader* I )                                                                          = 0;
     virtual void                    set_geometry_occ        ( CDB::MODEL* M )                                                                       = 0;
     virtual void                    set_handler             ( sound_event* E )                                                                      = 0;
 
     virtual void                    update                  ( const Fvector& P, const Fvector& D, const Fvector& N, float dt )                      = 0;
     virtual void                    update_events           ( )                                                                                     = 0;
     virtual void                    statistic               ( CSound_stats&  dest )                                                                 = 0;
 
 #ifdef __BORLANDC__
     virtual SoundEnvironment_LIB*   get_env_library         ()                                                                                      = 0;
     virtual void                    refresh_env_library     ()                                                                                      = 0;
     virtual void                    set_user_env            (CSound_environment* E)                                                                 = 0;
     virtual void                    refresh_sources         ()                                                                                      = 0;
     virtual void                    set_environment         (u32 id, CSound_environment** dst_env)                                                  = 0;
     virtual void                    set_environment_size    (CSound_environment* src_env, CSound_environment** dst_env)                             = 0;
 #endif
 };
 extern XRSOUND_API CSound_manager_interface*        Sound;
 
 IC void ref_sound::create                       ( BOOL _3D, LPCSTR name,    int     type)               {   ::Sound->create                 (*this,_3D,name,type);              }
 IC void ref_sound::destroy                      ( )                                                     {   ::Sound->destroy                (*this);                            }
 IC void ref_sound::play                         ( CObject* O,                       u32 flags, float d) {   ::Sound->play                   (*this,O,flags,d);                  }
 IC void ref_sound::play_unlimited               ( CObject* O,                       u32 flags, float d) {   ::Sound->play_unlimited         (*this,O,flags,d);                  }
 IC void ref_sound::play_at_pos                  ( CObject* O,   const Fvector &pos, u32 flags, float d) {   ::Sound->play_at_pos            (*this,O,pos,flags,d);              }
 IC void ref_sound::play_at_pos_unlimited        ( CObject* O,   const Fvector &pos, u32 flags, float d) {   ::Sound->play_at_pos_unlimited  (*this,O,pos,flags,d);              }
 IC void ref_sound::set_position                 ( const Fvector &pos)                                   {   VERIFY(feedback);feedback->set_position(pos);                       }
 IC void ref_sound::set_frequency                ( float freq)                                           {   if (feedback)   feedback->set_frequency(freq);                      }
 IC void ref_sound::set_range                    ( float min, float max )                                {   if (feedback)   feedback->set_range(min,max);                       }
 IC void ref_sound::set_volume                   ( float vol )                                           {   if (feedback)   feedback->set_volume(vol);                          }
 IC void ref_sound::set_priority                 ( float p )                                             {   if (feedback)   feedback->set_priority(p);                      }
 IC void ref_sound::stop                         ( )                                                     {   if (feedback)   feedback->stop();                                   }
 IC const CSound_params* ref_sound::get_params   ( )                                                     {   if (feedback)   return feedback->get_params(); else return NULL;    }
 IC void ref_sound::set_params                   ( CSound_params* p )                                    
 {   
     if (feedback){
         feedback->set_position  (p->position);
         feedback->set_frequency (p->freq);
         feedback->set_range     (p->min_distance,p->max_distance);
         feedback->set_volume    (p->volume);
     }
 }
 IC void ref_sound::clone                        ( const ref_sound& from, int type )     
 {
     feedback    = 0;
     g_object    = 0;
     handle      = from.handle;
     g_type      = (type==st_SourceType)?handle->game_type():type;
 }
 #endif





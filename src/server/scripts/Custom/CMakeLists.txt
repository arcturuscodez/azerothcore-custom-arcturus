set(scripts_STAT_SRCS
  ${scripts_STAT_SRCS}
  ${AC_SCRIPTS_DIR}/Custom/custom_script_loader.cpp
  ${AC_SCRIPTS_DIR}/Custom/warlock_demonic_empowerment.cpp
  ${AC_SCRIPTS_DIR}/Custom/warlock_felguard_mandate.cpp
  ${AC_SCRIPTS_DIR}/Custom/warlock_crimson_shade.cpp
  ${AC_SCRIPTS_DIR}/Custom/warlock_corrupted_blood.cpp
  ${AC_SCRIPTS_DIR}/Custom/warlock_wrath_of_chaos.cpp
  ${AC_SCRIPTS_DIR}/Custom/cs_demons.cpp
  ${AC_SCRIPTS_DIR}/Custom/endless_instances.cpp
  ${AC_SCRIPTS_DIR}/Custom/arcturus_trade_skills.cpp
  ${AC_SCRIPTS_DIR}/Custom/arcturus_gameplay_watch.cpp
)

AC_ADD_SCRIPT_LOADER("Custom" "ScriptLoader.h")

message("  -> Prepared: Arcturus Custom scripts")

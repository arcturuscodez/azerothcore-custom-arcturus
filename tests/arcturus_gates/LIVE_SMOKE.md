# Live smoke — post-wipe checklist (Cinderfury + Noggenfogger baseline)
#
# After empty DBs + worldserver applies pending SQL + rebuild/restart:
#
# 1. Confirm NO retired customs: `.lookup item Mannoroth` / `.additem 900001` fail
# 2. `.additem 900017` — equip Cinderfury → chat ignite; fire spells stronger; stam down
# 3. Cast Hellfire → persistent aura, no self-burn; cast again to quench
# 4. Use Cinderfury → Infernal Detonation (HP burn + nova); 2 min CD
# 5. `.additem 900016` — use → stock Noggenfogger Elixir morph (spell 16591);
#    use again → flesh returns; death also clears
# 6. Inferno → pet named Infernal with Immolation/Cleave/Anguish/War Stomp; stone consumed
# 7. Ritual of Doom → Doomguard permanent pet; figurine consumed
# 8. `.demons` works; souls table exists; kill → +1 soul; tempering every 100 lifetime;
#    talent grants appear at milestones (`.demons ranks`); no auto-learned borrowed spells
# 9. Paragon: worldserver loads ALE without `unpack` nil errors
# 10. Client: CustomItemFix + ArcturusItemFix for bag icons / right-click on 900016/900017

# Live smoke — post-deploy checklist (Cinderfury + Noggenfogger baseline)
#
# 1. `.additem 900017` — equip Cinderfury → chat ignite; fire spells stronger; stam down
# 2. Cast Hellfire → persistent aura, no self-burn; cast again to quench
# 3. Use Cinderfury → Infernal Detonation (HP burn + nova); 2 min CD
# 4. `.additem 900016` — use → display 21151 morph at ~player size; use again → flesh returns
#    (tune WarlockLegendary.Noggenfogger.Scale if still too big/small; default 0.35)
# 5. Confirm retired 900001–900015 / 900018+ are gone (`.additem` fails / no template)
# 6. Paragon: worldserver loads ALE without `unpack` nil errors
# 7. Client: CustomItemFix or Item.dbc for bag icons on 900016/900017 if still `?`

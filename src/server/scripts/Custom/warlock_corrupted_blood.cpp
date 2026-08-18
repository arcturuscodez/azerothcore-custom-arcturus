/*
 * Corrupted Blood (90042) — Soul Reaver (5000 souls) rank passive.
 *
 * Replaces Feltouched Communion (90003/90009), which is retired and stripped on login.
 *
 * A PASSIVE aura is never sent to the client (Aura::CanBeSentToClient), so the stats the
 * player must see live on a second spell:
 *
 *   90042  learned PASSIVE  — PERIODIC_DUMMY (this script) + healing taken + health/5s
 *   90043  visible buff     — armor + all resistances + mana/5s, cast on self from here
 *   90044  Coagulate        — SCHOOL_ABSORB, amount is script-set (40% of max health)
 *   90045  Coagulate        — shadow nova, borrows the DK Blood Boil visual
 *
 * That split is the Master Demonologist shape and is what puts an icon on the buff bar.
 * It also buys a second set of three effect slots, which the stat list needs.
 *
 * Coagulate is a low-health emergency shield, not a death save: it fires after a hit
 * leaves the warlock under 25% health, on a 2 minute internal cooldown held on the aura
 * script (90042 is permanent, so the member lives as long as the aura does).
 */

#include "Chat.h"
#include "Config.h"
#include "GameTime.h"
#include "Player.h"
#include "Random.h"
#include "SpellAuraEffects.h"
#include "SpellInfo.h"
#include "SpellScript.h"
#include "SpellScriptLoader.h"
#include "StringFormat.h"
#include "Timer.h"
#include "Unit.h"
#include "WorldSession.h"

#include <algorithm>
#include <array>

namespace
{
    // Script-cast halves. Never taught, never on the Chaos tab — they would show up as
    // phantom spellbook entries.
    constexpr uint32 SPELL_CORRUPTED_BLOOD_BUFF = 90043;
    constexpr uint32 SPELL_COAGULATE_ABSORB     = 90044;
    constexpr uint32 SPELL_COAGULATE_NOVA       = 90045;

    // Cosmetic seam: 0 = no visual. Lets an animation be bolted on with a config change
    // plus one spell_dbc row instead of a rebuild.
    constexpr char const* CONFIG_VISUAL_SPELL_ID  = "Arcturus.CorruptedBlood.VisualSpellId";
    constexpr char const* CONFIG_WHISPERS_ENABLE  = "Arcturus.CorruptedBlood.Whispers.Enable";
    constexpr char const* CONFIG_WHISPER_MIN_SECS = "Arcturus.CorruptedBlood.Whispers.MinSeconds";
    constexpr char const* CONFIG_WHISPER_MAX_SECS = "Arcturus.CorruptedBlood.Whispers.MaxSeconds";

    constexpr uint32 WHISPER_MIN_SECONDS_DEFAULT = 240;
    constexpr uint32 WHISPER_MAX_SECONDS_DEFAULT = 900;
    constexpr uint32 CONFIG_CACHE_MS = 5000u;

    constexpr uint8  COAGULATE_TRIGGER_HEALTH_PCT = 25;
    constexpr uint8  COAGULATE_ABSORB_HEALTH_PCT  = 40;
    constexpr uint32 COAGULATE_COOLDOWN_SECONDS   = 120;

    inline constexpr std::array<char const*, 16> BLOOD_WHISPERS = {{
        "You were never the one in control.",
        "I was here before the demons. I will be here after.",
        "Do you feel how thin your skin has become?",
        "Every soul you take, I taste first.",
        "Your heart is such a small cage.",
        "They knelt to the wrong thing. They knelt to you.",
        "Keep killing. I am almost awake.",
        "You call it magic. I call it feeding.",
        "There is so much of me now, and so little of you.",
        "Do not stop. I get so cold when you rest.",
        "The demons follow you because they are afraid of me.",
        "One day you will look in a mirror and I will wave back.",
        "I remember the name you had before.",
        "Your bones hum when I am pleased.",
        "Let me drive. Just for one fight.",
        "You are not a warlock. You are a wound that walks."
    }};

    struct BloodConfig
    {
        uint32 visualSpellId = 0;
        bool whispersEnable = true;
        uint32 whisperMinSecs = WHISPER_MIN_SECONDS_DEFAULT;
        uint32 whisperMaxSecs = WHISPER_MAX_SECONDS_DEFAULT;
    };

    BloodConfig _bloodCache{};
    uint32 _bloodCacheMs = 0;

    BloodConfig const& LoadedBloodConfig()
    {
        uint32 const now = getMSTime();
        if (!_bloodCacheMs || getMSTimeDiff(_bloodCacheMs, now) >= CONFIG_CACHE_MS)
        {
            _bloodCache.visualSpellId = sConfigMgr->GetOption<uint32>(CONFIG_VISUAL_SPELL_ID, 0);
            _bloodCache.whispersEnable = sConfigMgr->GetOption<bool>(CONFIG_WHISPERS_ENABLE, true);
            _bloodCache.whisperMinSecs = sConfigMgr->GetOption<uint32>(
                CONFIG_WHISPER_MIN_SECS, WHISPER_MIN_SECONDS_DEFAULT);
            _bloodCache.whisperMaxSecs = sConfigMgr->GetOption<uint32>(
                CONFIG_WHISPER_MAX_SECS, WHISPER_MAX_SECONDS_DEFAULT);
            _bloodCacheMs = now ? now : 1u;
        }
        return _bloodCache;
    }

    uint32 ConfiguredVisualSpellId()
    {
        return LoadedBloodConfig().visualSpellId;
    }

    time_t RollNextWhisper()
    {
        BloodConfig const& cfg = LoadedBloodConfig();
        uint32 minSecs = cfg.whisperMinSecs;
        uint32 maxSecs = cfg.whisperMaxSecs;
        if (maxSecs < minSecs)
            std::swap(minSecs, maxSecs);

        return GameTime::GetGameTime().count() + urand(minSecs, maxSecs);
    }
}

class spell_warlock_corrupted_blood : public AuraScript
{
    PrepareAuraScript(spell_warlock_corrupted_blood);

    bool Validate(SpellInfo const* /*spellInfo*/) override
    {
        return ValidateSpellInfo({ SPELL_CORRUPTED_BLOOD_BUFF, SPELL_COAGULATE_ABSORB, SPELL_COAGULATE_NOVA });
    }

    void HandleApply(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        EnsureVisibleBuff(target);

        if (uint32 visualId = ConfiguredVisualSpellId())
            target->CastSpell(target, visualId, true);

        // Roll on apply so a fresh login never opens with a whisper.
        _nextWhisper = RollNextWhisper();
    }

    void HandleRemove(AuraEffect const* /*aurEff*/, AuraEffectHandleModes /*mode*/)
    {
        Unit* target = GetTarget();
        target->RemoveAurasDueToSpell(SPELL_CORRUPTED_BLOOD_BUFF);

        if (uint32 visualId = ConfiguredVisualSpellId())
            target->RemoveAurasDueToSpell(visualId);
    }

    void HandlePeriodic(AuraEffect const* /*aurEff*/)
    {
        Unit* target = GetTarget();
        // The passive is the source of truth. A dispel, a zone change or a config flip on
        // the visual can drop the visible half while 90042 is still on — re-assert instead
        // of trusting the apply hook to have been the last word.
        EnsureVisibleBuff(target);
        TryWhisper(target->ToPlayer());
    }

    bool CheckProc(ProcEventInfo& eventInfo)
    {
        DamageInfo* damageInfo = eventInfo.GetDamageInfo();
        if (!damageInfo || !damageInfo->GetDamage())
            return false;

        Unit* target = GetTarget();
        if (!target->IsAlive() || !target->HealthBelowPct(COAGULATE_TRIGGER_HEALTH_PCT))
            return false;

        return GameTime::GetGameTime().count() >= _coagulateReadyAt;
    }

    void HandleProc(ProcEventInfo& /*eventInfo*/)
    {
        Unit* target = GetTarget();
        _coagulateReadyAt = GameTime::GetGameTime().count() + COAGULATE_COOLDOWN_SECONDS;

        // DBC base points stay 0: stock shields read the real amount off the aura, so the
        // client tooltip only shows a number if the script supplies one.
        int32 const absorb = int32(target->CountPctFromMaxHealth(COAGULATE_ABSORB_HEALTH_PCT));
        target->CastCustomSpell(SPELL_COAGULATE_ABSORB, SPELLVALUE_BASE_POINT0, absorb, target, true);
        target->CastSpell(target, SPELL_COAGULATE_NOVA, true);
    }

    void Register() override
    {
        AfterEffectApply += AuraEffectApplyFn(spell_warlock_corrupted_blood::HandleApply, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        AfterEffectRemove += AuraEffectRemoveFn(spell_warlock_corrupted_blood::HandleRemove, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY, AURA_EFFECT_HANDLE_REAL);
        OnEffectPeriodic += AuraEffectPeriodicFn(spell_warlock_corrupted_blood::HandlePeriodic, EFFECT_0, SPELL_AURA_PERIODIC_DUMMY);
        DoCheckProc += AuraCheckProcFn(spell_warlock_corrupted_blood::CheckProc);
        OnProc += AuraProcFn(spell_warlock_corrupted_blood::HandleProc);
    }

private:
    static void EnsureVisibleBuff(Unit* target)
    {
        if (!target->HasAura(SPELL_CORRUPTED_BLOOD_BUFF))
            target->CastSpell(target, SPELL_CORRUPTED_BLOOD_BUFF, true);
    }

    void TryWhisper(Player* player)
    {
        if (!player || !LoadedBloodConfig().whispersEnable)
            return;

        WorldSession* session = player->GetSession();
        if (!session || session->IsBot() || session->PlayerLoading())
            return;

        time_t const now = GameTime::GetGameTime().count();
        if (now < _nextWhisper)
            return;

        _nextWhisper = RollNextWhisper();

        char const* line = BLOOD_WHISPERS[urand(0, uint32(BLOOD_WHISPERS.size()) - 1)];
        ChatHandler(session).SendSysMessage(Acore::StringFormat(
            "|cff8b0000Your blood whispers:|r |cffb02020\"{}\"|r", line));
    }

    time_t _coagulateReadyAt = 0;
    time_t _nextWhisper = 0;
};

void AddSC_warlock_corrupted_blood()
{
    RegisterSpellScript(spell_warlock_corrupted_blood);
}

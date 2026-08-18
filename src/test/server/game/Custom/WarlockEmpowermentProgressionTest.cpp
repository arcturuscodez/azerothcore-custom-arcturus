/*
 * Contract tests for Demonic Empowerment rank / talent math.
 *
 * RankIndexFor and BonusTalentPointsFor are header-inline so these compile
 * without linking Custom scripts. They catch table-vs-formula drift that would
 * desync .demons, talent grants, and the Chaos-tab unlocks.
 */

#include "Custom/warlock_demonic_empowerment.h"
#include "Custom/warlock_arcturus_spells.h"
#include "gtest/gtest.h"

using namespace WarlockEmpowerment;

TEST(WarlockEmpowermentProgression, RankIndexBoundaries)
{
    EXPECT_EQ(RankIndexFor(0), 0u);
    EXPECT_EQ(RankIndexFor(99), 0u);
    EXPECT_EQ(RankIndexFor(100), 1u);
    EXPECT_EQ(RankIndexFor(249), 1u);
    EXPECT_EQ(RankIndexFor(250), 2u);
    EXPECT_EQ(RankIndexFor(499), 2u);
    EXPECT_EQ(RankIndexFor(500), 3u);
    EXPECT_EQ(RankIndexFor(999), 3u);
    EXPECT_EQ(RankIndexFor(1000), 4u);
    EXPECT_EQ(RankIndexFor(2499), 4u);
    EXPECT_EQ(RankIndexFor(2500), 5u);
    EXPECT_EQ(RankIndexFor(4999), 5u);
    EXPECT_EQ(RankIndexFor(5000), 6u);
    EXPECT_EQ(RankIndexFor(499999), 14u);
    EXPECT_EQ(RankIndexFor(500000), 15u);
    EXPECT_EQ(RankIndexFor(0xFFFFFFFFu), RANKS.size() - 1);
}

TEST(WarlockEmpowermentProgression, RankTableIsMonotonicAndNamed)
{
    ASSERT_FALSE(RANKS.empty());
    EXPECT_EQ(RANKS.front().minKills, 0u);
    EXPECT_STREQ(RANKS.front().name, "Apprentice");
    EXPECT_STREQ(RANKS.back().name, "Void Eternal");
    for (std::size_t i = 1; i < RANKS.size(); ++i)
        EXPECT_GT(RANKS[i].minKills, RANKS[i - 1].minKills) << "rank " << i;
}

TEST(WarlockEmpowermentProgression, TalentGrantsCumulative)
{
    EXPECT_EQ(BonusTalentPointsFor(0), 0u);
    EXPECT_EQ(BonusTalentPointsFor(99), 0u);
    EXPECT_EQ(BonusTalentPointsFor(100), 5u);
    EXPECT_EQ(BonusTalentPointsFor(499), 5u);
    EXPECT_EQ(BonusTalentPointsFor(500), 10u);
    EXPECT_EQ(BonusTalentPointsFor(1000), 20u);
    EXPECT_EQ(BonusTalentPointsFor(2500), 30u);
    EXPECT_EQ(BonusTalentPointsFor(5000), 45u);
    EXPECT_EQ(BonusTalentPointsFor(10000), 60u);
    EXPECT_EQ(BonusTalentPointsFor(25000), 80u);
    EXPECT_EQ(BonusTalentPointsFor(50000), 100u);
    EXPECT_EQ(BonusTalentPointsFor(100000), 120u);
    EXPECT_EQ(BonusTalentPointsFor(249999), 120u);
    EXPECT_EQ(BonusTalentPointsFor(250000), 145u);
    EXPECT_EQ(BonusTalentPointsFor(500000), 145u);

    uint32 sum = 0;
    for (TalentGrant const& grant : TALENT_GRANTS)
        sum += grant.points;
    EXPECT_EQ(sum, 145u);
}

TEST(WarlockEmpowermentProgression, RankSpellsDoNotOverlapRetired)
{
    for (RankSpell const& taught : RANK_SPELLS)
    {
        for (uint32 retired : RETIRED_RANK_SPELLS)
            EXPECT_NE(taught.id, retired) << taught.name;
    }
}

TEST(WarlockEmpowermentProgression, ClampAppliedSouls)
{
    EXPECT_EQ(ClampAppliedSouls(0, 10000), 0u);
    EXPECT_EQ(ClampAppliedSouls(500, 10000), 500u);
    EXPECT_EQ(ClampAppliedSouls(10000, 10000), 10000u);
    EXPECT_EQ(ClampAppliedSouls(10001, 10000), 10000u);
    EXPECT_EQ(ClampAppliedSouls(50000, 0), 50000u) << "cap 0 is uncapped";
}

TEST(WarlockEmpowermentProgression, BrandCapIsBelowDefaultSoulCap)
{
    EXPECT_EQ(ArcturusSpells::BRAND_SOUL_CAP, 500u);
    EXPECT_LT(ArcturusSpells::BRAND_SOUL_CAP, 10000u);
    EXPECT_EQ(ClampAppliedSouls(ArcturusSpells::BRAND_SOUL_CAP + 1, ArcturusSpells::BRAND_SOUL_CAP),
        ArcturusSpells::BRAND_SOUL_CAP);
}

TEST(WarlockEmpowermentProgression, ShadeAndWardShareDreadWarlockGate)
{
    uint32 shadeSouls = 0;
    uint32 wardSouls = 0;
    uint32 wrathSouls = 0;
    uint32 bloodSouls = 0;
    for (RankSpell const& taught : RANK_SPELLS)
    {
        if (taught.id == ArcturusSpells::SPELL_CRIMSON_SHADE)
            shadeSouls = taught.minSouls;
        if (taught.id == SPELL_WARD_OF_THE_SOUL_EATER)
            wardSouls = taught.minSouls;
        if (taught.id == SPELL_WRATH_OF_CHAOS)
            wrathSouls = taught.minSouls;
        if (taught.id == SPELL_CORRUPTED_BLOOD)
            bloodSouls = taught.minSouls;
    }
    EXPECT_EQ(shadeSouls, 2500u);
    EXPECT_EQ(wardSouls, 2500u);
    EXPECT_EQ(wrathSouls, 5000u);
    EXPECT_EQ(bloodSouls, 5000u);
}

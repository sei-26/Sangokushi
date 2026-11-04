#pragma once
class ATacticalField;
// Explicit visual test fixture: a visible enemy near two officers; no fabricated battle result.
bool PrepareSupportContactPreview(ATacticalField*);
// A bounded tactical fixture after an ordinary paid deployment, never a settled victory.
bool PrepareSupportActionPreview(ATacticalField*,int Variant);
bool PrepareSupportCombinationPreview(ATacticalField*,int Variant);
bool PrepareDefensePreview(ATacticalField*,int Variant);
bool PrepareDefenseAftermathPreview(ATacticalField*,int Variant);

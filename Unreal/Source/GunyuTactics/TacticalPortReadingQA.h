#pragma once
class ATacticalField;
// Plays the initial battle and explicitly retains the optional port mission.
bool PreparePortReadingPreview(ATacticalField* Field,int Phase,bool North);
bool VerifyPortReading(ATacticalField* Field);

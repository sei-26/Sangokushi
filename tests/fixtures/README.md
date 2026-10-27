# Journey save compatibility fixture

`journey-v1-chain.bin` is a real schema-1 archive written by the pre-combo rules library (repository state b38d0c7) before modifying the serializer on 2026-10-27. No user save data is included.

Created through `JourneySaveData`: Reset → Meet → snapshot council → Prepare(0,0) → Execute Lure(0,3,At(8,11)), Ambush(1,3), Breakthrough(2,3) → EncodeJourneySave.

Expected: three successful legacy events, damage 0/413/556, original known contacts and council snapshot. New readers must preserve those values with unknown/legacy combo=0, including after saving again in the current schema.

SHA-256: `a2323dcb3eb71ae85ef584b886af0db447ed6ed8482bd4219dae3d755abf2933`.

## Schema 2

`journey-v2-combo.bin` was written before the battle-story serializer change at repository state 7bbcddb, using the same Reset/Meet/snapshot/Prepare(0,0)/Lure-Ambush-Breakthrough sequence. Expected current combo damage is 0/553/781 and multipliers 100/125/150. Schema-3 readers must preserve those facts without inventing personal trust, a pledge, or historical cooperation participation.

SHA-256: `e17b495b2dd6f243b12f98acbf2e82f2e7d59153f34b289c860cced6d07a2f60`.

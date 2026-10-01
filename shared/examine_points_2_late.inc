/* Gleich auf PC und C64; der Schrein am Morgen, nachdem die graue Spur gelesen ist. */
    {.kind = POINT_SYMBOL,
     .map = MAP_FOREST,
     .symbol = 'O',
     .needs = OBS(OBS_GREY_TRACE),
     .grants = OBS(OBS_SHRINE_INSCRIPTION),
     .dialogue = D_X_INSCRIPTION_LATE,
     .note = N_VISITOR},

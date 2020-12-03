/**
 * Javascript file um Daten zu speichern, hier anpassen um Punkte, Gruppen etc. anzupassen 
 */

//Struktur: 
//        Nummer des LED: "Name des Punktes"
var points = {
    0: "Bromatte",
    1: "BOB Lauterbrunnen",
    2: "Wengeneralpbahn",
    3: "Wengeneralpbahn",
    4: "leer",
    5: "Wengeneralpbahn Wengen",
    6: "BOB Grindelwald",
    7: "BOB Grindelwald",
    8: "leer",
    9: "Sandweidli",
    10: "Wengeneralpbahn Alpiglen",
    11: "Wengeneralpbahn Alpiglen",
    12: "Lauberhorn",
    13: "Alpiglen",
    14: "BOB Grindelwald",
    15: "Wengeneralpbahn Wengen",
    16: "Schwarze Lütschine",
    17: "Schwarze Lütschine",
    18: "Weisshorn",
    19: "Weisse Lütschine",
    20: "Birchen",
    21: "leer",
    22: "Weisse Lütschine",
    23: "Weisse Lütschine",
    24: "Leiterhorn",
    25: "Wengeneralp",
    26: "Weisse Lütschine",
    27: "Kleiner Tschugggen",
    28: "Bussalp",
    29: "Zweilütschinen",
    30: "Drahtseilbahn Wengen-Männlichen",
    31: "Männlichen",
    32: "Bustligen",
    33: "Grindelwald",
    34: "Schwarze Lütschine",
    35: "Schwendi",
    36: "Wengen",
    37: "Lütschental",
    38: "Gündlischwand",
    39: "Schwarze Lütschine",
    40: "Schynigeplatten-Bahn",
    41: "Schynigeplatten-Bahn",
    42: "Schynige Platten",
    43: "Drahtseilbahn nach Usserläger",
    44: "Alpweg",
    45: "Laucheren",
    46: "Tschuggen",
    47: "Burg",
    48: "Bira",
    49: "Sägisse",
    50: "Roti Flueh",
    51: "Wengeneralpbahn",
    52: "Laucherehorn",
    53: "Oberberghorn",
    54: "leer",
    55: "Reeti",
    56: "Esel",
    57: "Winteregg",
    58: "Männlenenhütte",
    59: "Mittagskrinne",
    60: "Faulhorn",
    61: "Gassenhorn",
    62: "Sägistalsee",
    63: "Bachsee",
};

//Struktur von Einträgen: 
//  ["Name",  [Rot-Wert, Grün-Wert, Blau-Wert],   [Punkt1, Punkt2, Punkt3...]],
var groups = [
    ["Gewaesser", [0, 0, 255],
        [16, 17, 19, 22, 23, 26, 34, 39, 62, 63]
    ],
    ["Gipfel", [192, 192, 192],
        [12, 18, 24, 27, 31, 46, 47, 48, 49, 50, 52, 53, 55, 56, 57, 59, 60, 61, ]
    ],
    ["Weiler", [255, 128, 0],
        [0, 9, 13, 20, 25, 28, 32, 35, 38, 44, 45, ]
    ],
    ["Orte", [255, 255, 0],
        [29, 33, 36, 37, 42, ]
    ],
    ["Huetten", [0, 255, 0],
        [58, ]
    ],
    ["Bahnen", [255, 0, 0],
        [1, 2, 3, 5, 6, 7, 10, 11, 14, 40, 41, 51, ]
    ],
    ["Drahtseilbahn", [255, 0, 0],
        [30, 43, ]
    ]
];
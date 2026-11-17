Conventions :

- Distances en mètres (1 unité = 1 m), coefficients de diffusion/absorption en m-1.
- Scène : Y vertical, origine au niveau de référence du terrain.
- Repère planétaire : centre à l’origine ; `P_planète = P_monde + (0, R_sol, 0)`.
- Rayon `r = length(P_planète)`, altitude `h = r - R_sol`, verticale locale `up = P_planète / r`.
- Directions normalisées, du point vers la destination ; `sunDirection` pointe vers le Soleil.
- Transmittance LUT : `T_top(r, mu)`, avec `mu = dot(up, direction)`, jusqu’au sommet sans traverser le sol.
- RGB = transmissions sans unité dans [0, 1] ; A = 1. Occultation par la planète traitée séparément.
- LUTs physiques : données linéaires, sans exposition, tone mapping ou conversion sRGB.
- Transmittance : axe X = longueur du trajet normalisée, axe Y = distance à l’horizon normalisée ; limite tangente au sol incluse.
- Paramétrisation de Bruneton : extrémités du domaine aux centres des texels de bord, avec conversions UV <-> domaine symétriques (dimensions > 1).

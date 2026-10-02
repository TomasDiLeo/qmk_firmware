// Custom Fire Effect with a reactive layer
RGB_MATRIX_EFFECT(AMBERS)

#ifdef RGB_MATRIX_CUSTOM_EFFECT_IMPLS

static const uint8_t AMBERS_pal[4][3] = {{255, 190, 20}, {255, 90, 0}, {200, 10, 0}, {255, 190, 20}};

static bool AMBERS(effect_params_t* params) {
    RGB_MATRIX_USE_LIMITS(led_min, led_max);

    uint16_t tm = (uint16_t)((g_rgb_timer * (1 + rgb_matrix_config.speed / 4)) >> 8);

    for (uint8_t i = led_min; i < led_max; i++) {
        int16_t x = g_led_config.point[i].x, y = g_led_config.point[i].y;
        uint8_t w, ph;
        uint8_t col = x / 10;
        uint8_t hc  = (uint8_t)(col * 113 + col * col * 7 + 29);
        w = 0; ph = 255;

        // wind: each recent key press sends a gust that bends and lifts the flame, and throws sparks
        int16_t push = 0;
        uint8_t gust = 0, sw = 0, sph = 0;
#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
        for (uint8_t j = 0; j < g_last_hit_tracker.count; j++) {
            uint16_t tk = scale16by8(g_last_hit_tracker.tick[j], qadd8(rgb_matrix_config.speed >> 2, 1));
            if (tk > 255) continue;
            int16_t hx = x - g_last_hit_tracker.x[j], hy = y - g_last_hit_tracker.y[j];
            int16_t dist = sqrt16(hx * hx + hy * hy), dd = abs(dist - (int16_t)tk);
            uint8_t s = dd < 40 ? 255 - (dd * 255) / 40 : 0;
            s = scale8(scale8(s, 255 - tk), 131);
            if (s > gust) gust = s;
            push += (hx > 0 ? 1 : (hx < 0 ? -1 : 0)) * (s >> 2);
            int16_t u = (tk * (511 - tk)) >> 9;
            for (uint8_t q = 0; q < 3; q++) {
                int16_t sx = g_last_hit_tracker.x[j] + (((q - 1) * u * 3) >> 4) + ((sin8(tk * 5 + q * 85) - 128) >> 5);
                int16_t sy = g_last_hit_tracker.y[j] - ((u * (36 + q * 8)) >> 6);
                int32_t sdx = x - sx, sdy = y - sy, sd2 = sdx * sdx + sdy * sdy;
                if (sd2 >= 120) continue;
                uint8_t e = 255 - (uint8_t)(sd2 * 255 / 120);
                e = scale8(e, 255 - (tk >> 1));
                e = scale8(e, 128 + (sin8(tk * 9 + q * 70) >> 1));
                e = scale8(e, 193);
                if (e > sw) { sw = e; sph = tk >> 1; }
            }
        }
        if (push > 48) push = 48;
        if (push < -48) push = -48;
#endif

        // flame: heat fades with height, torn into tongues by 3 noise layers, swaying sideways
        int32_t hf = ((64 - y) * 255) / 60;
        if (hf < 0) hf = 0;
        int16_t xs = x + ((sin8(y * 5 + tm * 2) - 128) >> 4) - ((push * (hf > 255 ? 255 : (int16_t)hf)) >> 8);
        hf = hf * 255 / (150 + (sin8(hc + (tm >> 1)) >> 1));   // each column pulses to its own height
        hf = hf * (255 - (gust >> 1)) / 255;                    // the gust lifts the flame
        if (hf <= 255) {
            uint8_t n    = (sin8(xs * 7 + (y << 2) + tm) + sin8(xs * 13 - y * 3 + tm * 2 + 85) + sin8(xs * 29 + y * 11 + tm * 3)) / 3;
            uint8_t cool = scale8(scale8(255 - n, 46), (uint8_t)hf);
            uint8_t base = 255 - hf;
            w  = base > cool ? base - cool : 0;
            w  = (w + scale8(w, w)) >> 1;
            w  = qadd8(w, gust >> 2);
            ph = (uint8_t)hf;
        }

        // ember: a lane in this column may launch a spark that rises past the flame
        if (((uint8_t)(col * 59 + col * col * 13 + 71)) < 255) {
            int32_t p = (tm + hc * 41) % (1500 + hc * 6);
            if (p < 240) {
                int32_t ey = 64 - (p * 64) / 240;
                int32_t ex = col * 10 + 5 + ((sin8(p * 3 + hc) - 128) >> 4);
                int32_t edx = x - ex, edy = y - ey, d2 = edx * edx + edy * edy;
                if (d2 < 120) {
                    uint8_t ew = 255 - (uint8_t)(d2 * 255 / 120);
                    ew = scale8(ew, 255 - ((uint8_t)(p * 255 / 240) >> 1));
                    ew = scale8(ew, 128 + (sin8(p * 11 + hc) >> 1));
                    if (ew > w) { w = ew; ph = (uint8_t)((p * 255 / 240) >> 1); }
                }
            }
        }

        if (sw > w) { w = sw; ph = sph; }   // sparks thrown by key presses

        uint8_t pal_seg = (ph) / 85;
        if (pal_seg > 2) pal_seg = 2;
        uint16_t pal_f = ((ph) - pal_seg * 85) * 3;
        if (pal_f > 255) pal_f = 255;
        uint8_t pal_r = AMBERS_pal[pal_seg][0] + ((((int16_t)AMBERS_pal[pal_seg + 1][0] - AMBERS_pal[pal_seg][0]) * (int16_t)pal_f) >> 8);
        uint8_t pal_g = AMBERS_pal[pal_seg][1] + ((((int16_t)AMBERS_pal[pal_seg + 1][1] - AMBERS_pal[pal_seg][1]) * (int16_t)pal_f) >> 8);
        uint8_t pal_b = AMBERS_pal[pal_seg][2] + ((((int16_t)AMBERS_pal[pal_seg + 1][2] - AMBERS_pal[pal_seg][2]) * (int16_t)pal_f) >> 8);
        uint8_t pal_v = scale8(w, rgb_matrix_config.hsv.v);
        uint8_t cr = scale8(pal_r, pal_v), cg = scale8(pal_g, pal_v), cb = scale8(pal_b, pal_v);
        {   // reactive layer: Stardust
            uint8_t w, ph;
            w = 0; ph = 0;
#ifdef RGB_MATRIX_KEYREACTIVE_ENABLED
        for (uint8_t j = 0; j < g_last_hit_tracker.count; j++) {
            uint16_t tk = scale16by8(g_last_hit_tracker.tick[j], qadd8(rgb_matrix_config.speed >> 2, 1));
            if (tk > 255) continue;
            int16_t dx = x - g_last_hit_tracker.x[j], dy = y - g_last_hit_tracker.y[j];
            int16_t a = 0;
            int16_t dist = sqrt16(dx * dx + dy * dy);
            int16_t dd = abs(dist - (int16_t)tk);
            a = dd < 1 ? 255 - (dd * 255) / 1 : 0;
            int16_t behind = (int16_t)tk - dist;
            if (behind > 0 && behind < 3) {
                uint8_t h  = (uint8_t)(dx * 31 + dy * 57 + dx * dy * 3);
                uint8_t q  = (uint8_t)(h + tk * 3);
                uint8_t sp = q < 128 ? q * 2 : (255 - q) * 2;
                sp = scale8(sp, sp);
                sp = scale8(sp, 255 - (behind * 255) / 3);
                if (sp > a) a = sp;
            }
            a = scale8(a, 255 - tk);
            if (a > w) { w = a; ph = (uint8_t)tk; }
        }
#endif
            uint8_t ov_seg = ((uint8_t)(85 + (ph >> 1))) / 85;
            if (ov_seg > 2) ov_seg = 2;
            uint16_t ov_f = (((uint8_t)(85 + (ph >> 1))) - ov_seg * 85) * 3;
            if (ov_f > 255) ov_f = 255;
            uint8_t ov_r = AMBERS_pal[ov_seg][0] + ((((int16_t)AMBERS_pal[ov_seg + 1][0] - AMBERS_pal[ov_seg][0]) * (int16_t)ov_f) >> 8);
            uint8_t ov_g = AMBERS_pal[ov_seg][1] + ((((int16_t)AMBERS_pal[ov_seg + 1][1] - AMBERS_pal[ov_seg][1]) * (int16_t)ov_f) >> 8);
            uint8_t ov_b = AMBERS_pal[ov_seg][2] + ((((int16_t)AMBERS_pal[ov_seg + 1][2] - AMBERS_pal[ov_seg][2]) * (int16_t)ov_f) >> 8);
            uint8_t ov_v = scale8(w, rgb_matrix_config.hsv.v);
            cr = qadd8(cr, scale8(ov_r, ov_v)); cg = qadd8(cg, scale8(ov_g, ov_v)); cb = qadd8(cb, scale8(ov_b, ov_v));
        }
        rgb_matrix_set_color(i, cr, cg, cb);
    }
    return rgb_matrix_check_finished_leds(led_max);
}

#endif

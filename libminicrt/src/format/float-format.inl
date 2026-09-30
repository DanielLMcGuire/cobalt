// ++C C Runtime Library (libminicrt)
// Copyright 2026 Daniel McGuire
// Licensed under the MIT License

static inline int ld_isnan(long double val)
{
#if defined(__clang__) || defined(__GNUC__)
    return __builtin_isnan(val);
#else
    return val != val;
#endif
}

static inline int ld_isinf(long double val)
{
#if defined(__clang__) || defined(__GNUC__)
    return __builtin_isinf(val);
#else
    return val != 0.0L && (val == val * 2.0L);
#endif
}

static inline int ld_signbit(long double val)
{
#if defined(__clang__) || defined(__GNUC__)
    return __builtin_signbit(val);
#else
    union { double d; uint64_t u; } b;
    b.d = (double)val;
    return (int)(b.u >> 63);
#endif
}

static const long double pow10_tab[] = {
    1e1L, 1e2L, 1e4L, 1e8L, 1e16L, 1e32L, 1e64L, 1e128L, 1e256L
#if defined(__LDBL_MAX_10_EXP__) && (__LDBL_MAX_10_EXP__ > 1000)
    , 1e512L, 1e1024L, 1e2048L, 1e4096L
#endif
};

static const long double inv_pow10_tab[] = {
    1e-1L, 1e-2L, 1e-4L, 1e-8L, 1e-16L, 1e-32L, 1e-64L, 1e-128L, 1e-256L
#if defined(__LDBL_MAX_10_EXP__) && (__LDBL_MAX_10_EXP__ > 1000)
    , 1e-512L, 1e-1024L, 1e-2048L, 1e-4096L
#endif
};

static int get_exp10_and_normalize(long double *pval) 
{
    long double v = *pval;
    int exp = 0;
    if (v <= 0.0L) 
    {
        *pval = 0.0L;
        return 0;
    }
    const int num_pow10 = (int)(sizeof(pow10_tab) / sizeof(pow10_tab[0]));
    if (v >= 10.0L)
    {
        for (int i = num_pow10 - 1; i >= 0; i--) 
        {
            while (v >= pow10_tab[i])
            {
                v /= pow10_tab[i];
                exp += (1 << i);
            }
        }
    } 
    else if (v < 1.0L)
    {
        for (int i = num_pow10 - 1; i >= 0; i--)
        {
            while (v <= inv_pow10_tab[i])
            {
                v *= pow10_tab[i];
                exp -= (1 << i);
            }
        }
        while (v < 1.0L && v > 0.0L)
        {
            v *= 10.0L;
            exp--;
        }
    }
    while (v >= 10.0L)
    {
        v /= 10.0L;
        exp++;
    }
    while (v < 1.0L && v > 0.0L)
    {
        v *= 10.0L;
        exp--;
    }
    *pval = v;
    return exp;
}

static void emit_float(char *buf, size_t size, size_t *idx, 
                       long double val, int width, int zero_pad, int precision, 
                       int uppercase, int left_align, int plus_sign, int space_sign, int alt_form) 
{
    if (precision < 0) precision = 6;

    if (ld_isnan(val))
    {
        int pad = width - 3;
        if (!left_align) while (pad-- > 0) emit_char(buf, size, idx, ' ');
        emit_str(buf, size, idx, uppercase ? "NAN" : "nan");
        if (left_align) while (pad-- > 0) emit_char(buf, size, idx, ' ');
        return;
    }
    if (ld_isinf(val))
    {
        int is_neg = ld_signbit(val);
        char sign_char = is_neg ? '-' : (plus_sign ? '+' : (space_sign ? ' ' : 0));
        int pad = width - 3 - (sign_char ? 1 : 0);
        if (!left_align) while (pad-- > 0) emit_char(buf, size, idx, ' ');
        if (sign_char) emit_char(buf, size, idx, sign_char);
        emit_str(buf, size, idx, uppercase ? "INF" : "inf");
        if (left_align) while (pad-- > 0) emit_char(buf, size, idx, ' ');
        return;
    }

    int is_negative = ld_signbit(val);
    if (is_negative) val = -val;

    char digits_buf[5120];
    for (int i = 0; i < (int)sizeof(digits_buf); i++) digits_buf[i] = '0';
    int d_idx = 0;
    int int_len = 0;
    long double norm = 0.0L;

    if (val == 0.0L)
    {
        digits_buf[d_idx++] = '0';
        int_len = 1;
        for (int i = 0; i < precision; i++)
            if (d_idx < (int)sizeof(digits_buf) - 2) digits_buf[d_idx++] = '0';
    } else {
        norm = val;
        int exp = get_exp10_and_normalize(&norm);

        if (exp < 0)
        {
            digits_buf[d_idx++] = '0';
            int_len = 1;
            int leading_zeros = -exp - 1;
            int frac_count = 0;
            while (frac_count < leading_zeros && frac_count < precision + 1 && d_idx < (int)sizeof(digits_buf) - 2)
            {
                digits_buf[d_idx++] = '0';
                frac_count++;
            }
            while (frac_count < precision + 1 && d_idx < (int)sizeof(digits_buf) - 2)
            {
                int d = (int)norm;
                if (d < 0) d = 0;
                if (d > 9) d = 9;
                digits_buf[d_idx++] = (char)('0' + d);
                norm = (norm - d) * 10.0L;
                frac_count++;
            }
        } else {
            int_len = exp + 1;
            int total_needed = int_len + precision + 1;
            if (total_needed > (int)sizeof(digits_buf) - 2)
                total_needed = (int)sizeof(digits_buf) - 2;

            while (d_idx < total_needed)
            {
                int d = (int)norm;
                if (d < 0) d = 0;
                if (d > 9) d = 9;
                digits_buf[d_idx++] = (char)('0' + d);
                norm = (norm - d) * 10.0L;
            }
        }
    }

    int carry = 0;
    if (val != 0.0L && d_idx > 0)
    {
        if (digits_buf[d_idx - 1] > '5')
        {
            carry = 1;
        }
        else if (digits_buf[d_idx - 1] == '5')
        {
            if (norm != 0.0L)
            {
                carry = 1;
            }
            else if (d_idx > 1 && (digits_buf[d_idx - 2] - '0') % 2 != 0)
            {
                carry = 1;
            }
        }
    }
    if (val != 0.0L && d_idx > 0) d_idx--;

    for (int i = d_idx - 1; i >= 0 && carry; i--)
    {
        if (digits_buf[i] == '9')
            digits_buf[i] = '0';
        else
        {
            digits_buf[i]++;
            carry = 0;
        }
    }

    int extra_int = carry ? 1 : 0;

    char sign_char = 0;
    if (is_negative) sign_char = '-';
    else if (plus_sign) sign_char = '+';
    else if (space_sign) sign_char = ' ';

    int has_dot = (precision > 0) || alt_form;
    int total_len = (int_len + extra_int) + (has_dot ? 1 : 0) + precision + (sign_char ? 1 : 0);
    int pad_chars = width - total_len;

    if (!left_align && !zero_pad) while (pad_chars-- > 0) emit_char(buf, size, idx, ' ');
    if (sign_char) emit_char(buf, size, idx, sign_char);
    if (!left_align && zero_pad) while (pad_chars-- > 0) emit_char(buf, size, idx, '0');
    if (extra_int) emit_char(buf, size, idx, '1');
    for (int i = 0; i < int_len; i++) emit_char(buf, size, idx, digits_buf[i]);
    
    if (has_dot)
    {
        emit_char(buf, size, idx, '.');
        for (int i = 0; i < precision; i++)
            emit_char(buf, size, idx, digits_buf[int_len + i]);
    }
    if (left_align) while (pad_chars-- > 0) emit_char(buf, size, idx, ' ');
}

static void emit_float_sci(char *buf, size_t size, size_t *idx, 
                           long double val, int width, int zero_pad, int precision, int uppercase,
                           int left_align, int plus_sign, int space_sign, int alt_form) 
{
    if (precision < 0) precision = 6;

    if (ld_isnan(val))
    {
        int pad = width - 3;
        if (!left_align) while (pad-- > 0) emit_char(buf, size, idx, ' ');
        emit_str(buf, size, idx, uppercase ? "NAN" : "nan");
        if (left_align) while (pad-- > 0) emit_char(buf, size, idx, ' ');
        return;
    }
    if (ld_isinf(val))
    {
        int is_neg = ld_signbit(val);
        char sign_char = is_neg ? '-' : (plus_sign ? '+' : (space_sign ? ' ' : 0));
        int pad = width - 3 - (sign_char ? 1 : 0);
        if (!left_align) while (pad-- > 0) emit_char(buf, size, idx, ' ');
        if (sign_char) emit_char(buf, size, idx, sign_char);
        emit_str(buf, size, idx, uppercase ? "INF" : "inf");
        if (left_align) while (pad-- > 0) emit_char(buf, size, idx, ' ');
        return;
    }

    int is_negative = ld_signbit(val);
    if (is_negative) val = -val;

    int exp = 0;
    if (val > 0.0L)
        exp = get_exp10_and_normalize(&val);

    char digits_buf[1024];
    int d_idx = 0;
    
    for (int i = 0; i < precision + 2; i++)
    {
        int d = (int)val;
        if (d < 0) d = 0;
        if (d > 9) d = 9;
        digits_buf[d_idx++] = (char)('0' + d);
        val = (val - d) * 10.0L;
    }

    int carry = 0;
    if (d_idx > 0)
    {
        if (digits_buf[d_idx - 1] > '5')
        {
            carry = 1;
        }
        else if (digits_buf[d_idx - 1] == '5')
        {
            if (val != 0.0L)
            {
                carry = 1;
            }
            else if (d_idx > 1 && (digits_buf[d_idx - 2] - '0') % 2 != 0)
            {
                carry = 1;
            }
        }
    }
    d_idx--;

    for (int i = d_idx - 1; i >= 0 && carry; i--)
    {
        if (digits_buf[i] == '9')
            digits_buf[i] = '0';
        else
        {
            digits_buf[i]++;
            carry = 0;
        }
    }

    if (carry)
    {
        exp++;
        for (int i = d_idx - 1; i > 0; i--) digits_buf[i] = digits_buf[i - 1];
        digits_buf[0] = '1';
    }

    char exp_buf[32];
    int elen = 0;
    int temp_exp = exp >= 0 ? exp : -exp;
    while (temp_exp > 0)
    {
        exp_buf[elen++] = (char)('0' + (temp_exp % 10));
        temp_exp /= 10;
    }
    if (elen < 2)
    {
        if (elen == 0) exp_buf[elen++] = '0';
        exp_buf[elen++] = '0';
    }

    char sign_char = 0;
    if (is_negative) sign_char = '-';
    else if (plus_sign) sign_char = '+';
    else if (space_sign) sign_char = ' ';

    int has_dot = (precision > 0) || alt_form;
    int total_len = 1 + (has_dot ? 1 : 0) + precision + 2 + elen + (sign_char ? 1 : 0);
    int pad_chars = width - total_len;

    if (!left_align && !zero_pad) while (pad_chars-- > 0) emit_char(buf, size, idx, ' ');
    if (sign_char) emit_char(buf, size, idx, sign_char);
    if (!left_align && zero_pad) while (pad_chars-- > 0) emit_char(buf, size, idx, '0');
    
    emit_char(buf, size, idx, digits_buf[0]);
    if (has_dot)
    {
        emit_char(buf, size, idx, '.');
        for (int i = 0; i < precision; i++) emit_char(buf, size, idx, digits_buf[1 + i]);
    }
    emit_char(buf, size, idx, uppercase ? 'E' : 'e');
    emit_char(buf, size, idx, exp >= 0 ? '+' : '-');
    while (elen > 0) emit_char(buf, size, idx, exp_buf[--elen]);
    
    if (left_align) while (pad_chars-- > 0) emit_char(buf, size, idx, ' ');
}

static void emit_float_g(char *buf, size_t size, size_t *idx, 
                         long double val, int width, int zero_pad, int precision, int uppercase,
                         int left_align, int plus_sign, int space_sign, int alt_form) 
{
    if (precision < 0) precision = 6;
    if (precision == 0) precision = 1;

    if (ld_isnan(val) || ld_isinf(val))
    {
        emit_float(buf, size, idx, val, width, zero_pad, precision, uppercase, left_align, plus_sign, space_sign, alt_form);
        return;
    }

    long double abs_val = val < 0.0L ? -val : val;
    int exp = 0;
    if (abs_val > 0.0L)
    {
        long double v = abs_val;
        exp = get_exp10_and_normalize(&v);
    }

    int use_e = (exp < -4 || exp >= precision);
    int eff_precision = use_e ? (precision - 1) : (precision - 1 - exp);
    if (eff_precision < 0) eff_precision = 0;

    char tmp[1024];
    size_t tmp_idx = 0;

    if (use_e)
        emit_float_sci(tmp, sizeof(tmp), &tmp_idx, val, 0, 0, eff_precision, uppercase, 0, 0, 0, alt_form);
    else
        emit_float(tmp, sizeof(tmp), &tmp_idx, val, 0, 0, eff_precision, uppercase, 0, 0, 0, alt_form);

    int is_neg = (tmp[0] == '-');
    int content_start = is_neg ? 1 : 0;

    char sign_char = 0;
    if (is_neg) sign_char = '-';
    else if (plus_sign) sign_char = '+';
    else if (space_sign) sign_char = ' ';

    int e_pos = -1;
    for (int i = content_start; i < (int)tmp_idx; i++) 
    {
        if (tmp[i] == 'e' || tmp[i] == 'E') 
        {
            e_pos = i;
            break;
        }
    }

    int end_frac = (e_pos != -1) ? e_pos - 1 : (int)tmp_idx - 1;
    int dot_pos = -1;
    for (int i = content_start; i <= end_frac; i++)
    {
        if (tmp[i] == '.')
        {
            dot_pos = i;
            break;
        }
    }

    if (dot_pos != -1 && !alt_form)
    {
        int trim = end_frac;
        while (trim > dot_pos && tmp[trim] == '0')
            trim--;

        if (trim == dot_pos)
            trim--;
        
        int remove_count = end_frac - trim;
        if (remove_count > 0)
        {
            int src = end_frac + 1;
            int dst = trim + 1;
            while (src < (int)tmp_idx)
                tmp[dst++] = tmp[src++];

            tmp_idx = dst;
        }
    }

    int content_len = (int)tmp_idx - content_start;
    int total_len = content_len + (sign_char ? 1 : 0);
    int pad_chars = width - total_len;

    if (!left_align && !zero_pad) while (pad_chars-- > 0) emit_char(buf, size, idx, ' ');
    if (sign_char) emit_char(buf, size, idx, sign_char);
    if (!left_align && zero_pad) while (pad_chars-- > 0) emit_char(buf, size, idx, '0');
    
    for (int i = content_start; i < (int)tmp_idx; i++)
        emit_char(buf, size, idx, tmp[i]);

    if (left_align) while (pad_chars-- > 0) emit_char(buf, size, idx, ' ');
}

static void emit_float_hex(char *buf, size_t size, size_t *idx, 
                          long double lval, int width, int zero_pad, int precision, int uppercase,
                          int left_align, int plus_sign, int space_sign, int alt_form) 
{
    if (ld_isnan(lval))
    {
        int pad = width - 3;
        if (!left_align) while (pad-- > 0) emit_char(buf, size, idx, ' ');
        emit_str(buf, size, idx, uppercase ? "NAN" : "nan");
        if (left_align) while (pad-- > 0) emit_char(buf, size, idx, ' ');
        return;
    }
    if (ld_isinf(lval))
    {
        int is_neg = ld_signbit(lval);
        char sign_char = is_neg ? '-' : (plus_sign ? '+' : (space_sign ? ' ' : 0));
        int pad = width - 3 - (sign_char ? 1 : 0);
        if (!left_align) while (pad-- > 0) emit_char(buf, size, idx, ' ');
        if (sign_char) emit_char(buf, size, idx, sign_char);
        emit_str(buf, size, idx, uppercase ? "INF" : "inf");
        if (left_align) while (pad-- > 0) emit_char(buf, size, idx, ' ');
        return;
    }

    int is_negative = ld_signbit(lval);
    if (is_negative) lval = -lval;

    char tmp[128];
    int len = 0;
    const char *hex_digits = uppercase ? "0123456789ABCDEF" : "0123456789abcdef";

    char sign_char = 0;
    if (is_negative) sign_char = '-';
    else if (plus_sign) sign_char = '+';
    else if (space_sign) sign_char = ' ';

    int lead_digit = 0;
    int bin_exp = 0;
    char frac_hex[40];
    int num_frac = 0;

    if (lval == 0.0L)
    {
        lead_digit = 0;
        bin_exp = 0;
        num_frac = (precision >= 0) ? precision : 0;
        for (int i = 0; i < num_frac; i++) frac_hex[i] = 0;
    } else {
        while (lval >= 0x1p+256L) { lval *= 0x1p-256L; bin_exp += 256; }
        while (lval < 0x1p-256L)  { lval *= 0x1p+256L; bin_exp -= 256; }
        while (lval >= 0x1p+32L)  { lval *= 0x1p-32L;  bin_exp += 32; }
        while (lval < 0x1p-32L)   { lval *= 0x1p+32L;  bin_exp -= 32; }
        while (lval >= 2.0L)      { lval *= 0.5L;      bin_exp += 1; }
        while (lval < 1.0L)       { lval *= 2.0L;      bin_exp -= 1; }

        lead_digit = 1;
        lval -= 1.0L;

        int max_digits = (sizeof(long double) > 8) ? 30 : 14;
        int target_digits = (precision >= 0) ? (precision + 1) : max_digits;
        if (target_digits > 35) target_digits = 35;

        for (int i = 0; i < target_digits; i++)
        {
            lval *= 16.0L;
            int d = (int)lval;
            if (d < 0) d = 0;
            if (d > 15) d = 15;
            frac_hex[i] = (char)d;
            lval -= d;
        }

        if (precision >= 0)
        {
            num_frac = precision;
            if (frac_hex[precision] >= 8)
            {
                int c = 1;
                for (int i = precision - 1; i >= 0; i--)
                {
                    int sum = frac_hex[i] + c;
                    frac_hex[i] = (char)(sum % 16);
                    c = sum / 16;
                    if (c == 0) break;
                }
                if (c > 0)
                {
                    lead_digit += c;
                    if (lead_digit == 2)
                    {
                        lead_digit = 1;
                        bin_exp++;
                    }
                }
            }
        } else {
            num_frac = target_digits;
            while (num_frac > 0 && frac_hex[num_frac - 1] == 0)
                num_frac--;
        }
    }

    tmp[len++] = '0';
    tmp[len++] = uppercase ? 'X' : 'x';
    tmp[len++] = hex_digits[lead_digit];

    int has_dot = (num_frac > 0) || alt_form;
    if (has_dot)
    {
        tmp[len++] = '.';
        for (int i = 0; i < num_frac; i++)
            tmp[len++] = hex_digits[(int)frac_hex[i]];
    }

    tmp[len++] = uppercase ? 'P' : 'p';
    if (bin_exp >= 0)
        tmp[len++] = '+';
    else
    {
        tmp[len++] = '-';
        bin_exp = -bin_exp;
    }

    char exp_buf[20];
    int elen = 0;
    if (bin_exp == 0)
        exp_buf[elen++] = '0';
    else
    {
        while (bin_exp > 0)
        {
            exp_buf[elen++] = (char)('0' + (bin_exp % 10));
            bin_exp /= 10;
        }
    }

    while (elen > 0) 
        tmp[len++] = exp_buf[--elen];

    int total_len = len + (sign_char ? 1 : 0);
    int pad_chars = width - total_len;

    if (!left_align && !zero_pad) while (pad_chars-- > 0) emit_char(buf, size, idx, ' ');
    if (sign_char) emit_char(buf, size, idx, sign_char);
    if (!left_align && zero_pad) while (pad_chars-- > 0) emit_char(buf, size, idx, '0');
    
    for (int i = 0; i < len; i++)
        emit_char(buf, size, idx, tmp[i]);

    if (left_align) while (pad_chars-- > 0) emit_char(buf, size, idx, ' ');
}
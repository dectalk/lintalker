#!/usr/bin/env python3
"""
lex_codec.py -- decode/encode WinTalker English.lex binary dictionary

Usage:
    python3 lex_codec.py decode English.lex > English.txt
    python3 lex_codec.py encode English.txt English.lex

Text format (one word per line, comments start with '#'):
    WORD    /phon phon .../    pos[,pos]  [|  /phon .../  pos[,pos]]

The full POScodes table is stored in the header comments so encode produces a
byte-for-byte identical .lex file without any slot-index annotations in the
word lines.  Editing words and re-encoding works fine; only adding new POS
combinations beyond the original 128 slots would fail.

Phoneme symbols (opcode order):
  IY IH EH AE AA AH AO UH AX ER EY AY OY AW OW UW YU IR XR AR
  OR UR IX SIL RX LX EL EN w y r l h m n NG f v TH DH
  s z SH ZH p b t d k g CH JH TX DX QX DD
  S1=primary-stress  S2=secondary  SE=emphatic
  pR=pitch-rise  pF=pitch-fall  dI=dur-inc  dD=dur-dec
  Syl Wrd Prp Vrb Cm Per Qst Exc Cmp Par EW FW

POS codes:
  nn vb adj prep vaux rvaux interj conj cconj interr det adv inf gen
  relpro ppron ipron rpron dpron art quant neg sadv contr vpart
  subjpron objpron abbrev
"""

import struct
import sys
import re

# ── Constants ──────────────────────────────────────────────────────────────────

KEND          = 0x80   # OR'd with POS slot index → end of phoneme string
KALT          = 0xFF   # alternate-pronunciation marker byte
HASH_ENTRIES  = 28     # 'Z' - 'A' + 2
KPOS_SLOTS    = 128
HEADER_FIXED  = 1160   # bytes before word data (no flags field in real binary):
                       #   nextDict(4)+version(4)+type(4)+wordCount(4)
                       #   +hash[28](112)+POScodes[128][4](1024)
                       #   +words_off(4)+index_off(4)

# ── Phoneme table ──────────────────────────────────────────────────────────────

PHONEME_NAMES = [
    "IY","IH","EH","AE","AA","AH","AO","UH","AX","ER",   #  0–9
    "EY","AY","OY","AW","OW","UW","YU","IR","XR","AR",   # 10–19
    "OR","UR","IX","SIL","RX","LX","EL","EN","w","y",    # 20–29
    "r","l","h","m","n","NG","f","v","TH","DH",           # 30–39
    "s","z","SH","ZH","p","b","t","d","k","g",            # 40–49
    "CH","JH","TX","DX","QX","DD",                        # 50–55
    "S1","S2","SE","pR","pF","dI","dD",                   # 56–62
    "Syl","Wrd","Prp","Vrb","Cm","Per","Qst","Exc",       # 63–70
    "Cmp","Par","EW","FW",                                # 71–74
]
PHON_TO_CODE = {name: i for i, name in enumerate(PHONEME_NAMES)}

# ── POS name table ────────────────────────────────────────────────────────────

POS_NAMES = {
     0: "nn",      1: "vb",       2: "adj",      3: "prep",
     4: "vaux",    5: "rvaux",    6: "interj",   7: "conj",
     8: "cconj",   9: "interr",  10: "det",     11: "adv",
    12: "inf",    13: "gen",     14: "relpro",  15: "ppron",
    16: "ipron",  17: "rpron",   18: "dpron",   19: "art",
    20: "quant",  21: "neg",     22: "sadv",    23: "contr",
    24: "vpart",  25: "subjpron",26: "objpron", 32: "abbrev",
}
POS_FROM_NAME = {v: k for k, v in POS_NAMES.items()}

KUNDEF = -1   # kUndefPOS stored as int16 -1


# ── Low-level helpers ─────────────────────────────────────────────────────────

def be32(buf, off):
    return struct.unpack_from('>I', buf, off)[0]

def be16s(buf, off):
    return struct.unpack_from('>h', buf, off)[0]

def phonemes_to_str(codes):
    return " ".join(
        PHONEME_NAMES[c] if 0 <= c < len(PHONEME_NAMES) else f"#{c}"
        for c in codes
    )

def parse_phoneme_str(s):
    result = []
    for tok in s.split():
        if tok not in PHON_TO_CODE:
            raise ValueError(f"Unknown phoneme: {tok!r}")
        result.append(PHON_TO_CODE[tok])
    return result

def pos_row_to_names(row):
    """[2, -1, 3, -1] -> 'adj,prep'  (skips -1 / kUndefPOS entries)."""
    return ",".join(POS_NAMES.get(v, f"pos{v}") for v in row if v != KUNDEF)

def names_to_pos_values(names_str):
    """'adj,prep' -> [2, 3]   (sorted, de-duped POS values)."""
    if not names_str or names_str == '-':
        return []
    vals = []
    for n in names_str.split(','):
        n = n.strip()
        if not n:
            continue
        if n not in POS_FROM_NAME:
            raise ValueError(f"Unknown POS name: {n!r}")
        vals.append(POS_FROM_NAME[n])
    return sorted(set(vals))


# ── Decode ────────────────────────────────────────────────────────────────────

def decode(lex_path, out=None):
    if out is None:
        out = sys.stdout

    with open(lex_path, 'rb') as f:
        data = bytearray(f.read())

    version   = be32(data, 4)
    typ       = be32(data, 8)
    wordCount = be32(data, 12)
    pos_table = [[be16s(data, 128 + (r * 4 + c) * 2) for c in range(4)]
                 for r in range(KPOS_SLOTS)]
    words_off = be32(data, 1152)   # index-array location (field name swapped in port)
    flags     = be32(data, 1160)   # overlaps first word entry — preserved for round-trip

    # ── Header ──
    print(f"# WinTalker English.lex  version={version} type={typ} flags=0x{flags:x}", file=out)
    print(f"# {wordCount} entries", file=out)
    print(f"#", file=out)
    print(f"# Format:  WORD  /phonemes/  pos[,pos]  [|  /alt-phonemes/  pos[,pos]]", file=out)
    print(f"#", file=out)
    print(f"# Phonemes:", file=out)
    print(f"#   IY IH EH AE AA AH AO UH AX ER  EY AY OY AW OW UW YU IR XR AR", file=out)
    print(f"#   OR UR IX SIL RX LX EL EN  w y r l h m n NG f v TH DH", file=out)
    print(f"#   s z SH ZH  p b t d k g  CH JH TX DX QX DD", file=out)
    print(f"#   S1=primary-stress  S2=secondary  SE=emphatic", file=out)
    print(f"#   pR=pitch-rise  pF=pitch-fall  dI=dur-inc  dD=dur-dec", file=out)
    print(f"#   Syl Wrd Prp Vrb Cm Per Qst Exc Cmp Par EW FW", file=out)
    print(f"#", file=out)
    print(f"# POS: nn vb adj prep vaux rvaux interj conj cconj interr det adv inf gen", file=out)
    print(f"#      relpro ppron ipron rpron dpron art quant neg sadv contr vpart", file=out)
    print(f"#      subjpron objpron abbrev", file=out)
    print(f"#", file=out)

    # ── POScodes table (embedded so encode can reconstruct exact slot mapping) ──
    print(f"# POS-TABLE (128 slots; each row: v0 v1 v2 v3; -1=unused):", file=out)
    for slot, row in enumerate(pos_table):
        names = pos_row_to_names(row)
        raw   = " ".join(f"{v:4d}" for v in row)
        print(f"# SLOT {slot:3d}: {raw}   # {names if names else '(unused)'}", file=out)
    print(f"#", file=out)

    # ── Word entries ──
    idx_base = words_off
    for i in range(wordCount):
        entry_off = be32(data, idx_base + i * 4)

        wlen = data[entry_off]
        word = bytes(data[entry_off + 1 : entry_off + 1 + wlen]).decode('ascii')
        p    = entry_off + 1 + wlen

        # Primary phoneme string
        phons1 = []
        while not (data[p] & KEND):
            phons1.append(data[p])
            p += 1
        slot1  = data[p] & 0x7F
        pos1   = pos_row_to_names(pos_table[slot1])
        p += 1

        # Optional alternate pronunciation
        phons2 = pos2 = None
        if p < len(data) and data[p] == KALT:
            p += 1
            phons2 = []
            while not (data[p] & KEND):
                phons2.append(data[p])
                p += 1
            slot2 = data[p] & 0x7F
            pos2  = pos_row_to_names(pos_table[slot2])

        ps1  = phonemes_to_str(phons1)
        line = f"{word:<20s} /{ps1}/  {pos1 if pos1 else '-'}"
        if phons2 is not None:
            ps2   = phonemes_to_str(phons2)
            line += f"  |  /{ps2}/  {pos2 if pos2 else '-'}"
        print(line, file=out)


# ── Encode ────────────────────────────────────────────────────────────────────

def encode(txt_path, lex_path):
    # ── Parse text file ──────────────────────────────────────────────────────
    header_meta  = {'version': 1, 'type': 1, 'flags': 0}
    pos_table_in = None   # list of 128 rows [[v0,v1,v2,v3], ...] from header
    raw_entries  = []     # (word, phons1, pos_names1, phons2, pos_names2)

    _slot_pat = re.compile(
        r'^#\s*SLOT\s+(\d+):\s+(-?\d+)\s+(-?\d+)\s+(-?\d+)\s+(-?\d+)'
    )

    with open(txt_path, 'r') as f:
        in_pos_table = False
        pos_rows_read = []

        for lineno, line in enumerate(f, 1):
            line = line.rstrip('\n')

            if line.lstrip().startswith('#'):
                # Mine metadata
                for key, pat in [('version', r'version=(\d+)'),
                                  ('type',    r'type=(\d+)')]:
                    m = re.search(pat, line)
                    if m:
                        header_meta[key] = int(m.group(1))
                m = re.search(r'flags=0x([0-9a-fA-F]+)', line)
                if m:
                    header_meta['flags'] = int(m.group(1), 16)

                # POScodes table rows
                m = _slot_pat.match(line)
                if m:
                    slot = int(m.group(1))
                    row  = [int(m.group(i)) for i in range(2, 6)]
                    if len(pos_rows_read) <= slot:
                        pos_rows_read.extend([[-1,-1,-1,-1]] * (slot - len(pos_rows_read) + 1))
                    pos_rows_read[slot] = row
                continue

            if not line.strip():
                continue

            # Word line: WORD  /phonemes/  pos[,pos]  [|  /phonemes/  pos]
            parts = line.split('|', 1)
            main_s = parts[0].strip()
            alt_s  = parts[1].strip() if len(parts) > 1 else None

            m = re.match(r'(\S+)\s+/([^/]*)/\s*([\w,]*)', main_s)
            if not m:
                print(f"WARNING line {lineno}: can't parse {main_s!r}", file=sys.stderr)
                continue

            word       = m.group(1).upper()
            phons1     = parse_phoneme_str(m.group(2))
            pos_names1 = m.group(3).strip()

            phons2 = pos_names2 = None
            if alt_s:
                m2 = re.match(r'/([^/]*)/\s*([\w,]*)', alt_s)
                if m2:
                    phons2     = parse_phoneme_str(m2.group(1))
                    pos_names2 = m2.group(2).strip()
                else:
                    print(f"WARNING line {lineno}: can't parse alt {alt_s!r}", file=sys.stderr)

            raw_entries.append((word, phons1, pos_names1, phons2, pos_names2))

    if len(pos_rows_read) == KPOS_SLOTS:
        pos_table_in = pos_rows_read
        while len(pos_table_in) < KPOS_SLOTS:
            pos_table_in.append([-1, -1, -1, -1])

    # ── Build POS slot mapping ────────────────────────────────────────────────
    # Strategy: if we have the original table from the header, use it to find
    # the correct slot for each POS name-set.  If not, build from scratch.

    def row_matches_names(row, names_str):
        """True if the row's defined values (not -1) match the POS names set."""
        row_vals  = frozenset(v for v in row if v != KUNDEF)
        want_vals = frozenset(names_to_pos_values(names_str))
        return row_vals == want_vals

    if pos_table_in is not None:
        # Use exact slots from decoded table
        def find_slot(names_str):
            for slot, row in enumerate(pos_table_in):
                if row_matches_names(row, names_str):
                    return slot
            raise ValueError(
                f"POS combination {names_str!r} not found in POS-TABLE header. "
                "Add a new slot or re-decode to regenerate the table."
            )
        pos_table_out = [list(r) for r in pos_table_in]
    else:
        # Build table from scratch
        pos_sets  = []
        seen_sets = {}

        def find_slot(names_str):
            vals = tuple(sorted(names_to_pos_values(names_str)))
            if vals not in seen_sets:
                if len(pos_sets) >= KPOS_SLOTS:
                    raise ValueError(f"More than {KPOS_SLOTS} unique POS combinations")
                seen_sets[vals] = len(pos_sets)
                pos_sets.append(vals)
            return seen_sets[vals]

        # Pre-pass to register all POS combinations
        for _, _, pn1, _, pn2 in raw_entries:
            find_slot(pn1)
            if pn2 is not None:
                find_slot(pn2)

        pos_table_out = []
        for vals in pos_sets:
            row = list(vals) + [-1] * 4
            pos_table_out.append(row[:4])
        while len(pos_table_out) < KPOS_SLOTS:
            pos_table_out.append([-1, -1, -1, -1])

    # ── Sort word entries ─────────────────────────────────────────────────────
    raw_entries.sort(key=lambda e: e[0])
    wordCount = len(raw_entries)

    # ── Build word data blob ──────────────────────────────────────────────────
    word_data    = bytearray()
    word_offsets = []

    for word, phons1, pn1, phons2, pn2 in raw_entries:
        word_offsets.append(len(word_data))
        wb = word.encode('ascii')
        word_data.append(len(wb))
        word_data.extend(wb)
        word_data.extend(phons1)
        word_data.append(find_slot(pn1) | KEND)
        if phons2 is not None:
            word_data.append(KALT)
            word_data.extend(phons2)
            word_data.append(find_slot(pn2) | KEND)

    # ── Build hash table ──────────────────────────────────────────────────────
    hash_arr    = [wordCount] * HASH_ENTRIES
    hash_arr[26] = wordCount
    hash_arr[27] = wordCount

    for i in range(wordCount - 1, -1, -1):
        ch = ord(raw_entries[i][0][0]) if raw_entries[i][0] else 0
        if ord('A') <= ch <= ord('Z'):
            hash_arr[ch - ord('A')] = i

    # Forward-fill letters that have no entries
    for i in range(HASH_ENTRIES - 2, -1, -1):
        if hash_arr[i] == wordCount:
            hash_arr[i] = hash_arr[i + 1]

    # ── Assemble binary ───────────────────────────────────────────────────────
    word_data_start   = HEADER_FIXED
    index_array_start = word_data_start + len(word_data)
    abs_offsets       = [word_data_start + off for off in word_offsets]

    version_out = header_meta.get('version', 1)
    type_out    = header_meta.get('type',    1)
    flags_out   = header_meta.get('flags',   0)

    buf = bytearray()
    buf.extend(struct.pack('>I', 0))              # nextDict_off
    buf.extend(struct.pack('>I', version_out))
    buf.extend(struct.pack('>I', type_out))
    buf.extend(struct.pack('>I', wordCount))
    for h in hash_arr:
        buf.extend(struct.pack('>I', h))
    for row in pos_table_out:
        for v in row:
            buf.extend(struct.pack('>h', v))
    buf.extend(struct.pack('>I', index_array_start))  # words_off = index location
    buf.extend(struct.pack('>I', 0))                  # index_off = 0

    assert len(buf) == HEADER_FIXED, f"Header size {len(buf)} != {HEADER_FIXED}"

    buf.extend(word_data)
    for off in abs_offsets:
        buf.extend(struct.pack('>I', off))

    with open(lex_path, 'wb') as f:
        f.write(buf)

    print(f"Encoded {wordCount} words → {len(buf)} bytes → {lex_path}", file=sys.stderr)


# ── Main ──────────────────────────────────────────────────────────────────────

def main():
    if len(sys.argv) < 3:
        sys.exit(
            "Usage:\n"
            "  lex_codec.py decode FILE.lex          (writes text to stdout)\n"
            "  lex_codec.py encode FILE.txt FILE.lex"
        )

    cmd = sys.argv[1].lower()
    if cmd == 'decode':
        decode(sys.argv[2])
    elif cmd == 'encode':
        if len(sys.argv) < 4:
            sys.exit("Usage: lex_codec.py encode FILE.txt FILE.lex")
        encode(sys.argv[2], sys.argv[3])
    else:
        sys.exit(f"Unknown command: {cmd!r}  (use 'decode' or 'encode')")


if __name__ == '__main__':
    main()

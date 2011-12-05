function ltrim(s) { sub(/^[ \t]+/, "", s); return s }
function rtrim(s) { sub(/[ \t]+$/, "", s); return s }
function trim(s)  { return rtrim(ltrim(s)); }

BEGIN {FS=":"}

/Mispred_penalty_cond_br/ {
    print trim($2);
}
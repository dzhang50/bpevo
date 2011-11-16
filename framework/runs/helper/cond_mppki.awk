function ltrim(s) { sub(/^[ \t]+/, "", s); return s }
function rtrim(s) { sub(/[ \t]+$/, "", s); return s }
function trim(s)  { return rtrim(ltrim(s)); }

BEGIN {FS=":"}

/Conditional_MPPKI/ {
    print trim($2);
}